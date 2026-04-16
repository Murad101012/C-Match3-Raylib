#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "game_logic.h"
#include "game_config.h"
#include "render.h"
#include "input_gem_return.h"

// Properties of general entities
static int16_t _entity_type[MAX_ROWS][MAX_COLS];
static int16_t _entity_posx_posy[MAX_ROWS][MAX_COLS][2 /*ROWS&COLS*/];

// Properties of specifially gem entity
static bool _entity_match_state[MAX_ROWS][MAX_COLS];

// Properties of Cell
static int16_t _cell_posx_posy[MAX_ROWS][MAX_COLS][2 /*ROWS&COLS*/];

static int16_t _returned_entity_type_on_mouse_click[2];

static Vec2I _chosen_gems_indexs[GEM_CHOSEN_COUNT];

void set_entity_position(Vec2I entity_position, Vec2I entity_position_new)
{
  if (entity_position.x < rows && entity_position.y < cols)
  {
    _entity_posx_posy[entity_position.x][entity_position.y][0] =
        entity_position_new.x;
    _entity_posx_posy[entity_position.x][entity_position.y][1] =
        entity_position_new.y;
    return;
  }
  printf("game_logic.c: row: %d / col: %d cause overflow, can't set "
         "entity_position",
         entity_position.x, entity_position.y);
}

Vec2I get_entity_position(Vec2I entity_position)
{
  if (entity_position.x < rows && entity_position.y < cols)
  {
    return (Vec2I){_entity_posx_posy[entity_position.x][entity_position.y][0],
                   _entity_posx_posy[entity_position.x][entity_position.y][1]};
  }

  printf("game_logic.c: row: %d / col: %d cause overflow, can't return "
         "entity_position",
         entity_position.x, entity_position.y);
  return (Vec2I){-1, -1};
}

void set_cell_position(Vec2I cell_index, Vec2I cell_position_new)
{
  if (cell_index.x < rows && cell_index.y < cols)
  {
    _cell_posx_posy[cell_index.x][cell_index.y][0] = cell_position_new.x;
    _cell_posx_posy[cell_index.x][cell_index.y][1] = cell_position_new.y;
    return;
  }
  printf(
      "game_logic.c: row: %d / col: %d cause overflow, can't set cell_position",
      cell_index.x, cell_index.y);
}

Vec2I get_cell_position(Vec2I cell_position)
{
  if (cell_position.x < rows && cell_position.y < cols)
  {
    return (Vec2I){_cell_posx_posy[cell_position.x][cell_position.y][0],
                   _cell_posx_posy[cell_position.x][cell_position.y][1]};
  }

  printf("game_logic.c: row: %d / col: %d cause overflow, can't return "
         "cell_position",
         cell_position.x, cell_position.y);
  return (Vec2I){-1, -1};
}

int16_t get_entity_type(Vec2I entity_index)
{
  if (entity_index.x < rows && entity_index.y < cols)
  {
    return _entity_type[entity_index.x][entity_index.y];
  }
  printf("game_logic.c: row: %d / col: %d cause overflow, can't return "
         "entity_type",
         entity_index.x, entity_index.y);
  return -1;
}

void set_entity_type(Vec2I entity_index, int16_t type_new)
{
  if (entity_index.x < rows && entity_index.y < cols)
  {
    _entity_type[entity_index.x][entity_index.y] = type_new;
    return;
  }
  printf(
      "game_logic.c: row: %d / col: %d cause overflow, can't set entity_type",
      entity_index.x, entity_index.y);
}

bool get_entity_match_state(Vec2I entity_index)
{
  if (entity_index.x < rows && entity_index.y < cols)
  {
    return _entity_match_state[entity_index.x][entity_index.y];
  }
  printf("game_logic.c: row: %d / col: %d cause overflow, can't return "
         "entity_match_state",
         entity_index.x, entity_index.y);
  return -1;
}

void set_entity_match_state(Vec2I entity_index, bool state_new)
{
  if (entity_index.x < rows && entity_index.y < cols)
  {
    _entity_match_state[entity_index.x][entity_index.y] = state_new;
    return;
  }
  printf(
      "game_logic.c: row: %d / col: %d cause overflow, can't set entity_state",
      entity_index.x, entity_index.y);
}

void _update_gem_position_based_on_cell(int i, int j)
{
  Vec2I cell_position_cache = get_cell_position(Vec2I(i, j));
  set_entity_position((Vec2I){i, j}, (Vec2I){cell_position_cache.x + _rectangle_width / 4, cell_position_cache.y + _rectangle_height / 4});
}

/**
 * @brief Keep index (Row/Cols) of gem's type
 *
 */
static void _gem_choser(int row, int col)
{
  for (int i = 0; i < GEM_CHOSEN_COUNT; i++)
  {
    // We only check rows since, if it's -999, cols also must be -999
    if (_chosen_gems_indexs[i].x == -999)
    {
      _chosen_gems_indexs[i].x = row;
      _chosen_gems_indexs[i].y = col;
      break;
    }
  }
}

static bool _check_horizontal_matches(int i, int j)
{
  if (MAX_COLS - 2 > j)
  {
    for (int y = 1; y < 3; y++)
    {

      if (get_entity_type((Vec2I){i, j}) != get_entity_type((Vec2I){i, j + y}))
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
      if (get_entity_type((Vec2I){i, j}) != get_entity_type((Vec2I){i, j + y}))
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
  if (MAX_ROWS - 2 > i)
  {
    for (int x = 1; x < 3; x++)
    {
      if (get_entity_type((Vec2I){i, j}) != get_entity_type((Vec2I){i + x, j}))
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
      if (get_entity_type((Vec2I){i, j}) != get_entity_type((Vec2I){i + x, j}))
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
  if (MAX_COLS - 1 > j && j >= 1)
  {
    if ((get_entity_type((Vec2I){i, j}) != get_entity_type((Vec2I){i, j - 1})) ||
        (get_entity_type((Vec2I){i, j}) != get_entity_type((Vec2I){i, j + 1})))
    {
      return false;
    }
    return true;
  }
  return false;
}

static bool _check_up_down_match(int i, int j)
{
  if (MAX_ROWS - 1 > i && i >= 1)
  {
    if ((get_entity_type((Vec2I){i, j}) != get_entity_type((Vec2I){i - 1, j})) ||
        (get_entity_type((Vec2I){i, j}) != get_entity_type((Vec2I){i + 1, j})))
    {
      return false;
    }
    return true;
  }
  return false;
}

static void _remove_matches()
{
  for (int i = 0; i < MAX_ROWS; i++)
  {
    for (int j = 0; j < MAX_COLS; j++)
    {
      if (get_entity_match_state((Vec2I){i, j}))
      {
        set_entity_type((Vec2I){i, j}, -1);
      }
    }
  }
}

/**
 * @brief Cheap version match finder for all gems and mark them _is_matched =
 * true, which is CPU friendly
 * @note It's required to use only when the gem's position, types or any change
 * on board/gems will not happen until the _find_matches() finish
 */
static void _find_matches()
{
  bool match_found = false;
  for (int i = 0; i < MAX_ROWS; i++)
  {
    for (int j = 0; j < MAX_COLS; j++)
    {
      if (MAX_ROWS - 2 > i)
      {
        // Look for vertical
        if (get_entity_type((Vec2I){i, j}) != -1 && _check_vertical_matches(i, j))
        {
          for (int x = 0; x < 3; x++)
          {
            set_entity_match_state((Vec2I){i + x, j}, true);
            if (!match_found)
            {
              match_found = true;
            }
          }
        }
      }
      if (MAX_COLS - 2 > j)
      {
        // Look for horizontal
        if (get_entity_type((Vec2I){i, j}) != -1 && _check_horizontal_matches(i, j))
        {
          for (int y = 0; y < 3; y++)
          {
            set_entity_match_state((Vec2I){i, j + y}, true);
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
  int16_t temp_gem_type_one = get_entity_type((Vec2I){_chosen_gems_indexs[0].x, _chosen_gems_indexs[0].y});
  int16_t temp_gem_type_two = get_entity_type((Vec2I){_chosen_gems_indexs[1].x, _chosen_gems_indexs[1].y});

  // if returned type is empty (-1) or illegal (-999) we return as false for swap requirement not satisfied
  if (temp_gem_type_one == -1 || temp_gem_type_one == -999 || temp_gem_type_two == -1 || temp_gem_type_two == -999)
    return false;

  // Before checking all gems, it do simple check for those two selected gems
  return (_match_availability_checker_for_gem(_chosen_gems_indexs[0].x,
                                              _chosen_gems_indexs[0].y) ||
          _match_availability_checker_for_gem(_chosen_gems_indexs[1].x,
                                              _chosen_gems_indexs[1].y));
}

/**
 * @brief Reset chosen gems
 */
static void _reset_chosen_gems_state()
{
  for (int i = 0; i < GEM_CHOSEN_COUNT; i++)
  {
    _chosen_gems_indexs[i].x = -999;
    _chosen_gems_indexs[i].y = -999;
  }
}

static void _swap_gems_types()
{
  // Taking the type of first element
  int16_t tempType =
      get_entity_type((Vec2I){_chosen_gems_indexs[0].x, _chosen_gems_indexs[0].y});

  // Then swapping them
  set_entity_type((Vec2I){_chosen_gems_indexs[0].x, _chosen_gems_indexs[0].y},
                  get_entity_type((Vec2I){_chosen_gems_indexs[1].x, _chosen_gems_indexs[1].y}));

  set_entity_type((Vec2I){_chosen_gems_indexs[1].x, _chosen_gems_indexs[1].y}, tempType);
}

/**
 * @brief Find matches and change it's type based on horizontal/vertical check
 *
 */
static void _reinitialize_matches()
{
  // We collect types those are cause match in four direction by eliminating
  int _gem_types_length = GEM_TYPE_LENGTH;
  /*Since comparing between type and color not possible and originally
  each _gem_type_color's index equal to type (e.g: _gem_type_color[2] is same as
  type = 2) creating an index based color from 0 to 1 represinting color's
  index*/
  int _gem_types[_gem_types_length];
  for (int i = 0; i < _gem_types_length; i++)
  {
    _gem_types[i] = i; // Each index represent corresponding color equal to
                       // index's of _gem_type_color
  }

  int _excluded_gem_types_length = _gem_types_length; // This is just for naming
  int _excluded_gem_types[_excluded_gem_types_length];

  /*Filling array with -999 to prevent garbage value and showing index with -999
  as didn't assigned number yet*/
  for (int i = 0; i < _excluded_gem_types_length; i++)
  {
    _excluded_gem_types[i] = -999;
  }
  bool _while_worked_once = false;
  for (int i = 0; i < MAX_ROWS; i++)
  {
    for (int j = 0; j < MAX_COLS; j++)
    {
      if (get_entity_match_state((Vec2I){i, j}))
      {
        bool _gem_match_neutralized = false;

        // Steps for _excluded_gem_types to change next value if swapped type
        // also need to be excluded
        int step = 0;
        while (!_gem_match_neutralized && !_while_worked_once)
        {
          // Assuming it's as neutralized. If it's not, it will automatically
          // set to the false
          _gem_match_neutralized = true;
          //_while_worked_once = true;

          _excluded_gem_types[step] = get_entity_type((Vec2I){i, j});
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
              set_entity_type((Vec2I){i, j}, x); // Assigning found type that hasn't checked for match yet
              break;
            }
          }

          _gem_match_neutralized = !_match_availability_checker_for_gem(i, j);
        }
        // If while loop end, it means a type found that not cause match so,
        // marking as not matched
        set_entity_match_state((Vec2I){i, j}, false);

        // Resetting exclude types for next gems those cause matched (-999 means
        // not assigned anything as null)
        for (int z = 0; z < _excluded_gem_types_length; z++)
        {
          _excluded_gem_types[z] = -999;
        }
      }
    }
  }
}

void _initialize_game()
{
  // Creating randomized gams are type between 1 and 4;
  // Initializing board
  int _current_board_x_position = _board_beginning_position_x;
  int _current_board_y_position = _board_beginning_position_y;

  srand(esp_random());

  for (int i = 0; i < MAX_ROWS; i++)
  {
    for (int j = 0; j < MAX_COLS; j++)
    {
      // CELLS section
      set_cell_position((Vec2I){i, j}, (Vec2I){_current_board_x_position, _current_board_y_position});
      _current_board_x_position += _rectangle_cols_space + _rectangle_width;

      // GEMS section
      set_entity_type((Vec2I){i, j}, rand() % 4);
      set_entity_match_state((Vec2I){i, j}, false);
      _update_gem_position_based_on_cell(i, j); // Updating gem's properties based on it's cell
    }

    _current_board_x_position = _board_beginning_position_x;
    _current_board_y_position += _rectangle_row_space + _rectangle_height;
  }

  // Clean junk values
  _reset_chosen_gems_state();

  // Finding matches those happen when first initializing and removing them to
  // prevent free score for player
  _find_matches();
  _reinitialize_matches();
}

void logic()
{
  touch_input_screen.read();

  if (touch_input_screen.isTouched)
  {
    Vec2I _returned_entity_index = _entity_return_on_mouse_click();

    /*Checking if player click to a gem inside the board or clicked to illegal
      gem where we assume it's outside of the board or a empty cell where not
      gem available (marked as gem.type = -1);*/
    if (_returned_entity_index.x == -999 ||
        _returned_entity_index.x == -1)
    {
      _reset_chosen_gems_state();
      return;
    }

    _gem_choser(_returned_entity_index.x, _returned_entity_index.y);

    printf("First gem index:[%d][%d], Second gem index:[%d][%d]\n",
           _chosen_gems_indexs[0].x, _chosen_gems_indexs[0].y,
           _chosen_gems_indexs[1].x, _chosen_gems_indexs[1].y);

    if (_chosen_gems_indexs[GEM_CHOSEN_COUNT - 1].y != -999)
    {
      /* Manhattan Distance to check distance between chosen two gems if:
         1. Distance = 1: Eligable to swap those gems;
         2. Distance >= 2: Distance between two gems are more than two or chosen
         as diagonally;
         3. Distance = 0: Choose same gem again. */
      int _d = abs(_chosen_gems_indexs[0].x - _chosen_gems_indexs[1].x) +
               abs(_chosen_gems_indexs[0].y - _chosen_gems_indexs[1].y);

      if (_d != 1)
      {
        _reset_chosen_gems_state();
        // If the distance more than once, we assume player want to choose
        // second gem as it's first gem to match with something else
        _gem_choser(_returned_entity_index.x,
                    _returned_entity_index.y);
        return;
      }

      _swap_gems_types();
      if (_check_swap_gem_requirement())
      {
        printf("_check_swap_gem_requirement turned true\n");
        _find_matches();
        _remove_matches();
        _reset_chosen_gems_state();
        _refresh_the_screen();
      }
      else
      {
        // We revert swap of gems, since they doesn't meet the require for
        // swapping gems (E.g, the match didn't happen when player swap those
        // gems)
        _swap_gems_types();
        _reset_chosen_gems_state();
      }
    }
  }
}