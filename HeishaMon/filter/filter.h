#pragma once

#include "stdint.h"

typedef enum
{
    FILTER_TYPE_NONE,
    FILTER_TYPE_MIN,
    FILTER_TYPE_MAX,
    FILTER_TYPE_AVG,
    FILTER_TYPE_Last
} filter_type_t;

typedef struct
{
    uint32_t filter_count;
    double filter_sum;
} filter_context_t;

void filter_clear(filter_context_t *context);
void filter_update(filter_context_t *context, filter_type_t type, float value);
float filter_get_value(filter_context_t *context, filter_type_t type);