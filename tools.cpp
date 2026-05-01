#include "tools.h"
#include <cstdint>
#include <HardwareSerial.h>
#include <esp_random.h>

#define SEQUENCE_LIMIT 30

static uint32_t current_time;

static sequence_object _sequence_objects_array[SEQUENCE_LIMIT];

sequence_object *_add_to_sequence(sequence_object sequence_object)
{
    for (uint8_t i = 0; i < SEQUENCE_LIMIT; i++)
    {
        if (_sequence_objects_array[i].active == false)
        {
            _sequence_objects_array[i] = sequence_object;
            _sequence_objects_array[i].active = true;
            return &_sequence_objects_array[i];
        }
    }
    Serial.printf("\nSequence reached to limit cannot add new sequence_object");
    return nullptr;
}

void _remove_from_sequence(sequence_object *sequence_object)
{
    if (sequence_object != nullptr)
    {
        sequence_object->active = false;
        sequence_object->auto_kill_method_enum_type = NOT_AUTO_KILL_SEQUENCE;
        sequence_object->auto_kill_method_value_current = 0;
    }
}

void _check_sequence()
{
    current_time = millis();
    for (uint8_t i = 0; i < SEQUENCE_LIMIT; i++)
    {
        if (_sequence_objects_array[i].active)
        {
            if (_sequence_objects_array[i].next_execute_time <= current_time)
            {
                _sequence_objects_array[i].next_execute_time = millis() + _sequence_objects_array[i].delay;
                switch (_sequence_objects_array[i].function_type)
                {
                case FUNC_SIMPLE:
                    _sequence_objects_array[i].without_ptr();
                    break;
                case FUNC_WITH_INT_ARRAY:
                    _sequence_objects_array[i].with_int_array(_sequence_objects_array[i].custom_int_array);
                    break;
                }

                if (_sequence_objects_array[i].auto_kill_method_enum_type != NOT_AUTO_KILL_SEQUENCE)
                {
                    if (_sequence_objects_array[i].auto_kill_method_enum_type == TIMES_AUTO_KILL_SEQUENCE)
                    {
                        if (_sequence_objects_array[i].auto_kill_method_value_current >= _sequence_objects_array[i].auto_kill_method_value_target)
                        {
                            _remove_from_sequence(&_sequence_objects_array[i]);
                        }
                        else
                        {
                            _sequence_objects_array[i].auto_kill_method_value_current++;
                        }
                    }

                    if (_sequence_objects_array[i].auto_kill_method_enum_type == MILLISECONDS_AUTO_KILL_SEQUENCE)
                    {
                        if (_sequence_objects_array[i].auto_kill_method_value_current >= _sequence_objects_array[i].auto_kill_method_value_target)
                        {
                            _remove_from_sequence(&_sequence_objects_array[i]);
                        }
                        else
                        {
                            _sequence_objects_array[i].auto_kill_method_value_current += _sequence_objects_array[i].delay;
                        }
                    }

                    if (_sequence_objects_array[i].auto_kill_method_enum_type == CONDITION_MET_AUTO_KILL_SEQUENCE)
                    {
                        if (_sequence_objects_array[i].auto_kill_method_value_current == true)
                        {
                            _remove_from_sequence(&_sequence_objects_array[i]);
                        }
                    }
                }
            }
        }
    }
}

void _intializate_sequence()
{
    // Clearing potential garbage values
    for (uint8_t i = 0; i < SEQUENCE_LIMIT; i++)
    {
        _sequence_objects_array[i].next_execute_time = 0;
        _sequence_objects_array[i].delay = 0;
        _sequence_objects_array[i].active = false;
        _sequence_objects_array[i].auto_kill_method_enum_type = NOT_AUTO_KILL_SEQUENCE;
        _sequence_objects_array[i].auto_kill_method_value_current = 0;
        _sequence_objects_array[i].auto_kill_method_value_target = 0;
    }
}

int Lerp(float begin, float end, float t)
{
    if (t > 1)
    {
        t = 1;
    }
    else if (t < 0)
    {
        t = 0;
    }
    return begin + (end - begin) * t;
}
