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

void rules_loop(void);
void rules_boot(void);
void rules_new_event(const char *event);
int rules_parse(char *file);
void rules_setup(void);
void rules_timer_cb(int nr);
void rules_event_cb(char *name);

#endif