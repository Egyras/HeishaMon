#include "filter.h"

void filter_clear(filter_context_t *context)
{
    context->filter_count = 0;
    context->filter_sum = 0;
}

void filter_update(filter_context_t *context, filter_type_t type, float value)
{
    switch (type)
    {
    case FILTER_TYPE_MIN:
        if (context->filter_count == 0 || value < context->filter_sum)
        {
            context->filter_count = 1;
            context->filter_sum = value;
        }
        break;
    case FILTER_TYPE_MAX:
        if (context->filter_count == 0 || value > context->filter_sum)
        {
            context->filter_count = 1;
            context->filter_sum = value;
        }
        break;
    case FILTER_TYPE_AVG:
        context->filter_count++;
        context->filter_sum += value;
        break;
    default:
        break;
    }
}

float filter_get_value(filter_context_t *context, filter_type_t type)
{
    switch (type)
    {
    case FILTER_TYPE_MIN:
    case FILTER_TYPE_MAX:
        return context->filter_sum;
    case FILTER_TYPE_AVG:
        if (context->filter_count != 0)
        {
            return context->filter_sum / context->filter_count;
        }
        break;
    }

    return 0;
}