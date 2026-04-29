#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <Arduino.h>


#include "game_logic.h"
#include "render.h"
#include "src/display.h"
#include "sound.h"

uint32_t last_send_time = 0;
uint16_t interval = 23.2;


static void _initialize_esp()
{
  Serial.begin(115200);
  
  //delay(1000);
  randomSeed(esp_random());

  touch_input_screen.begin(GT911_ADDR1);
  touch_input_screen.setRotation(ROTATION_INVERTED);

  gfx->begin(); // calls Arduino_4848_Display::begin()
  gfx->setBacklight(true);
}

void logMemory()
{
  Serial.printf("Used PSRAM: %d\n", ESP.getPsramSize() - ESP.getFreePsram());
}

void setup()
{
  _initialize_esp();

  // Preparing board and random gems
  _initialize_game();

  // Creating a canvas
  initialize_render();

  // Creating WiFi_AP
  _initialize_wifi_ap();

  _initialize_audio_engine();

  _refresh_the_screen();
}

void loop()
{
  logic();
  uint32_t now = millis();
  if (now - last_send_time >= interval)
  {
    // Attempt to send
    if (_send_audio_to_vlc_robust())
    {
      last_send_time = now;
    }
    else
    {
      // If it failed, we don't update last_send_time.
      // We try again as fast as possible in the next loop!
    }
  }
}