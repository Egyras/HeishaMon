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

int rule_operator_ge_callback(struct rules_t *obj, int a, int b, int *ret) {
  *ret = obj->varstack.nrbytes;

  unsigned int size = alignedbytes(obj->varstack.nrbytes+sizeof(struct vm_vinteger_t));

  struct vm_vinteger_t *out = (struct vm_vinteger_t *)&obj->varstack.buffer[obj->varstack.nrbytes];
  out->ret = 0;
  out->type = VINTEGER;

  /*
   * Values can only be equal when the type matches
   */
  if(
      (
        obj->varstack.buffer[a] == VNULL || obj->varstack.buffer[a] == VCHAR ||
        obj->varstack.buffer[b] == VNULL || obj->varstack.buffer[b] == VCHAR
      )
    &&
      (obj->varstack.buffer[a] != obj->varstack.buffer[b])
    ) {
    out->value = 0;
  } else {
    switch(obj->varstack.buffer[a]) {
      case VNULL: {
        out->value = 0;

/* LCOV_EXCL_START*/
#ifdef DEBUG
        printf("%s NULL\n", __FUNCTION__);
#endif
/* LCOV_EXCL_STOP*/
      } break;
      case VINTEGER: {
        float av = 0.0;
        float bv = 0.0;
        if(obj->varstack.buffer[a] == VINTEGER) {
          struct vm_vinteger_t *na = (struct vm_vinteger_t *)&obj->varstack.buffer[a];
          av = (float)na->value;
        }
        if(obj->varstack.buffer[b] == VFLOAT) {
          struct vm_vfloat_t *nb = (struct vm_vfloat_t *)&obj->varstack.buffer[b];
          bv = nb->value;
        }
        if(obj->varstack.buffer[b] == VINTEGER) {
          struct vm_vinteger_t *nb = (struct vm_vinteger_t *)&obj->varstack.buffer[b];
          bv = (float)nb->value;
        }
        if(av > bv || fabs(av-bv) < EPSILON) {
          out->value = 1;
        } else {
          out->value = 0;
        }

/* LCOV_EXCL_START*/
#ifdef DEBUG
        printf("%s %g %g\n", __FUNCTION__, av, bv);
#endif
/* LCOV_EXCL_STOP*/
      } break;
      case VFLOAT: {
        float av = 0.0;
        float bv = 0.0;
        if(obj->varstack.buffer[a] == VFLOAT) {
          struct vm_vfloat_t *na = (struct vm_vfloat_t *)&obj->varstack.buffer[a];
          av = na->value;
        }
        if(obj->varstack.buffer[b] == VFLOAT) {
          struct vm_vfloat_t *nb = (struct vm_vfloat_t *)&obj->varstack.buffer[b];
          bv = nb->value;
        }
        if(obj->varstack.buffer[b] == VINTEGER) {
          struct vm_vinteger_t *nb = (struct vm_vinteger_t *)&obj->varstack.buffer[b];
          bv = (float)nb->value;
        }
        if(av > bv || fabs(av-bv) < EPSILON) {
          out->value = 1;
        } else {
          out->value = 0;
        }

/* LCOV_EXCL_START*/
#ifdef DEBUG
        printf("%s %g %g\n", __FUNCTION__, av, bv);
#endif
/* LCOV_EXCL_STOP*/
      } break;
      /*
       * FIXME
       */
      /* LCOV_EXCL_START*/
      case VCHAR: {
        struct vm_vchar_t *na = (struct vm_vchar_t *)&obj->varstack.buffer[a];
        struct vm_vchar_t *nb = (struct vm_vchar_t *)&obj->varstack.buffer[b];
        if(na->value >= nb->value) {
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