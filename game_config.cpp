#include <stdint.h>
#include "game_config.h"
#include "gem_asset.h"
#include "cell_asset.h"
#include "selected_entity_frame_asset.h"

uint8_t rows = 7;
uint8_t cols = 7;

uint16_t _window_width = 700;
uint16_t _window_height = 700;
uint16_t _background_color = BLACK;

int16_t _board_beginning_position_x = 15;
int16_t _board_beginning_position_y = 10;

uint16_t _rectangle_width = 60;
uint16_t _rectangle_height = 60;
int8_t _rectangle_row_space = 5;
int8_t _rectangle_cols_space = 5;
uint16_t _rectangle_color = GREEN;

int8_t gem_width = 32;
int8_t gem_height = 32;

uint16_t targeted_frame_rate = 10;

game_state current_game_state = INPUT_WAITING;

uint8_t gem_swap_animation_time_to_complete = 150;

const uint16_t *gem_types_table[GEM_TYPE_LENGTH] = {
    gem_red,
    gem_green,
    gem_blue,
    gem_yellow,
    gem_purple,
    gem_orange
};

const uint16_t *cell_table[1] = {
    cell
};

const uint16_t *entity_frame = selected_entity_frame;

Touch_GT911 touch_input_screen(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, 480, 480);
