#include "WiFi.h"
#define INSTRUMENT_FILE
#include "tiny_instruments_sf2.h"
extern "C"
{
#define TSF_IMPLEMENTATION
#include "tsf.h"
#define TML_IMPLEMENTATION
#include "tml.h"
}
#define BACKGROUND_OST
#include "neon_rose_midi.h"
#define AUDIO_SAMPLES 512
int16_t audio_buffer[AUDIO_SAMPLES];

WiFiUDP udp;
const int port = 12345;

// The Synthesizer instance
tsf *g_TinySoundFont = nullptr;

// The MIDI Loader instance (a linked list of MIDI messages)
tml_message *g_MidiFirstMsg = nullptr;

// A pointer to keep track of where we are in the song
tml_message *g_MidiCurrentMsg = nullptr;

// Track the song time in milliseconds
double g_SongTimeMS = 0;

void _initialize_audio_engine()
{
    // 1. Load the SoundFont from your memory array
    g_TinySoundFont = tsf_load_memory(_tiny_instruments_sf2, sizeof(_tiny_instruments_sf2));

    // 2. Setup output: Mono, 22050Hz, and 0dB gain (1.0f)
    tsf_set_output(g_TinySoundFont, TSF_MONO, 22050, 0);

    // 3. Load the MIDI file instructions
    g_MidiFirstMsg = tml_load_memory(_neon_rose_midi, sizeof(_neon_rose_midi));

    // 4. Start at the beginning of the song
    g_MidiCurrentMsg = g_MidiFirstMsg;
    g_SongTimeMS = 0;
}

void _render_audio_block(int16_t *output_buffer, int samples_to_render)
{
    // Step A: Update the song clock
    // At 22050Hz, 512 samples is 23.2ms
    double block_duration_ms = (samples_to_render * 1000.0) / 22050.0;
    double end_time_ms = g_SongTimeMS + block_duration_ms;

    // Step B: Loop through all MIDI messages scheduled for this time window
    while (g_MidiCurrentMsg && g_MidiCurrentMsg->time <= end_time_ms)
    {
        switch (g_MidiCurrentMsg->type)
        {
        case TML_NOTE_ON:
            // This function automatically maps the channel to the right instrument
            tsf_channel_note_on(g_TinySoundFont, g_MidiCurrentMsg->channel, g_MidiCurrentMsg->key, g_MidiCurrentMsg->velocity / 127.0f);
            break;

        case TML_NOTE_OFF:
            tsf_channel_note_off(g_TinySoundFont, g_MidiCurrentMsg->channel, g_MidiCurrentMsg->key);
            break;
        case TML_PROGRAM_CHANGE:
            // This changes the instrument (Piano, Drums, etc.)
            tsf_channel_set_presetnumber(g_TinySoundFont, g_MidiCurrentMsg->channel, g_MidiCurrentMsg->program, (g_MidiCurrentMsg->channel == 9));
            break;
        }
        g_MidiCurrentMsg = g_MidiCurrentMsg->next;

        // Loop song if we reach the end
        if (!g_MidiCurrentMsg)
        {
            g_MidiCurrentMsg = g_MidiFirstMsg;
            g_SongTimeMS = 0;
            for (int i = 0; i < 16; i++)
                tsf_channel_sounds_off_all(g_TinySoundFont, i);
            return;
        }
    }

    // Step C: Render the "Speaker Positions" into our buffer
    tsf_render_short(g_TinySoundFont, output_buffer, samples_to_render, 0);

    // Update global time for the next call
    g_SongTimeMS = end_time_ms;
}

bool _send_audio_to_vlc_robust()
{
    _render_audio_block(audio_buffer, AUDIO_SAMPLES);

    if (udp.beginPacket("192.168.4.2", port))
    {
        udp.write((const uint8_t *)audio_buffer, AUDIO_SAMPLES * sizeof(int16_t));

        // Returns 1 if the packet was sent to the Wi-Fi stack successfully
        return (udp.endPacket() == 1);
    }
    return false;
}

void _initialize_wifi_ap(){
    WiFi.softAP("ESP_AP", "12345678");
    udp.begin(port);
}