#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "game_logic.h"
#include "render.h"
#include "src/display.h"

static void _initialize_esp()
{
  Serial.begin(115200);
  delay(1000);
  randomSeed(esp_random());

  touch_input_screen.begin(GT911_ADDR1);
  touch_input_screen.setRotation(ROTATION_INVERTED);

  gfx->begin(); // calls Arduino_4848_Display::begin()
  gfx->setBacklight(true);
}

void setup()
{
  _initialize_esp();

  // Preparing board and random gems
  _initialize_game();

  // Creating a canvas
  initialize_render();

  _refresh_the_screen();
}

void loop()
{
  logic();
}