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

int rule_operator_power_callback(struct rules_t *obj, int a, int b, int *ret) {
  *ret = obj->varstack.nrbytes;

  if((obj->varstack.buffer[a]) == VNULL || (obj->varstack.buffer[b]) == VNULL) {
    unsigned int size = alignedbytes(obj->varstack.nrbytes+sizeof(struct vm_vnull_t));

    struct vm_vnull_t *out = (struct vm_vnull_t *)&obj->varstack.buffer[obj->varstack.nrbytes];

    out->ret = 0;
    out->type = VNULL;

/* LCOV_EXCL_START*/
#ifdef DEBUG
    printf("%s NULL\n", __FUNCTION__);
#endif
/* LCOV_EXCL_STOP*/

    obj->varstack.nrbytes = size;
    obj->varstack.bufsize = MAX(obj->varstack.bufsize, alignedvarstack(obj->varstack.nrbytes));
  } else if((obj->varstack.buffer[a]) == VCHAR || (obj->varstack.buffer[b]) == VCHAR) {
  } else if((obj->varstack.buffer[a]) == VFLOAT || (obj->varstack.buffer[b]) == VFLOAT) {
    float f = 0;
    int i = 0;
    unsigned int size = alignedbytes(obj->varstack.nrbytes+sizeof(struct vm_vfloat_t));

    struct vm_vfloat_t *out = (struct vm_vfloat_t *)&obj->varstack.buffer[obj->varstack.nrbytes];

    if((obj->varstack.buffer[a]) == VFLOAT) {
      struct vm_vfloat_t *na = (struct vm_vfloat_t *)&obj->varstack.buffer[a];
      f = na->value;
    } else if((obj->varstack.buffer[a]) == VINTEGER) {
      struct vm_vinteger_t *na = (struct vm_vinteger_t *)&obj->varstack.buffer[a];
      i = na->value;
    }
    if((obj->varstack.buffer[b]) == VFLOAT) {
      struct vm_vfloat_t *nb = (struct vm_vfloat_t *)&obj->varstack.buffer[b];
      if((obj->varstack.buffer[a]) == VFLOAT) {
        out->value = pow(f, nb->value);
/* LCOV_EXCL_START*/
#ifdef DEBUG
        printf("1 %s %g %g\n", __FUNCTION__,f, nb->value);
#endif
/* LCOV_EXCL_STOP*/
      } else if((obj->varstack.buffer[a]) == VINTEGER) {
        out->value = pow(i, nb->value);
/* LCOV_EXCL_START*/
#ifdef DEBUG
        printf("2 %s %d %g\n", __FUNCTION__,i, nb->value);
#endif
/* LCOV_EXCL_STOP*/
      }
    } else if((obj->varstack.buffer[b]) == VINTEGER) {
      struct vm_vinteger_t *nb = (struct vm_vinteger_t *)&obj->varstack.buffer[b];
      out->value = pow(f, nb->value);
/* LCOV_EXCL_START*/
#ifdef DEBUG
      printf("3 %s %g %d\n", __FUNCTION__,f, nb->value);
#endif
/* LCOV_EXCL_STOP*/
    }

    out->ret = 0;
    out->type = VFLOAT;

/* LCOV_EXCL_START*/
#ifdef DEBUG
    printf("%s %g %d\n", __FUNCTION__, f, i);
#endif
/* LCOV_EXCL_STOP*/

    obj->varstack.nrbytes = size;
    obj->varstack.bufsize = MAX(obj->varstack.bufsize, alignedvarstack(obj->varstack.nrbytes));
  } else if((obj->varstack.buffer[a]) == VINTEGER && (obj->varstack.buffer[b]) == VINTEGER) {
    unsigned int size = alignedbytes(obj->varstack.nrbytes+sizeof(struct vm_vinteger_t));

    struct vm_vinteger_t *out = (struct vm_vinteger_t *)&obj->varstack.buffer[obj->varstack.nrbytes];

    struct vm_vinteger_t *na = (struct vm_vinteger_t *)&obj->varstack.buffer[a];
    struct vm_vinteger_t *nb = (struct vm_vinteger_t *)&obj->varstack.buffer[b];
    out->ret = 0;
    out->type = VINTEGER;
    out->value = pow(na->value, nb->value);

/* LCOV_EXCL_START*/
#ifdef DEBUG
    printf("4 %s %d %d\n", __FUNCTION__, na->value, nb->value);
#endif
/* LCOV_EXCL_STOP*/
    obj->varstack.nrbytes = size;
    obj->varstack.bufsize = MAX(obj->varstack.bufsize, alignedvarstack(obj->varstack.nrbytes));
  }


  return 0;
}
