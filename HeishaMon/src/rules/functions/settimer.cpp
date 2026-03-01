/*
  Copyright (C) CurlyMo

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

#include "../function.h"
#include "../../common/mem.h"
#include "../../common/log.h"
#include "../rules.h"
#include "../../common/timerqueue.h"

int8_t rule_function_set_timer_callback(void) {
  struct timerqueue_t *node = NULL;
  struct itimerval it_val;
  uint16_t sec = 0, nr = 0;
  uint8_t x = rules_gettop();

  if(x < 2 || x > 2) {
    return -1;
  }

  switch(rules_type(-1)) {
    case VNULL: {
      rules_remove(-1);
      rules_remove(-1);
      return -1;
    } break;
    case VINTEGER: {
      sec = rules_tointeger(-1);
    } break;
    case VFLOAT: {
      sec = (int)rules_tofloat(-1);
    } break;
  }
  rules_remove(-1);

  switch(rules_type(-1)) {
    case VNULL: {
      rules_remove(-1);
      return -1;
    } break;
    case VINTEGER: {
      nr = rules_tointeger(-1);
    } break;
    case VFLOAT: {
      nr = (int)rules_tofloat(-1);
    } break;
  }

  timerqueue_insert(sec, 0, nr);

  logprintf_P(F("timer #%d set to %d seconds"), nr, sec);

  return 0;
}
