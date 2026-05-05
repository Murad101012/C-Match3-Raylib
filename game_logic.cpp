#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "game_logic.h"
#include "game_config.h"
#include "render.h"
#include "input_gem_return.h"
#include "tools.h"

// Properties of general entities
static int16_t _entity_type[MAX_ROWS][MAX_COLS];
static Vec2I _entity_posx_posy_absolute[MAX_ROWS][MAX_COLS];
static Vec2I _entity_posx_posy_animation[MAX_ROWS][MAX_COLS];

// Properties of specifially gem entity
static bool _entity_match_state[MAX_ROWS][MAX_COLS];

static bool _entities_in_animation[MAX_ROWS][MAX_COLS];

// Properties of Cell
static Vec2I _cell_posx_posy[MAX_ROWS][MAX_COLS];

static Vec2I _chosen_gems_indexs[GEM_CHOSEN_COUNT];

static const uint8_t pixel_move_step_animation = 1;

static void _after_gem_animation_end();

static sequence_object *checking_if_all_animation_states_are_false = nullptr;

void set_entity_absolute_position(Vec2I entity_position, Vec2I entity_position_new)
{
  if (entity_position.x < rows && entity_position.y < cols)
  {
    _entity_posx_posy_absolute[entity_position.x][entity_position.y].x =
        entity_position_new.x;
    _entity_posx_posy_absolute[entity_position.x][entity_position.y].y =
        entity_position_new.y;
    return;
  }
  Serial.printf("game_logic.c: row: %d / col: %d cause overflow, can't set "
                "entity_position",
                entity_position.x, entity_position.y);
}

Vec2I get_entity_absolute_position(Vec2I entity_position)
{
  if (entity_position.x < rows && entity_position.y < cols)
  {
    return (Vec2I){_entity_posx_posy_absolute[entity_position.x][entity_position.y].x,
                   _entity_posx_posy_absolute[entity_position.x][entity_position.y].y};
  }

  Serial.printf("game_logic.c: row: %d / col: %d cause overflow, can't return "
                "entity_position",
                entity_position.x, entity_position.y);
  return (Vec2I){-1, -1};
}

void set_entity_animation_position(Vec2I entity_position, Vec2I entity_position_new)
{
  if (entity_position.x < rows && entity_position.y < cols)
  {
    _entity_posx_posy_animation[entity_position.x][entity_position.y].x =
        entity_position_new.x;
    _entity_posx_posy_animation[entity_position.x][entity_position.y].y =
        entity_position_new.y;
    return;
  }
  Serial.printf("game_logic.c: row: %d / col: %d cause overflow, can't set "
                "entity_position",
                entity_position.x, entity_position.y);
}

Vec2I get_entity_animation_position(Vec2I entity_position)
{
  if (entity_position.x < rows && entity_position.y < cols)
  {
    return (Vec2I){_entity_posx_posy_animation[entity_position.x][entity_position.y].x,
                   _entity_posx_posy_animation[entity_position.x][entity_position.y].y};
  }

  Serial.printf("game_logic.c: row: %d / col: %d cause overflow, can't return "
                "entity_position",
                entity_position.x, entity_position.y);
  return (Vec2I){-1, -1};
}

void set_cell_position(Vec2I cell_index, Vec2I cell_position_new)
{
  if (cell_index.x < rows && cell_index.y < cols)
  {
    _cell_posx_posy[cell_index.x][cell_index.y].x = cell_position_new.x;
    _cell_posx_posy[cell_index.x][cell_index.y].y = cell_position_new.y;
    return;
  }
  Serial.printf(
      "game_logic.c: row: %d / col: %d cause overflow, can't set cell_position",
      cell_index.x, cell_index.y);
}

void set_animation_state(Vec2I entity_index, bool state)
{
  _entities_in_animation[entity_index.x][entity_index.y] = state;
}

bool get_animation_state(Vec2I entity_index)
{
  return _entities_in_animation[entity_index.x][entity_index.y];
}

bool check_all_animation_states_are_false()
{
  for (uint8_t i = 0; i < rows; i++)
  {
    for (uint8_t j = 0; j < cols; j++)
    {
      if (get_animation_state(Vec2I{i, j}))
      {
        return false;
      }
    }
  }
  return true;
}

void check_all_animation_states_are_false_for_sequence_object(sequence_object *self)
{
  if (!self->auto_kill_method_value)
  {
    self->auto_kill_method_value = check_all_animation_states_are_false();
    if (self->auto_kill_method_value)
    {
      checking_if_all_animation_states_are_false = nullptr;
    }
  }
}

/**
 * @brief Set the entity position by pixel animation object
 * custom_int_array mapping:
 * [0,1] = Entity Index (Grid X, Y)
 * [2,3] = Target Position (Pixel X, Y)
 * [4]   = Elapsed Time (ms)
 * [5]   = Target Time to reach (ms) - Added as per your request
 * @param self
 */
void set_entity_position_by_pixel_animation(sequence_object *self)
{
  Vec2I entity_index = {self->custom_int_array[0], self->custom_int_array[1]};
  Vec2I target = {self->custom_int_array[2], self->custom_int_array[3]};

  // Guard against division by zero if target time isn't set
  float target_time = (float)self->custom_int_array[5];
  if (target_time <= 0)
    target_time = 200.0f;

  if (entity_index.x < rows && entity_index.y < cols)
  {
    // 1. Increment elapsed time by the object's delay
    self->custom_int_array[4] += self->delay;

    // 2. Calculate t (normalized 0.0 to 1.0)
    float t = (float)self->custom_int_array[4] / target_time;

    // 3. Get current visual position to calculate the delta
    Vec2I current_visual = get_entity_animation_position(entity_index);

    // 4. Apply your Lerp function for both axes
    set_entity_animation_position(entity_index, Vec2I{Lerp((float)current_visual.x, (float)target.x, t), Lerp((float)current_visual.y, (float)target.y, t)});
    // Serial.printf("t: %f\n", t);

    // 5. Finalize when t reaches 1.0 (offset to 0.97f)
    if (t >= 1.0f)
    {
      // Serial.printf("t is: %, so moved the position to target\n", t);
      set_entity_animation_position(entity_index, target);
      set_animation_state(Vec2I{entity_index.x, entity_index.y}, false);
      _refresh_the_screen_dirty();
      _remove_entity_dirty(entity_index);

      // Signal CONDITION_MET_AUTO_KILL_SEQUENCE
      self->auto_kill_method_value = true;
    }
    else
    {
      set_animation_state(Vec2I{entity_index.x, entity_index.y}, true);
    }
    return;
  }

  Serial.printf(
      "game_logic.c: row: %d / col: %d cause overflow, can't set cell_position",
      entity_index.x, entity_index.y);
}

Vec2I get_cell_position(Vec2I cell_position)
{
  if (cell_position.x < rows && cell_position.y < cols)
  {
    return (Vec2I){_cell_posx_posy[cell_position.x][cell_position.y].x,
                   _cell_posx_posy[cell_position.x][cell_position.y].y};
  }

  Serial.printf("game_logic.c: row: %d / col: %d cause overflow, can't return "
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
  Serial.printf("game_logic.c: row: %d / col: %d cause overflow, can't return "
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
  Serial.printf(
      "game_logic.c: row: %d / col: %d cause overflow, can't set entity_type",
      entity_index.x, entity_index.y);
}

bool get_entity_match_state(Vec2I entity_index)
{
  if (entity_index.x < rows && entity_index.y < cols)
  {
    return _entity_match_state[entity_index.x][entity_index.y];
  }
  Serial.printf("game_logic.c: row: %d / col: %d cause overflow, can't return "
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
  Serial.printf(
      "game_logic.c: row: %d / col: %d cause overflow, can't set entity_state",
      entity_index.x, entity_index.y);
}

void _update_entity_position_based_on_cell(int i, int j)
{
  Vec2I cell_position_cache = get_cell_position(Vec2I(i, j));
  set_entity_absolute_position((Vec2I){i, j}, (Vec2I){cell_position_cache.x + _rectangle_width / 4, cell_position_cache.y + _rectangle_height / 4});
  set_entity_animation_position((Vec2I){i, j}, (Vec2I){cell_position_cache.x + _rectangle_width / 4, cell_position_cache.y + _rectangle_height / 4});
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
  
  if (_chosen_gems_indexs[0].x != -999 && _chosen_gems_indexs[0].x != -1)
  {
    _draw_selected_entity_frame(_chosen_gems_indexs[0]);
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
  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      if (get_entity_match_state((Vec2I){i, j}))
      {
        set_entity_type((Vec2I){i, j}, -1);
        set_entity_match_state((Vec2I){i, j}, false);
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
static bool _find_matches()
{
  bool match_found = false;
  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      if (rows - 2 > i)
      {
        // Look for vertical
        if (get_entity_type((Vec2I){i, j}) != -1 && _check_vertical_matches(i, j))
        {
          if (!match_found)
          {
            match_found = true;
          }
          for (int x = 0; x < 3; x++)
          {
            set_entity_match_state((Vec2I){i + x, j}, true);
          }
        }
      }
      if (cols - 2 > j)
      {
        // Look for horizontal
        if (get_entity_type((Vec2I){i, j}) != -1 && _check_horizontal_matches(i, j))
        {
          if (!match_found)
          {
            match_found = true;
          }
          for (int y = 0; y < 3; y++)
          {
            set_entity_match_state((Vec2I){i, j + y}, true);
          }
        }
      }
    }
  }
  return match_found;
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

static void _swap_entity_position_animation(Vec2I first_entity_index, Vec2I second_entity_index)
{
  Vec2I first_gem_position = get_entity_animation_position(first_entity_index);
  Vec2I second_gem_position = get_entity_animation_position(second_entity_index);
  _add_to_sequence(sequence_object{.delay = targeted_frame_rate, .function_type = FUNC_ITSELF, .with_itself = &set_entity_position_by_pixel_animation, .auto_kill_method_enum_type = CONDITION_MET_AUTO_KILL_SEQUENCE, .custom_int_array = {first_entity_index.x, first_entity_index.y, second_gem_position.x, second_gem_position.y, 0, gem_swap_animation_time_to_complete}});
  _add_to_sequence(sequence_object{.delay = targeted_frame_rate, .function_type = FUNC_ITSELF, .with_itself = &set_entity_position_by_pixel_animation, .auto_kill_method_enum_type = CONDITION_MET_AUTO_KILL_SEQUENCE, .custom_int_array = {second_entity_index.x, second_entity_index.y, first_gem_position.x, first_gem_position.y, 0, gem_swap_animation_time_to_complete}});
  _add_entity_dirty(first_entity_index);
  _add_entity_dirty(second_entity_index);

  if (checking_if_all_animation_states_are_false == nullptr || !checking_if_all_animation_states_are_false->active)
  {
    checking_if_all_animation_states_are_false = _add_to_sequence(sequence_object{.delay = targeted_frame_rate, .function_type = FUNC_ITSELF, .with_itself = &check_all_animation_states_are_false_for_sequence_object, .auto_kill_method_enum_type = CONDITION_MET_AUTO_KILL_SEQUENCE, .on_complete_callbacks = {_after_gem_animation_end}, .callback_count = 1});
  }

  // Serial.printf("Swapping entities with animation: 1)entity[%d][%d] - type: %d, 2)entity[%d][%d] - type: %d\n", first_entity_index.x, first_entity_index.y, get_entity_type(Vec2I{first_entity_index.x, first_entity_index.y}), second_entity_index.x, second_entity_index.y, get_entity_type(Vec2I{second_entity_index.x, second_entity_index.y}));
}

static void _swap_entity_types_and_positions(Vec2I first_entity_index, Vec2I second_entity_index)
{
  // Taking the type of first element
  int16_t tempType = get_entity_type(first_entity_index);

  // Then swapping them
  set_entity_type(first_entity_index, get_entity_type(second_entity_index));

  set_entity_type(second_entity_index, tempType);

  // Then we change their position
  //  Taking the position of first element
  Vec2I tempPosition = get_entity_absolute_position(first_entity_index);

  // Then swapping them
  set_entity_animation_position(first_entity_index, get_entity_absolute_position(second_entity_index));

  set_entity_animation_position(second_entity_index, tempPosition);

  _swap_entity_position_animation(first_entity_index, second_entity_index);
}

static void _gem_gravity()
{
  current_game_state = GEM_FALLING;

  Vec2I found_empty_cell_index;
  int8_t length_empty_cells_vertically = 0;
  int8_t available_length_to_up = 0;
  Vec2I empty_cell_at_most_bottom;
  bool found_an_entity_to_animate_for_gravity = false;

  // First we find empty cells (Entity type equals to -1)
  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      /*Checks: I)Is it empty? II)Is it not in border (Since in above there are not any entity to drag)
      III)If there have an entity to drag (If it has an emptity cell, means we can't reference this empty cell for offsetting the entities above)*/
      if (get_entity_type((Vec2I){i, j}) == -1 && i > 0 && get_entity_type((Vec2I){i - 1, j}) != -1)
      {
        found_empty_cell_index.x = i;
        found_empty_cell_index.y = j;
        found_an_entity_to_animate_for_gravity = true;

        // We will look the vertical direction(down) to length of empty cells in column
        for (int x = 0; x < rows; x++)
        {
          if (found_empty_cell_index.x + x < rows)
          {
            if (get_entity_type((Vec2I){found_empty_cell_index.x + x, found_empty_cell_index.y}) == -1)
            {
              length_empty_cells_vertically++;
            }
            else
            {
              break;
            }
          }
          else
          {
            break;
          }
        }
        empty_cell_at_most_bottom.x = found_empty_cell_index.x + (length_empty_cells_vertically - 1);
        empty_cell_at_most_bottom.y = found_empty_cell_index.y;
        // Serial.printf("Found length for empty cell at: [%d][%d], is: %d\n", found_empty_cell_index.x, found_empty_cell_index.y, length_empty_cells_vertically);

        // We will look up (from the found empty cell) find out how much gem it satisfy to put them down. If we reach to the border we stop
        for (int x = found_empty_cell_index.x - 1; x >= 0; x--)
        {
          if (get_entity_type((Vec2I){found_empty_cell_index.x - 1 - available_length_to_up, found_empty_cell_index.y}) != -1)
            available_length_to_up++;
          else
            break;
        }

        // Serial.printf("\nfound_empty_cell_index: [%d][%d]\nlength_empty_cells_vertically: %d\navailable_length_to_up: %d\nempty_cell_at_most_bottom: [%d][%d]", found_empty_cell_index.x, found_empty_cell_index.y, length_empty_cells_vertically, available_length_to_up, empty_cell_at_most_bottom.x, empty_cell_at_most_bottom.y);

        // After we found how much gem on above available to offset to down, we will begin to move them bottom
        for (int x = 0; x < available_length_to_up; x++)
        {
          // Serial.print("_gem_gravity() calling swapping for next gems: ");
          _swap_entity_types_and_positions((Vec2I){empty_cell_at_most_bottom.x /*We go to the most bottom of empty cell in vertical column*/ - x /*And going up in each time*/, empty_cell_at_most_bottom.y}, (Vec2I){empty_cell_at_most_bottom.x - x - length_empty_cells_vertically, empty_cell_at_most_bottom.y});
        }

        // At the end since we proceed the empty cell, we reset the values
        length_empty_cells_vertically = 0;
        available_length_to_up = 0;
      }
    }
  }

  if (!found_an_entity_to_animate_for_gravity)
  {
    _after_gem_animation_end();
  }
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

static void _refill_empty_cell()
{
  current_game_state = GEM_REFILLING;
  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      if (get_entity_type(Vec2I{i, j}) == -1)
      {
        set_entity_type((Vec2I){i, j}, rand() % 4);
        set_entity_match_state((Vec2I){i, j}, false);
        _update_entity_position_based_on_cell(i, j); // Updating gem's properties based on it's cell

        Vec2I entity_beginning_animation_position = get_entity_absolute_position(Vec2I{i, j});
        entity_beginning_animation_position.y = -100;

        set_entity_animation_position(Vec2I{i, j}, entity_beginning_animation_position);

        Vec2I entity_end_animation_position = get_entity_absolute_position(Vec2I{i, j});

        _add_to_sequence(sequence_object{.delay = targeted_frame_rate, .function_type = FUNC_ITSELF, .with_itself = &set_entity_position_by_pixel_animation, .auto_kill_method_enum_type = CONDITION_MET_AUTO_KILL_SEQUENCE, .custom_int_array = {i, j, entity_end_animation_position.x, entity_end_animation_position.y, 0, gem_swap_animation_time_to_complete}});
        _add_entity_dirty((Vec2I){i, j});
        if (checking_if_all_animation_states_are_false == nullptr || !checking_if_all_animation_states_are_false->active || checking_if_all_animation_states_are_false->auto_kill_method_value == true)
        {
          checking_if_all_animation_states_are_false = _add_to_sequence(sequence_object{.delay = targeted_frame_rate, .function_type = FUNC_ITSELF, .with_itself = &check_all_animation_states_are_false_for_sequence_object, .auto_kill_method_enum_type = CONDITION_MET_AUTO_KILL_SEQUENCE, .on_complete_callbacks = {_after_gem_animation_end}, .callback_count = 1});
        }
      }
    }
  }
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
      _update_entity_position_based_on_cell(i, j); // Updating gem's properties based on it's cell

      // Clearing the junk values
      _entities_in_animation[i][j] = false;
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

static void _after_gem_animation_end()
{
  if (current_game_state == GEM_SWAPPING_BY_PLAYER)
  {
    // Serial.print("GEM_SWAPPING_BY_PLAYER is true\n");
    if (_check_swap_gem_requirement())
    {
      // Serial.print("Found matched gems when player swapped the gems\n");
      _find_matches();
      _remove_matches();
      _gem_gravity();
      _refill_empty_cell();
      _reset_chosen_gems_state();
    }
    else
    {
      // We revert swap of gems, since they doesn't meet the require for
      // swapping gems (E.g, the match didn't happen when player swap those
      // gems)
      _swap_entity_types_and_positions(_chosen_gems_indexs[0], _chosen_gems_indexs[1]);
      current_game_state = INPUT_WAITING;
      _reset_chosen_gems_state();
      // Serial.print("Couldn't find matches, reverted\n");
    }
  }
  else if (current_game_state == GEM_FALLING || current_game_state == GEM_REFILLING)
  {
    // Serial.print("GEM_FALLING is true\n");
    if (_find_matches())
    {
      _remove_matches();
      _gem_gravity();
      _refill_empty_cell();
      // Serial.print("Find new matched after player swapped and gem's down\n");
    }
    else
    {
      // Serial.print("Couldn't find new matches after player swapped gem and gems falled\n");
      current_game_state = INPUT_WAITING;
    }
  }
}

void logic()
{
  touch_input_screen.read();

  if (touch_input_screen.isTouched && current_game_state == INPUT_WAITING)
  {
    Vec2I _returned_entity_index = _entity_return_on_mouse_click();

    /*Checking if player click to a gem inside the board or clicked to illegal
      gem where we assume it's outside of the board or a empty cell where not
      gem available (marked as gem.type = -1);*/
    if (_returned_entity_index.x == -999 ||
        _returned_entity_index.x == -1 || get_entity_type(_returned_entity_index) == -1 || get_entity_type(_returned_entity_index) == -999)
    {
      _reset_chosen_gems_state();
      return;
    }

    _gem_choser(_returned_entity_index.x, _returned_entity_index.y);

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

      current_game_state = GEM_SWAPPING_BY_PLAYER;
      _swap_entity_types_and_positions(_chosen_gems_indexs[0], _chosen_gems_indexs[1]);
    }
  }
}