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


int rule_operator_or_callback(struct rules_t *obj, int a, int b, int *ret) {
  *ret = obj->varstack.nrbytes;

  unsigned int size = alignedbytes(obj->varstack.nrbytes+sizeof(struct vm_vinteger_t));

  struct vm_vinteger_t *out = (struct vm_vinteger_t *)&obj->varstack.buffer[obj->varstack.nrbytes];
  out->ret = 0;
  out->type = VINTEGER;

  if(obj->varstack.buffer[a] == VNULL && obj->varstack.buffer[b] == VNULL) {
    out->value = 0;
/* LCOV_EXCL_START*/
#ifdef DEBUG
    printf("%s NULL\n", __FUNCTION__);
#endif
/* LCOV_EXCL_STOP*/
  } else {
    /*
     * Values can only be equal when the type matches
     */
    switch(obj->varstack.buffer[a]) {
      case VINTEGER: {
        struct vm_vinteger_t *n = (struct vm_vinteger_t *)&obj->varstack.buffer[a];
        if(n->value > 0) {
          out->value = 1;
        } else {
          out->value = 0;
        }
/* LCOV_EXCL_START*/
#ifdef DEBUG
        printf("%s %d\n", __FUNCTION__, n->value);
#endif
/* LCOV_EXCL_STOP*/

      } break;
      case VFLOAT: {
        struct vm_vfloat_t *n = (struct vm_vfloat_t *)&obj->varstack.buffer[a];
        if(n->value > 0) {
          out->value = 1;
        } else {
          out->value = 0;
        }
      } break;
      /*
       * FIXME
       */
      /* LCOV_EXCL_START*/
      case VCHAR: {
        out->value = 1;
      } break;
      /* LCOV_EXCL_STOP*/
    }
    switch(obj->varstack.buffer[b]) {
      case VINTEGER: {
        struct vm_vinteger_t *n = (struct vm_vinteger_t *)&obj->varstack.buffer[b];
        if(n->value > 0 || out->value == 1) {
          out->value = 1;
        } else {
          out->value = 0;
        }
/* LCOV_EXCL_START*/
#ifdef DEBUG
        printf("%s %d\n", __FUNCTION__, n->value);
#endif
/* LCOV_EXCL_STOP*/
      } break;
      case VFLOAT: {
        struct vm_vfloat_t *n = (struct vm_vfloat_t *)&obj->varstack.buffer[b];
        if(n->value > 0 || out->value == 1) {
          out->value = 1;
        } else {
          out->value = 0;
        }
      } break;
      /*
       * FIXME
       */
      /* LCOV_EXCL_START*/
      case VCHAR: {
        out->value = 1;
      } break;
      /* LCOV_EXCL_STOP*/
    }
  }

  obj->varstack.nrbytes = size;
  obj->varstack.bufsize = MAX(obj->varstack.bufsize, alignedvarstack(obj->varstack.nrbytes));

  return 0;
}