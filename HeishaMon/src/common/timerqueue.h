/*
  Copyright (C) CurlyMo

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _RULES_TIMERQUEUE_H_
#define _RULES_TIMERQUEUE_H_

#include <stdint.h>

typedef struct timerqueue_t {
  int sec;
  int usec;
  int nr;
} timerqueue_t;

extern struct timerqueue_t **timerqueue;
extern int timerqueue_size;
extern void timer_cb(int nr);

struct timerqueue_t *timerqueue_pop();
struct timerqueue_t *timerqueue_peek();
void timerqueue_insert(int sec, int usec, int nr);
void timerqueue_update(void);


#endif