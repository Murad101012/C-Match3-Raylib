#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROWS 5
#define COLS 5

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

static Color _gem_type_color[] = {RED, BLUE, LIME, GOLD};

struct Gem
{
  int type;
  Color color;
  bool _is_matched;
  int order;
  int pos_y;
  int pos_x;
  int index_x;
  int index_y;
};

struct Gem gems[ROWS][COLS];

struct Gem _illegal_gem;

struct Cell
{
  int order;
  int pos_x;
  int pos_y;
  int index_x;
  int index_y;
  struct Gem gem;
};

struct Cell cell[ROWS][COLS];

struct Chosen_Gems
{
  bool gem_assigned;
  struct Gem _chosen_gem;
};

struct Chosen_Gems _chosen_gems[GEMCHOSENCOUNT];

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
      DrawRectangle(cell[i][j].pos_x, cell[i][j].pos_y, _rectangle_width, _rectangle_height, _rectangle_color);
    }
  }
}

static void _draw_gems()
{
  for (int i = 0; i < ROWS; i++)
  {
    for (int j = 0; j < COLS; j++)
    {
      if (gems[i][j].type >= 0)
      {
        DrawCircle(gems[i][j].pos_x, gems[i][j].pos_y, _circle_radius, _gem_type_color[gems[i][j].type]);
      }
    }
  }
}

static void _update_gem_position_based_on_cell(int i, int j)
{
  gems[i][j].pos_x = cell[i][j].pos_x + _rectangle_width / 2;
  gems[i][j].pos_y = cell[i][j].pos_y + _rectangle_height / 2;
  gems[i][j].order = cell[i][j].order;
  gems[i][j].index_x = cell[i][j].index_x;
  gems[i][j].index_y = cell[i][j].index_y;
}

/**
 * @brief Translates screen-space mouse coordinates into 2D grid indices.
 * * @param None (Uses Raylib's GetMousePosition internally).
 * @return void (Currently logs the clicked cell index to the console).
 * @note Currently the gap between gems are clickable but, it's intentionally left
 */
static struct Gem _gem_return_on_mouse_click()
{
  Vector2 mouse_position_normalized =
      (Vector2){GetMousePosition().x - _board_beginning_position_x,
                GetMousePosition().y - _board_beginning_position_y};

  int index_x = (int)((mouse_position_normalized.y / (float)(_rectangle_height + _rectangle_row_space)));
  int index_y = (int)((mouse_position_normalized.x / (float)(_rectangle_width + _rectangle_cols_space)));

  if (index_x < ROWS && index_y < COLS && index_x >= 0 && index_y >= 0 && mouse_position_normalized.x >= 0 && mouse_position_normalized.y >= 0)
  {
    printf("You clicked: %d\n", gems[index_x][index_y].order);
    return gems[index_x][index_y];
  }
  return _illegal_gem;
}

/**
 * @brief Process information of last chosen 2 gems to keep in array
 *
 */
static void _gem_choser(struct Gem gem)
{
  for (int i = 0; i < GEMCHOSENCOUNT; i++)
  {
    if (_chosen_gems[i].gem_assigned == false)
    {
      _chosen_gems[i]._chosen_gem = gem;
      _chosen_gems[i].gem_assigned = true;
      break;
    }
  }
}

static bool _check_swap_gem_requirement()
{
  // Check four direction
  return true; // Assuming it's true
}

static bool _check_horizontal_matches(int i, int j)
{
  if (COLS - 2 > j)
  {
    for (int y = 1; y < 3; y++)
    {
      if (gems[i][j].type != gems[i][j + y].type)
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
      if (gems[i][j].type != gems[i][j + y].type)
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
      if (gems[i][j].type != gems[i + x][j].type)
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
      if (gems[i][j].type != gems[i + x][j].type)
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
    if ((gems[i][j].type != gems[i][j - 1].type) || (gems[i][j].type != gems[i][j + 1].type))
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
    if ((gems[i][j].type != gems[i - 1][j].type) || (gems[i][j].type != gems[i + 1][j].type))
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
      if (gems[i][j]._is_matched)
      {
        gems[i][j].type = -1;
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
        if (gems[i][j].type != -1 && _check_vertical_matches(i, j))
        {
          for (int x = 0; x < 3; x++)
          {
            gems[i + x][j]._is_matched = true;
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
        if (gems[i][j].type != -1 && _check_horizontal_matches(i, j))
        {
          for (int y = 0; y < 3; y++)
          {
            gems[i][j + y]._is_matched = true;
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
static bool _match_availability_checker_for_gem(int i, int j)
{
  // Look for vertical
  printf("Vertical called for: gems[%d][%d]\n", i, j);
  if (_check_vertical_matches(i, j))
    return true;

  // Look for horizontal
  printf("Horizontal called for: gems[%d][%d]\n", i, j);
  if (_check_horizontal_matches(i, j))
    return true;

  // Look for vertical reverse
  printf("Vertical reverse called for: gems[%d][%d]\n", i, j);
  if (_check_vertical_matches_reverse(i, j))
    return true;

  // Look for horizontal reverse
  printf("Horizontal reverse called for: gems[%d][%d]\n", i, j);
  if (_check_horizontal_matches_reverse(i, j))
    return true;

  // Look for sides of gem
  printf("Left/Right called for: gems[%d][%d]\n", i, j);
  if (_check_left_right_match(i, j))
    return true;

  // Look bottom/above of gem
  printf("Up/Down called for: gems[%d][%d]\n", i, j);
  if (_check_up_down_match(i, j))
    return true;

  // If match no found
  return false;
}

/**
 * @brief Reset chosen gems
 * @note Only gem_assigned boolean reset, since _gem_choser() assumes
 * _chosen_gems.gems_chosen is not assigned if gem_assigned return false
 */
static void _reset_chosen_gems_state()
{
  for (int i = 0; i < GEMCHOSENCOUNT; i++)
  {
    _chosen_gems[i].gem_assigned = false;
  }
}

static void _swap_gems()
{
  for (int i = 0; i < ROWS; i++)
  {
    for (int j = 0; j < COLS; j++)
    {
      printf("%d", gems[i][j].type);
    }
    printf("\n");
  }

  struct Gem tempGem = _chosen_gems[0]._chosen_gem;

  // Chancing gems array index of _chosen_gems[0]
  gems[_chosen_gems[0]._chosen_gem.index_x][_chosen_gems[0]._chosen_gem.index_y] =
      _chosen_gems[1]._chosen_gem;

  // Chancing gems array index of _chosen_gems[1]
  gems[_chosen_gems[1]._chosen_gem.index_x][_chosen_gems[1]._chosen_gem.index_y] =
      tempGem;

  _update_gem_position_based_on_cell(_chosen_gems[0]._chosen_gem.index_x, _chosen_gems[0]._chosen_gem.index_y);
  _update_gem_position_based_on_cell(_chosen_gems[1]._chosen_gem.index_x, _chosen_gems[1]._chosen_gem.index_y);

  for (int i = 0; i < GEMCHOSENCOUNT; i++)
  {
    _chosen_gems[i].gem_assigned = false;
  }

  printf("\n");
  for (int i = 0; i < ROWS; i++)
  {
    for (int j = 0; j < COLS; j++)
    {
      printf("%d", gems[i][j].type);
    }
    printf("\n");
  }
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
      if (gems[i][j]._is_matched)
      {
        bool _gem_match_neutralized = false;

        // Steps for _excluded_gem_types to change next value if swapped type also need to be excluded
        int step = 0;
        while (!_gem_match_neutralized && !_while_worked_once)
        {
          // Assuming it's as neutralized. If it's not, it will automatically set to the false
          _gem_match_neutralized = true;
          //_while_worked_once = true;

          _excluded_gem_types[step] = gems[i][j].type;
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
              printf("Chancing %d to %d type for gems[%d][%d]\n", gems[i][j].type, x, i, j);
              gems[i][j].type = x; // Assigning found type that hasn't checked for match yet
              break;
            }
          }

          _gem_match_neutralized = !_match_availability_checker_for_gem(i, j);
        }
        // If while loop end, it means a type found that not cause match so, marking as not matched
        gems[i][j]._is_matched = false;
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

      // BOARD section
      cell[i][j].index_x = i;
      cell[i][j].index_y = j;
      cell[i][j].order = step;

      cell[i][j].pos_y = _current_board_y_position;
      cell[i][j].pos_x = _current_board_x_position;
      _current_board_x_position += _rectangle_cols_space + _rectangle_width;

      // GEMS section
      gems[i][j].type = rand() % 4;
      gems[i][j].color = _gem_type_color[gems[i][j].type];
      gems[i][j]._is_matched = false;
      _update_gem_position_based_on_cell(i, j); // Updating gem's properties based on it's cell

      step++;
    }

    _current_board_x_position = _board_beginning_position_x;
    _current_board_y_position += _rectangle_row_space + _rectangle_height;
  }

  // It's for ouside of the board when player click there
  _illegal_gem.type = -999;

  // Clean junk values
  for (int i = 0; i < GEMCHOSENCOUNT; i++)
  {
    _chosen_gems[i].gem_assigned = false;
  }
}

static void logic()
{
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
  {
    struct Gem gem = _gem_return_on_mouse_click();
    /*Checking if player click to a gem inside the board or clicked to illegal gem where we assume it's
    outside of the board or a empty cell where not gem available (marked as gem.type = -1);*/
    if (gem.type == -999 || gem.type == -1)
    {
      _reset_chosen_gems_state();
      return;
    }

    _gem_choser(gem);
    if (_chosen_gems[GEMCHOSENCOUNT - 1].gem_assigned == true)
    {
      /* Manhattan Distance to check distance between chosen two gems if:
         1. Distance = 1: Eligable to swap those gems;
         2. Distance >= 2: Distance between two gems are more than two or chosen as diagonally;
         3. Distance = 0: Choose same gem again. */
      int _d = abs(_chosen_gems[0]._chosen_gem.index_x - _chosen_gems[1]._chosen_gem.index_x) +
               abs(_chosen_gems[0]._chosen_gem.index_y - _chosen_gems[1]._chosen_gem.index_y);

      if (_d != 1)
      {
        _reset_chosen_gems_state();
        _gem_choser(gem);
        return;
      }

      for (int i = 0; i < GEMCHOSENCOUNT; i++)
      {
        printf("%d gem: %d\n", i, _chosen_gems[i]._chosen_gem.order);
      }
      if (_check_swap_gem_requirement())
      {
        _swap_gems();
        _find_matches();
        _remove_matches();
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
