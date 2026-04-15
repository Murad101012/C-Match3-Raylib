#include <stdint.h>
#include "game_config.h"

uint8_t rows = 7;
uint8_t cols = 6;

uint16_t _window_width = 700;
uint16_t _window_height = 700;
int8_t _target_fps = 30;
uint16_t _background_color = BLACK;

int16_t _board_beginning_position_x = 50;
int16_t _board_beginning_position_y = 25;

uint16_t _rectangle_width = 60;
uint16_t _rectangle_height = 60;
int8_t _rectangle_row_space = 5;
int8_t _rectangle_cols_space = 5;
uint16_t _rectangle_color = GREEN;

int8_t _circle_radius = 22;

uint16_t _gem_type_color[] = {RED, BLUE, DARK_GREEN, GOLD, PURPLE};

Touch_GT911 touch_input_screen(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, 480, 480);
