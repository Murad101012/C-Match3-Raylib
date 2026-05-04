#pragma once
#include "src/display.h"
#define DO_NOT_RENDER -10

void _draw_all_gems();
void _draw_all_cells();
void _refresh_the_screen_dirty();
void initialize_render();
void _add_entity_dirty(Vec2I entity_index);
void _remove_entity_dirty(Vec2I entity_index);