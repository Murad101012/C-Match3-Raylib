#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROWS 7
#define COLS 4

static int _window_width = 700;
static int _window_height = 700;
static int _target_fps = 30;
static Color _background_color = BLACK;

static int _board_beginning_position_x = 150;
static int _board_beginning_position_y = 90;

static int _rectangle_width = 75;
static int _rectangle_height = 75;
static int _rectangle_row_space = 5;
static int _rectangle_cols_space = 5;
static Color _rectangle_color = GREEN;

static int _circle_radius = 25;

static Color _gem_type_color[] = {RED, BLUE, LIME, GOLD};

struct Gem {
  int cell_index;
  int cell_x;
  int cell_y;
  int type;
  Color color;
  int pos_y;
  int pos_x;
};

struct Gem board[ROWS][COLS];

static void _initialize_window() {
  InitWindow(_window_height, _window_height, "Raylib testing");
  SetTargetFPS(_target_fps);
}

static void _draw_board() {
  int _current_x_position = _board_beginning_position_x;
  int _current_y_position = _board_beginning_position_y;
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      board[i][j].cell_y = _current_y_position;
      board[i][j].cell_x = _current_x_position;
      DrawRectangle(board[i][j].cell_x, board[i][j].cell_y, _rectangle_width,
                    _rectangle_height, _rectangle_color);
      _current_x_position += _rectangle_cols_space + _rectangle_width;
    }
    _current_x_position = _board_beginning_position_x;
    _current_y_position += _rectangle_row_space + _rectangle_height;
  }
}

static void _draw_gems_on_board() {
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      DrawCircle(board[i][j].cell_x + _rectangle_width / 2,
                 board[i][j].cell_y + _rectangle_height / 2, _circle_radius,
                 board[i][j].color);
    }
  }
}

static void _initialize_game() {
  // Creating randomized games are type between 1 and 4;
  srand(time(NULL));
  int step = 0;
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      board[i][j].cell_index = step;
      board[i][j].type = rand() % 4 + 1;
      board[i][j].color = _gem_type_color[board[i][j].type - 1];
      step++;
    }
  }
}

// This is the old version of _gem_return_on_mouse_click
static void _gem_return_on_mouse_click_deprecated() {
  Vector2 mouse_position = GetMousePosition();
  printf("%f %f", GetMousePosition().x, GetMousePosition().y);
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLS; j++) {
      if (mouse_position.x > board[i][j].cell_x &&
          mouse_position.x < board[i][j].cell_x + _rectangle_width) {
        if (mouse_position.y > board[i][j].cell_y &&
            mouse_position.y < board[i][j].cell_y + _rectangle_height) {
          printf("You clicked: %d\n", board[i][j].cell_index);
          return;
        }
      }
    }
  }
  printf("You clicked nowhere\n");
}

/**
 * @brief Translates screen-space mouse coordinates into 2D grid indices.
 * * @param None (Uses Raylib's GetMousePosition internally).
 * @return void (Currently logs the clicked cell index to the console).
 * @note Currently the gap between gems are clickable but, it's intentionally left
 */
static void _gem_return_on_mouse_click() {
  Vector2 mouse_position_normalized = 
  (Vector2){GetMousePosition().x - _board_beginning_position_x,
            GetMousePosition().y - _board_beginning_position_y};

  int index_x = (int)((mouse_position_normalized.y / (float)(_rectangle_height + _rectangle_row_space)));
  int index_y = (int)((mouse_position_normalized.x / (float)(_rectangle_width + _rectangle_cols_space)));

  if (index_x < ROWS && index_y < COLS
     && index_x >= 0 && index_y >= 0
     && mouse_position_normalized.x >= 0 && mouse_position_normalized.y >= 0) {
    printf("You clicked: %d\n", board[index_x][index_y].cell_index);
  }
}

int main(void) {
  _initialize_window();
  _initialize_game();

  while (!WindowShouldClose()) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      _gem_return_on_mouse_click();
    }
    BeginDrawing();
    ClearBackground(_background_color);
    _draw_board();
    _draw_gems_on_board();
    EndDrawing();
  }

  return 0;
}
