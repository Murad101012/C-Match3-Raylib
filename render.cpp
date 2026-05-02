#include "game_config.h"
#include "game_logic.h"
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
}

void _draw_board()
{
  for (int i = 0; i < MAX_ROWS; i++)
  {
    for (int j = 0; j < MAX_COLS; j++)
    {
      cache_cell = get_cell_position((Vec2I){i, j});
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
  }
}

void _draw_gems()
{
  for (int i = 0; i < MAX_ROWS; i++)
  {
    for (int j = 0; j < MAX_COLS; j++)
    {
      cache_entity_type = get_entity_type((Vec2I){i, j});

      if (cache_entity_type >= 0 && cache_entity_type < GEM_TYPE_LENGTH)
      {
        cache_entity = get_entity_animation_position((Vec2I){i, j});
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
  }
}

void _refresh_the_screen()
{
  _draw_board();
  _draw_gems();
  gfx->flush();
  gfx->draw16bitRGBBitmap(0, 0, test_buffer, 480, 480);
}