#pragma once

#include "stdint.h"

bool crash_helper_has_crashed();
void crash_helper_clear_crash();
void crash_helper_print_crash(char *buffer, uint16_t buffer_size);