#pragma once
#include <cstdint>
#include "game_config.h"
#define CUSTOM_INT_ARRAY_LENGTH 5

void _intializate_sequence();

/**
 * @brief Interpalation between values based on t as distance
 *
 * @param begin
 * @param end
 * @param t normalized distance between 0 and 1
 * @return int
 */
int Lerp(float begin, float end, float t);

typedef enum
{
    NOT_AUTO_KILL_SEQUENCE,
    TIMES_AUTO_KILL_SEQUENCE,
    MILLISECONDS_AUTO_KILL_SEQUENCE,
    CONDITION_MET_AUTO_KILL_SEQUENCE
} auto_kill_method_enum;

typedef enum
{
    FUNC_SIMPLE,
    FUNC_WITH_INT_ARRAY,
    FUNC_WITH_DOUBLE_VEC2I
} func_type_enum;

typedef struct
{
    // Variables assigned by developer
    uint16_t delay;
    func_type_enum function_type;
    union
    {
        void (*without_ptr)();
        void (*with_int_array)(int16_t *);
        void (*with_vec2i) (Vec2I, Vec2I);
    };

    auto_kill_method_enum auto_kill_method_enum_type;
    uint32_t auto_kill_method_value_target;

    // Variables assigned by runtime in game
    bool active;
    uint32_t next_execute_time;
    uint32_t auto_kill_method_value_current;
    int16_t custom_int_array[CUSTOM_INT_ARRAY_LENGTH];
} sequence_object;

void _check_sequence();
sequence_object *_add_to_sequence(sequence_object sequence_object);
void _remove_from_sequence(sequence_object *sequence_object);