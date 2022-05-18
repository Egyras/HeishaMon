/*
  Copyright (C) CurlyMo

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifdef ESP8266
  #pragma GCC diagnostic warning "-fpermissive"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

#include "../function.h"
#include "../../common/log.h"
#include "../../common/mem.h"
#include "../rules.h"
#include "../../common/timerqueue.h"

int rule_function_set_timer_callback(struct rules_t *obj, uint16_t argc, uint16_t *argv, int *ret) {
  struct timerqueue_t *node = NULL;
  struct itimerval it_val;
  int i = 0, x = 0, sec = 0, usec = 0, nr = 0;

  if(argc > 2) {
    return -1;
  }

  if(argc == 2) {
    if(obj->varstack.buffer[argv[0]] != VINTEGER) {
      return -1;
    }
    if(obj->varstack.buffer[argv[1]] != VINTEGER) {
      return -1;
    }

    struct vm_vinteger_t *val = (struct vm_vinteger_t *)&obj->varstack.buffer[argv[0]];
    nr = val->value;

    val = (struct vm_vinteger_t *)&obj->varstack.buffer[argv[1]];

    timerqueue_insert(val->value, 0, nr);

    logprintf_P(F("%s set timer #%d to %d seconds"), __FUNCTION__, nr, val->value);
  }

  if(argc == 1) {
    if(obj->varstack.buffer[argv[0]] != VINTEGER) {
      return -1;
    }

    struct vm_vinteger_t *val = (struct vm_vinteger_t *)&obj->varstack.buffer[argv[0]];
    nr = val->value;

   *ret = obj->varstack.nrbytes;

    unsigned int size = 0;

    int a = 0;
    for(a=0;a<timerqueue_size;a++) {
      if(timerqueue[a]->nr == nr) {
        size = alignedbytes(obj->varstack.nrbytes+sizeof(struct vm_vinteger_t));

        struct vm_vinteger_t *out = (struct vm_vinteger_t *)&obj->varstack.buffer[obj->varstack.nrbytes];
        out->ret = 0;
        out->type = VINTEGER;
        out->value = timerqueue[a]->sec;
        break;
      }
    }
    if(size == 0) {
      size = alignedbytes(obj->varstack.nrbytes+sizeof(struct vm_vnull_t));

      struct vm_vnull_t *out = (struct vm_vnull_t *)&obj->varstack.buffer[obj->varstack.nrbytes];
      out->ret = 0;
      out->type = VNULL;
    }

    obj->varstack.nrbytes = size;
    obj->varstack.bufsize = MAX(obj->varstack.bufsize, alignedvarstack(obj->varstack.nrbytes));
  }

  return 0;
}
