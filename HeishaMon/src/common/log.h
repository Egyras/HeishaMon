/*
  Copyright (C) CurlyMo

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _LOG_H_
#define _LOG_H_

#include <Arduino.h>

#define logprintln(a) _logprintln(__FILE__, __LINE__, a)
#define logprintf(a, ...) _logprintf(__FILE__, __LINE__, a, ##__VA_ARGS__)
#define logprintln_P(a) _logprintln_P(__FILE__, __LINE__, a)
#define logprintf_P(a, ...) _logprintf_P(__FILE__, __LINE__, a, ##__VA_ARGS__)

void _logprintln(const char *file, unsigned int line, char *msg);
void _logprintf(const char *file, unsigned int line, char *fmt, ...);
void _logprintln_P(const char *file, unsigned int line, const __FlashStringHelper *msg);
void _logprintf_P(const char *file, unsigned int line, const __FlashStringHelper *fmt, ...);

#endif
