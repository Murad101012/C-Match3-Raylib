#include "game_config.h"
#include "game_logic.h"
#include "src/display.h"
#include <esp_heap_caps.h>

// Prevents calling get_ function twice for both x and y
static Vec2I cache_cell;
static Vec2I cache_entity;
static int16_t cache_entity_type;
static bool _is_screen_dirty = true;

void initialize_render()
{

}

void _draw_board() {

  for (int i = 0; i < MAX_ROWS; i++) {
    for (int j = 0; j < MAX_COLS; j++) {
      cache_cell = get_cell_position((Vec2I){i, j});
      gfx->fillRect(cache_cell.x, cache_cell.y, _rectangle_width,
                    _rectangle_height, _rectangle_color);
    }
  }
}

void _draw_gems() {
  for (int i = 0; i < MAX_ROWS; i++) {
    for (int j = 0; j < MAX_COLS; j++) {
      cache_entity_type = get_entity_type((Vec2I){i, j});
      if (cache_entity_type >= 0) {
        cache_cell = get_entity_position((Vec2I){i, j});
        gfx->fillCircle(cache_cell.x, cache_cell.y, _circle_radius,
                        _gem_type_color[cache_entity_type]);
      }
    }
  }
}

//For now this only called when match happen at swap
void _refresh_the_screen()
{
  gfx->fillScreen(BLACK);
  _draw_board();
  _draw_gems();
  gfx->flush();
}