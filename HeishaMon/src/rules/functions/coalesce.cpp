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

#include "../function.h"
#include "../../common/mem.h"
#include "../rules.h"

int rule_function_coalesce_callback(struct rules_t *obj, uint16_t argc, uint16_t *argv, int *ret) {
/* LCOV_EXCL_START*/
#ifdef DEBUG
  printf("%s\n", __FUNCTION__);
#endif
/* LCOV_EXCL_STOP*/

  *ret = obj->varstack.nrbytes;

  int i = 0;
  for(i=0;i<argc;i++) {
    if(obj->varstack.buffer[argv[i]] == VNULL) {
      continue;
    } else {
      switch(obj->varstack.buffer[argv[i]]) {
        case VINTEGER: {
          unsigned int size = alignedbytes(obj->varstack.nrbytes+sizeof(struct vm_vinteger_t));

          struct vm_vinteger_t *out = (struct vm_vinteger_t *)&obj->varstack.buffer[obj->varstack.nrbytes];
          struct vm_vinteger_t *val = (struct vm_vinteger_t *)&obj->varstack.buffer[argv[i]];
          out->type = VINTEGER;
          out->ret = 0;
          out->value = val->value;
          obj->varstack.nrbytes = size;
          obj->varstack.bufsize = MAX(obj->varstack.bufsize, alignedvarstack(obj->varstack.nrbytes));
          return 0;
        } break;
        case VFLOAT: {
          unsigned int size = alignedbytes(obj->varstack.nrbytes+sizeof(struct vm_vfloat_t));

          struct vm_vfloat_t *out = (struct vm_vfloat_t *)&obj->varstack.buffer[obj->varstack.nrbytes];
          struct vm_vfloat_t *val = (struct vm_vfloat_t *)&obj->varstack.buffer[argv[i]];
          out->type = VFLOAT;
          out->ret = 0;
          out->value = val->value;
          obj->varstack.nrbytes = size;
          obj->varstack.bufsize = MAX(obj->varstack.bufsize, alignedvarstack(obj->varstack.nrbytes));
          return 0;
        } break;
        /* LCOV_EXCL_START*/
        case VCHAR: {
          exit(-1);
        } break;
        /* LCOV_EXCL_STOP*/
        default: {

        } break;
      }
    }
  }

  return 0;
}
