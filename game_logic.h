#include <stdint.h>
#include "game_config.h"

int16_t get_entity_type(Vec2I entity_index);
void set_entity_type(Vec2I entity_index, int16_t type_new);

Vec2I get_cell_position(Vec2I cell_index);
void set_cell_position(Vec2I cell_index, Vec2I cell_position_new);

Vec2I get_entity_position(Vec2I entity_index);
void set_entity_position(Vec2I entity_index, Vec2I entity_position_new);

bool get_entity_match_state(Vec2I entity_index);
void set_entity_match_state(Vec2I entity_index, bool type_new);

void _initialize_game();
void logic();