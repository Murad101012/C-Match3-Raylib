#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <bits/time.h>
#include <linux/time.h>
#include <stdint.h>

#define ROWS 7
#define COLS 7

#define GEMCHOSENCOUNT 2

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

static bool _is_screen_dirty = true;

static Color _gem_type_color[] = {RED, BLUE, LIME, GOLD, PURPLE};

// Properties of general entities
static int16_t _entity_type[ROWS][COLS];
static int16_t _entity_posx_posy[ROWS][COLS][2 /*ROWS&COLS*/];

// Properties of specifially gem entity
static bool _gems_match_state[ROWS][COLS];

// Properties of Cell
static int16_t _cell_posx_posy[ROWS][COLS][2 /*ROWS&COLS*/];

static int16_t _returned_gem_index_on_mouse_click[2];

static int16_t _chosen_gems_indexs[GEMCHOSENCOUNT][2 /*ROWS&COLS*/];

static void _initialize_window()
{
  InitWindow(_window_height, _window_height, "Raylib testing");
  SetTargetFPS(_target_fps);
}

static void _draw_board()
{

  for (int i = 0; i < ROWS; i++)
  {
    for (int j = 0; j < COLS; j++)
    {
      DrawRectangle(_cell_posx_posy[i][j][0], _cell_posx_posy[i][j][1], _rectangle_width, _rectangle_height, _rectangle_color);
    }
  }
}

static void _draw_gems()
{
  for (int i = 0; i < ROWS; i++)
  {
    for (int j = 0; j < COLS; j++)
    {
      if (_entity_type[i][j] >= 0)
      {
        DrawCircle(_entity_posx_posy[i][j][0], _entity_posx_posy[i][j][1], _circle_radius, _gem_type_color[_entity_type[i][j]]);
      }
    }
  }
}

static void _update_gem_position_based_on_cell(int i, int j)
{
  _entity_posx_posy[i][j][0] = _cell_posx_posy[i][j][0] + _rectangle_width / 2;
  _entity_posx_posy[i][j][1] = _cell_posx_posy[i][j][1] + _rectangle_height / 2;
}

/**
 * @brief Translates screen-space mouse coordinates into 2D grid indices.
 * * @param None (Uses Raylib's GetMousePosition internally).
 */
static void _gem_return_on_mouse_click()
{
  Vector2 mouse_position_normalized =
      (Vector2){GetMousePosition().x - _board_beginning_position_x,
                GetMousePosition().y - _board_beginning_position_y};

  int index_x = (int)((mouse_position_normalized.y / (float)(_rectangle_height + _rectangle_row_space)));
  int index_y = (int)((mouse_position_normalized.x / (float)(_rectangle_width + _rectangle_cols_space)));

  if (index_x < ROWS && index_y < COLS && index_x >= 0 && index_y >= 0 && mouse_position_normalized.x >= 0 && mouse_position_normalized.y >= 0)
  {
    _returned_gem_index_on_mouse_click[0] = index_x;
    _returned_gem_index_on_mouse_click[1] = index_y;
    printf("Returned gem index:[%d][%d]\n", index_x, index_y);
    return;
  }
  _returned_gem_index_on_mouse_click[0] = -999;
  _returned_gem_index_on_mouse_click[1] = -999;
}

/**
 * @brief Keep index (Row/Cols) of gem's type
 *
 */
static void _gem_choser(int row, int col)
{
  for (int i = 0; i < GEMCHOSENCOUNT; i++)
  {
    // We only check rows since, if it's -999, cols also must be -999
    if (_chosen_gems_indexs[i][0] == -999)
    {
      _chosen_gems_indexs[i][0] = row;
      _chosen_gems_indexs[i][1] = col;
      break;
    }
  }
}

static bool _check_horizontal_matches(int i, int j)
{
  if (COLS - 2 > j)
  {
    for (int y = 1; y < 3; y++)
    {
      if (_entity_type[i][j] != _entity_type[i][j + y])
      {
        return false;
      }
    }
    return true;
  }
  return false;
}

static bool _check_horizontal_matches_reverse(int i, int j)
{
  if (j >= 2)
  {
    for (int y = -1; y > -3; y--)
    {
      if (_entity_type[i][j] != _entity_type[i][j + y])
      {
        return false;
      }
    }
    return true;
  }
  return false;
}

static bool _check_vertical_matches(int i, int j)
{
  if (ROWS - 2 > i)
  {
    for (int x = 1; x < 3; x++)
    {
      if (_entity_type[i][j] != _entity_type[i + x][j])
      {
        return false;
      }
    }
    return true;
  }
  return false;
}

static bool _check_vertical_matches_reverse(int i, int j)
{
  if (i >= 2)
  {
    for (int x = -1; x > -3; x--)
    {
      if (_entity_type[i][j] != _entity_type[i + x][j])
      {
        return false;
      }
    }
    return true;
  }
  return false;
}

static bool _check_left_right_match(int i, int j)
{
  if (COLS - 1 > j && j >= 1)
  {
    if ((_entity_type[i][j] != _entity_type[i][j - 1]) || (_entity_type[i][j] != _entity_type[i][j + 1]))
    {
      return false;
    }
    return true;
  }
  return false;
}

static bool _check_up_down_match(int i, int j)
{
  if (ROWS - 1 > i && i >= 1)
  {
    if ((_entity_type[i][j] != _entity_type[i - 1][j]) || (_entity_type[i][j] != _entity_type[i + 1][j]))
    {
      return false;
    }
    return true;
  }
  return false;
}

static void _remove_matches()
{
  for (int i = 0; i < ROWS; i++)
  {
    for (int j = 0; j < COLS; j++)
    {
      if (_gems_match_state[i][j])
      {
        _entity_type[i][j] = -1;
      }
    }
  }
}

/**
 * @brief Cheap version match finder for all gems and mark them _is_matched = true, which is CPU friendly
 * @note It's required to use only when the gem's position, types or any change on board/gems
 * will not happen until the _find_matches() finish
 */
static void _find_matches()
{
  bool match_found = false;
  for (int i = 0; i < ROWS; i++)
  {
    for (int j = 0; j < COLS; j++)
    {
      if (ROWS - 2 > i)
      {
        // Look for vertical
        if (_entity_type[i][j] != -1 && _check_vertical_matches(i, j))
        {
          for (int x = 0; x < 3; x++)
          {
            _gems_match_state[i + x][j] = true;
            if (!match_found)
            {
              match_found = true;
            }
          }
        }
      }
      if (COLS - 2 > j)
      {
        // Look for horizontal
        if (_entity_type[i][j] != -1 && _check_horizontal_matches(i, j))
        {
          for (int y = 0; y < 3; y++)
          {
            _gems_match_state[i][j + y] = true;
            if (!match_found)
            {
              match_found = true;
            }
          }
        }
      }
    }
  }
}

/**
 * @brief Check a gem if have match from all possible directions
 *
 */
static inline bool _match_availability_checker_for_gem(int i, int j)
{
  // Look for vertical
  // printf("Vertical called for: gems[%d][%d]\n", i, j);
  if (_check_vertical_matches(i, j))
    return true;

  // Look for horizontal
  // printf("Horizontal called for: gems[%d][%d]\n", i, j);
  if (_check_horizontal_matches(i, j))
    return true;

  // Look for vertical reverse
  // printf("Vertical reverse called for: gems[%d][%d]\n", i, j);
  if (_check_vertical_matches_reverse(i, j))
    return true;

  // Look for horizontal reverse
  // printf("Horizontal reverse called for: gems[%d][%d]\n", i, j);
  if (_check_horizontal_matches_reverse(i, j))
    return true;

  // Look for sides of gem
  // printf("Left/Right called for: gems[%d][%d]\n", i, j);
  if (_check_left_right_match(i, j))
    return true;

  // Look bottom/above of gem
  // printf("Up/Down called for: gems[%d][%d]\n", i, j);
  if (_check_up_down_match(i, j))
    return true;

  // If match no found
  return false;
}

static bool _check_swap_gem_requirement()
{
  // Before checking all gems, it do simple check for those two selected gems
  return (_match_availability_checker_for_gem(_chosen_gems_indexs[0][0], _chosen_gems_indexs[0][1]) ||
          _match_availability_checker_for_gem(_chosen_gems_indexs[1][0], _chosen_gems_indexs[1][1]));
}

/**
 * @brief Reset chosen gems
 */
static void _reset_chosen_gems_state()
{
  for (int i = 0; i < GEMCHOSENCOUNT; i++)
  {
    _chosen_gems_indexs[i][0] = -999;
    _chosen_gems_indexs[i][1] = -999;
  }
}

static void _swap_gems_types()
{
  // Taking the type of first element
  int16_t tempType = _entity_type[_chosen_gems_indexs[0][0]][_chosen_gems_indexs[0][1]];

  // Then swapping them
  _entity_type[_chosen_gems_indexs[0][0]][_chosen_gems_indexs[0][1]] = _entity_type[_chosen_gems_indexs[1][0]][_chosen_gems_indexs[1][1]];

  _entity_type[_chosen_gems_indexs[1][0]][_chosen_gems_indexs[1][1]] = tempType;
}

/**
 * @brief Find matches and change it's type based on horizontal/vertical check
 *
 */
static void _reinitialize_matches()
{
  // We collect types those are cause match in four direction by eliminating
  int _gem_types_length = sizeof(_gem_type_color) / sizeof(_gem_type_color[0]);
  /*Since comparing between type and color not possible and originally
  each _gem_type_color's index equal to type (e.g: _gem_type_color[2] is same as type = 2)
  creating an index based color from 0 to 1 represinting color's index*/
  int _gem_types[_gem_types_length];
  for (int i = 0; i < _gem_types_length; i++)
  {
    _gem_types[i] = i; // Each index represent corresponding color equal to index's of _gem_type_color
  }

  int _excluded_gem_types_length = _gem_types_length; // This is just for naming
  int _excluded_gem_types[_excluded_gem_types_length];

  /*Filling array with -999 to prevent garbage value and showing index with -999 as didn't
  assigned number yet*/
  for (int i = 0; i < _excluded_gem_types_length; i++)
  {
    _excluded_gem_types[i] = -999;
  }
  bool _while_worked_once = false;
  for (int i = 0; i < ROWS; i++)
  {
    for (int j = 0; j < COLS; j++)
    {
      if (_gems_match_state[i][j])
      {
        bool _gem_match_neutralized = false;

        // Steps for _excluded_gem_types to change next value if swapped type also need to be excluded
        int step = 0;
        while (!_gem_match_neutralized && !_while_worked_once)
        {
          // Assuming it's as neutralized. If it's not, it will automatically set to the false
          _gem_match_neutralized = true;
          //_while_worked_once = true;

          _excluded_gem_types[step] = _entity_type[i][j];
          step++;

          // Checking next type from 0 to end where it's not excluded yet
          for (int x = 0; x < _gem_types_length; x++)
          {
            bool _found_a_new_type_that_not_excluded_yet = true;
            for (int y = 0; y < _excluded_gem_types_length; y++)
            {
              if (_gem_types[x] == _excluded_gem_types[y])
              {
                _found_a_new_type_that_not_excluded_yet = false;
                break;
              }
            }
            if (_found_a_new_type_that_not_excluded_yet)
            {
              _entity_type[i][j] = x; // Assigning found type that hasn't checked for match yet
              break;
            }
          }

          _gem_match_neutralized = !_match_availability_checker_for_gem(i, j);
        }
        // If while loop end, it means a type found that not cause match so, marking as not matched
        _gems_match_state[i][j] = false;
        // Resetting exclude types for next gems those cause matched (-999 means not assigned anything as null)
        for (int z = 0; z < _excluded_gem_types_length; z++)
        {
          _excluded_gem_types[z] = -999;
        }
      }
    }
  }
}

static void _initialize_game()
{
  // Creating randomized gams are type between 1 and 4;
  // Initializing board
  int _current_board_x_position = _board_beginning_position_x;
  int _current_board_y_position = _board_beginning_position_y;

  srand(time(NULL));

  int step = 0;
  for (int i = 0; i < ROWS; i++)
  {
    for (int j = 0; j < COLS; j++)
    {

      _cell_posx_posy[i][j][1] = _current_board_y_position;
      _cell_posx_posy[i][j][0] = _current_board_x_position;
      _current_board_x_position += _rectangle_cols_space + _rectangle_width;

      // GEMS section
      _entity_type[i][j] = rand() % 4;
      _gems_match_state[i][j] = false;
      _update_gem_position_based_on_cell(i, j); // Updating gem's properties based on it's cell

      step++;
    }

    _current_board_x_position = _board_beginning_position_x;
    _current_board_y_position += _rectangle_row_space + _rectangle_height;
  }

  // Clean junk values
  _reset_chosen_gems_state();
}

static void logic()
{
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
  {
    _gem_return_on_mouse_click();

    /*Checking if player click to a gem inside the board or clicked to illegal gem where we assume it's
      outside of the board or a empty cell where not gem available (marked as gem.type = -1);*/
    if (_returned_gem_index_on_mouse_click[0] == -999 || _returned_gem_index_on_mouse_click[0] == -1)
    {
      _reset_chosen_gems_state();
      return;
    }

    _gem_choser(_returned_gem_index_on_mouse_click[0], _returned_gem_index_on_mouse_click[1]);

    printf("First gem index:[%d][%d], Second gem index:[%d][%d]\n",
           _chosen_gems_indexs[0][0], _chosen_gems_indexs[0][1], _chosen_gems_indexs[1][0], _chosen_gems_indexs[1][1]);

    if (_chosen_gems_indexs[GEMCHOSENCOUNT - 1][1] != -999)
    {
      /* Manhattan Distance to check distance between chosen two gems if:
         1. Distance = 1: Eligable to swap those gems;
         2. Distance >= 2: Distance between two gems are more than two or chosen as diagonally;
         3. Distance = 0: Choose same gem again. */
      int _d = abs(_chosen_gems_indexs[0][0] - _chosen_gems_indexs[1][0]) +
               abs(_chosen_gems_indexs[0][1] - _chosen_gems_indexs[1][1]);

      if (_d != 1)
      {
        _reset_chosen_gems_state();
        // If the distance more than once, we assume player want to choose second gem as it's first gem to match with something else
        _gem_choser(_returned_gem_index_on_mouse_click[0], _returned_gem_index_on_mouse_click[1]);
        return;
      }

      _swap_gems_types();
      if (_check_swap_gem_requirement())
      {
        printf("_check_swap_gem_requirement turned true\n");
        _find_matches();
        _remove_matches();
        _reset_chosen_gems_state();
      }
      else
      {
        // We revert swap of gems, since they doesn't meet the require for swapping gems (E.g, the match didn't happen when player swap those gems)
        _swap_gems_types();
        _reset_chosen_gems_state();
      }
    }
  }
}

static void _refresh_the_screen()
{
  BeginDrawing();
  ClearBackground(_background_color);
  _draw_board();
  _draw_gems();
  EndDrawing();
}

static void benchmark()
{
  struct timespec start, end;
  long iterations = 100000;

  // Start the clock
  clock_gettime(CLOCK_MONOTONIC, &start);

  for (long n = 0; n < iterations; n++)
  {
    for (int i = 0; i < ROWS; i++)
    {
      for (int j = 0; j < COLS; j++)
      {
        _match_availability_checker_for_gem(i, j);
      }
    }
  }

  // Stop the clock
  clock_gettime(CLOCK_MONOTONIC, &end);

  // Calculate total time in seconds
  double time_taken = (end.tv_sec - start.tv_sec) +
                      (end.tv_nsec - start.tv_nsec) / 1e9;

  printf("Total time for %ld runs: %f seconds\n", iterations, time_taken);
  printf("Average time per run: %f microseconds\n", (time_taken / iterations) * 1e6);
}

int main(void)
{
  SetTraceLogLevel(LOG_WARNING);
  _initialize_window();

  // Preparing board and random gems
  _initialize_game();

  // Finding matches those happen when first initializing and removing them to prevent
  // free score for player
  _find_matches();
  _reinitialize_matches();

  // benchmark();

  while (!WindowShouldClose())
  {
    logic();
    if (_is_screen_dirty)
    {
      _refresh_the_screen();
    }
  }

  return 0;
}
