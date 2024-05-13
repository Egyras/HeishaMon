/*
  Copyright (C) CurlyMo

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef __RULES_H_
#define __RULES_H_

#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <Arduino.h>

#include "src/common/mem.h"

extern uint8_t nrrules;

void rules_boot(void);
int rules_parse(char *file);
void rules_setup(void);
void rules_timer_cb(int nr);
void rules_event_cb(const char *prefix, const char *name);
void rules_execute(void);

#endif
