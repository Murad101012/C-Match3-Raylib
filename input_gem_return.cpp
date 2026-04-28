#include "input_gem_return.h"
#include "game_config.h"
#include <stdio.h>
#include "src/Touch_GT911.h"

Vec2I _entity_return_on_mouse_click()
{
  Vec2I mouse_position_normalized =
      (Vec2I){touch_input_screen.points[0].x - _board_beginning_position_x,
              touch_input_screen.points[0].y - _board_beginning_position_y};

  int index_x = (int)((mouse_position_normalized.y /
                       (float)(_rectangle_height + _rectangle_row_space)));
  int index_y = (int)((mouse_position_normalized.x /
                       (float)(_rectangle_width + _rectangle_cols_space)));

  if (index_x < MAX_ROWS && index_y < MAX_COLS && index_x >= 0 &&
      index_y >= 0 && mouse_position_normalized.x >= 0 &&
      mouse_position_normalized.y >= 0)
  {
    //Serial.printf("Returned entity index:[%d][%d]\n", index_x, index_y);
    return (Vec2I){index_x, index_y};
  }
  return (Vec2I){-999, -999};
}