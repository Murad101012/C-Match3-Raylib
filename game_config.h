#pragma once
#include <stdint.h>
#include <src/Touch_GT911.h>

#define MAX_ROWS 7
#define MAX_COLS 7
#define GEM_CHOSEN_COUNT 2
#define GEM_TYPE_LENGTH 6

extern uint16_t _gem_type_color[GEM_TYPE_LENGTH];

typedef struct
{
    int x;
    int y;
} Vec2I;

#define RED 0xF800
#define BLUE 0x001F
#define GREEN 0x07E0
#define DARK_GREEN 0x03E0
#define GOLD 0xFEA0
#define PURPLE 0x780F
#define WHITE 0xFFFF
#define BLACK 0x0000

extern uint8_t rows;
extern uint8_t cols;

extern uint16_t _window_width;
extern uint16_t _window_height;
extern int8_t _target_fps;
extern uint16_t _background_color;

extern int16_t _board_beginning_position_x;
extern int16_t _board_beginning_position_y;

extern uint16_t _rectangle_width;
extern uint16_t _rectangle_height;
extern int8_t _rectangle_row_space;
extern int8_t _rectangle_cols_space;
extern uint16_t _rectangle_color;

extern int8_t gem_width;
extern int8_t gem_height;

extern const uint16_t *gem_types_table[GEM_TYPE_LENGTH];
extern const uint16_t *cell_table[1];

typedef enum
{
    GEM_RED,
    GEM_GREEN,
    GEM_BLUE,
    GEM_YELLOW,
    GEM_PURPLE,
    GEM_ORANGE
} gem_type;

typedef enum
{
    CELL
} cell_type;

#define TOUCH_SDA 19
#define TOUCH_SCL 45
#define TOUCH_INT -1
#define TOUCH_RST -1

extern Touch_GT911 touch_input_screen;