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

int rule_operator_eq_callback(struct rules_t *obj, int a, int b, int *ret) {
  *ret = obj->varstack.nrbytes;

  unsigned int size = alignedbytes(obj->varstack.nrbytes+sizeof(struct vm_vinteger_t));

  struct vm_vinteger_t *out = (struct vm_vinteger_t *)&obj->varstack.buffer[obj->varstack.nrbytes];
  out->ret = 0;
  out->type = VINTEGER;

  /*
   * Values can only be equal when the type matches
   */
  if((obj->varstack.buffer[a]) != (obj->varstack.buffer[b])) {
    out->value = 0;
  } else {
    switch(obj->varstack.buffer[a]) {
      case VNULL: {
        out->value = 1;

/* LCOV_EXCL_START*/
#ifdef DEBUG
        printf("%s NULL\n", __FUNCTION__);
#endif
/* LCOV_EXCL_STOP*/
      } break;
      case VINTEGER: {
        struct vm_vinteger_t *na = (struct vm_vinteger_t *)&obj->varstack.buffer[a];
        struct vm_vinteger_t *nb = (struct vm_vinteger_t *)&obj->varstack.buffer[b];
        if(na->value == nb->value) {
          out->value = 1;
        } else {
          out->value = 0;
        }

/* LCOV_EXCL_START*/
#ifdef DEBUG
        printf("%s %d %d\n", __FUNCTION__, na->value, nb->value);
#endif
/* LCOV_EXCL_STOP*/

      } break;
      case VFLOAT: {
        struct vm_vfloat_t *na = (struct vm_vfloat_t *)&obj->varstack.buffer[a];
        struct vm_vfloat_t *nb = (struct vm_vfloat_t *)&obj->varstack.buffer[b];
        if(fabs((float)na->value-(float)nb->value) < EPSILON) {
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
        struct vm_vchar_t *na = (struct vm_vchar_t *)&obj->varstack.buffer[a];
        struct vm_vchar_t *nb = (struct vm_vchar_t *)&obj->varstack.buffer[b];
        if(strcmp((char *)na->value, (char *)nb->value) == 0) {
          out->value = 1;
        } else {
          out->value = 0;
        }
      } break;
      /* LCOV_EXCL_STOP*/
    }
  }

  obj->varstack.nrbytes = size;
  obj->varstack.bufsize = MAX(obj->varstack.bufsize, alignedvarstack(obj->varstack.nrbytes));

  return 0;
}