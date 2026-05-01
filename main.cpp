#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <Arduino.h>

#include "game_logic.h"
#include "render.h"
#include "src/display.h"
#include "sound.h"
#include "tools.h"

sequence_object *sequence_1;
sequence_object *sequence_2;

static void _initialize_esp()
{
  Serial.begin(115200);

  // delay(1000);
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

  _intializate_sequence();

  sequence_1 = _add_to_sequence(sequence_object{.delay = 0, .function_type = FUNC_SIMPLE, .without_ptr = &_check_next_audio_send_timer});
  sequence_2 = _add_to_sequence(sequence_object{.delay = 0, .function_type = FUNC_SIMPLE, .without_ptr = &logic});
}

void loop()
{
  _check_sequence();
}