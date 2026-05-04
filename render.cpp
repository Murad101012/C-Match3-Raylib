#include "game_config.h"
#include "game_logic.h"
#include "render.h"
#include "src/display.h"
#include <esp_heap_caps.h>
#include <string.h>
#include "tools.h"

// Prevents calling get_ function twice for both x and y
static Vec2I cache_entity;
static Vec2I cache_cell;
static int16_t cache_entity_type;
static bool _is_screen_dirty = true;

uint16_t *test_buffer = nullptr;
static Vec2I _entity_dirty[MAX_ROWS * MAX_COLS];
static uint16_t count_dirty_entities = 0;

void _draw_cell(Vec2I cell_index)
{
  cache_cell = get_cell_position(cell_index);
  const uint16_t *cell = cell_table[0];
  int step = 0;

  for (int y = cache_cell.y; y < _rectangle_height + cache_cell.y; y++)
  {
    for (int x = cache_cell.x; x < _rectangle_width + cache_cell.x; x++)
    {
      if (cell[step] != 0x0000)
      {
        test_buffer[y * 480 + x] = cell[step];
      }
      step++;
    }
  }
}

void _draw_gem(Vec2I gem_index)
{
  cache_entity_type = get_entity_type(gem_index);

  if (cache_entity_type >= 0 && cache_entity_type < GEM_TYPE_LENGTH)
  {
    cache_entity = get_entity_animation_position(gem_index);
    int step = 0;
    const uint16_t *current_gem_sprite = gem_types_table[cache_entity_type];

    for (int y_offset = 0; y_offset < gem_height; y_offset++)
    {
      int draw_y = cache_entity.y + y_offset;

      for (int x_offset = 0; x_offset < gem_width; x_offset++)
      {
        int draw_x = cache_entity.x + x_offset;

        // CLIPPING GUARD: Only draw if inside the 480x480 screen
        if (draw_x >= _board_beginning_position_x && draw_x < 480 && draw_y >= _board_beginning_position_y && draw_y < 480)
        {
          if (current_gem_sprite[step] != 0x0000)
          {
            test_buffer[draw_y * 480 + draw_x] = current_gem_sprite[step];
          }
        }
        step++;
      }
    }
  }
}

void _draw_all_cells()
{
  for (int i = 0; i < MAX_ROWS; i++)
  {
    for (int j = 0; j < MAX_COLS; j++)
    {
      _draw_cell(Vec2I{i, j});
    }
  }
}

void _draw_all_gems()
{
  for (int i = 0; i < MAX_ROWS; i++)
  {
    for (int j = 0; j < MAX_COLS; j++)
    {
      _draw_gem(Vec2I{i, j});
    }
  }
}

void _add_entity_dirty(Vec2I entity_index)
{
  for (uint16_t i = 0; i < count_dirty_entities; i++)
  {
    if (_entity_dirty[i].x == entity_index.x && _entity_dirty[i].y == entity_index.y)
    {
      return;
    }
  }
  _entity_dirty[count_dirty_entities] = entity_index;
  count_dirty_entities++;
}

void _remove_entity_dirty(Vec2I entity_index)
{
  for (uint16_t i = 0; i < count_dirty_entities; i++)
  {
    if (_entity_dirty[i].x == entity_index.x && _entity_dirty[i].y == entity_index.y)
    {
      _entity_dirty[i] = _entity_dirty[count_dirty_entities - 1];
      count_dirty_entities--;
      _entity_dirty[count_dirty_entities] = Vec2I{DO_NOT_RENDER, DO_NOT_RENDER};
      break;
    }
  }
}

void _refresh_the_screen_dirty()
{

  for (uint16_t i = 0; i < count_dirty_entities; i++)
  {
    Vec2I anim_pos = get_entity_animation_position(_entity_dirty[i]);

    int index_x = (int)(anim_pos.y / (float)(_rectangle_height + _rectangle_row_space));
    int index_y = (int)(anim_pos.x / (float)(_rectangle_width + _rectangle_cols_space));

    if (index_x < MAX_ROWS && index_y < MAX_COLS && index_x >= 0 && index_y >= 0)
    {
      _draw_cell(Vec2I(index_x, index_y));
      _draw_gem(_entity_dirty[i]);

      Vec2I cell_pos = get_cell_position(Vec2I{index_x, index_y});

      // This section (Partial Screen sending) get help by GEMINI
      for (int row = 0; row < _rectangle_height; row++)
      {
        // Calculate where this specific row starts in the 480x480 buffer
        uint16_t *row_ptr = test_buffer + ((cell_pos.y + row) * 480) + cell_pos.x;

        // Draw one horizontal line at a time
        // x, y, buffer, width, height(is 1 because we do it row by row)
        gfx->draw16bitRGBBitmap(cell_pos.x, cell_pos.y + row, row_ptr, _rectangle_width, 1);
      }
    }
  }
}

void _refresh_the_screen_all()
{
  _draw_all_cells();
  _draw_all_gems();
  gfx->flush();
  gfx->draw16bitRGBBitmap(0, 0, test_buffer, 480, 480);
}

void initialize_render()
{
  // 1. Calculate bytes: 480 * 480 * 2 (16-bit is 2 bytes)
  size_t buffer_size = 480 * 480 * 2;

  // 2. Allocate memory in PSRAM
  test_buffer = (uint16_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM);

  if (test_buffer == nullptr)
  {
    Serial.println("Failed to allocate test buffer!");
    return;
  }

  // 3. Fill the buffer with a color manually
  for (int i = 0; i < (480 * 480); i++)
  {
    test_buffer[i] = 0;
  }

  _refresh_the_screen_all();

  for (uint16_t i = 0; i < i; i++)
  {
    _entity_dirty[i] = Vec2I{DO_NOT_RENDER, DO_NOT_RENDER};
  }
}