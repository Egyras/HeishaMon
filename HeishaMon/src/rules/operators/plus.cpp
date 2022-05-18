/*
  Copyright (C) CurlyMo

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifdef ESP8266
  #pragma GCC diagnostic warning "-fpermissive"
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "../function.h"
#include "../../common/mem.h"
#include "../rules.h"

int rule_operator_plus_callback(struct rules_t *obj, int a, int b, int *ret) {
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
  } else if((obj->varstack.buffer[a]) == VFLOAT && (obj->varstack.buffer[b]) == VFLOAT) {
    unsigned int size = alignedbytes(obj->varstack.nrbytes+sizeof(struct vm_vfloat_t));

    struct vm_vfloat_t *out = (struct vm_vfloat_t *)&obj->varstack.buffer[obj->varstack.nrbytes];
    struct vm_vfloat_t *na = (struct vm_vfloat_t *)&obj->varstack.buffer[a];
    struct vm_vfloat_t *nb = (struct vm_vfloat_t *)&obj->varstack.buffer[b];

    out->ret = 0;
    out->type = VFLOAT;
    out->value = na->value + nb->value;

/* LCOV_EXCL_START*/
#ifdef DEBUG
    printf("%s %g %g\n", __FUNCTION__, na->value, nb->value);
#endif
/* LCOV_EXCL_STOP*/

    obj->varstack.nrbytes = size;
    obj->varstack.bufsize = MAX(obj->varstack.bufsize, alignedvarstack(obj->varstack.nrbytes));
  } else if((obj->varstack.buffer[a]) == VFLOAT || (obj->varstack.buffer[b]) == VFLOAT) {
    float f = 0;
    int i = 0;
    unsigned int size = alignedbytes(obj->varstack.nrbytes+sizeof(struct vm_vfloat_t));

    if((obj->varstack.buffer[a]) == VFLOAT) {
      struct vm_vfloat_t *na = (struct vm_vfloat_t *)&obj->varstack.buffer[a];
      f = na->value;
    } else if((obj->varstack.buffer[a]) == VINTEGER) {
      struct vm_vinteger_t *na = (struct vm_vinteger_t *)&obj->varstack.buffer[a];
      i = na->value;
    }
    if((obj->varstack.buffer[b]) == VFLOAT) {
      struct vm_vfloat_t *nb = (struct vm_vfloat_t *)&obj->varstack.buffer[b];
      f = nb->value;
    } else if((obj->varstack.buffer[b]) == VINTEGER) {
      struct vm_vinteger_t *nb = (struct vm_vinteger_t *)&obj->varstack.buffer[b];
      i = nb->value;
    }

    struct vm_vfloat_t *out = (struct vm_vfloat_t *)&obj->varstack.buffer[obj->varstack.nrbytes];
    out->ret = 0;
    out->type = VFLOAT;
    out->value = i + f;

/* LCOV_EXCL_START*/
#ifdef DEBUG
    printf("%s %g %d\n", __FUNCTION__, f, i);
#endif
/* LCOV_EXCL_STOP*/

    obj->varstack.nrbytes = size;
    obj->varstack.bufsize = MAX(obj->varstack.bufsize, alignedvarstack(obj->varstack.nrbytes));
  } else {
    unsigned int size = alignedbytes(obj->varstack.nrbytes+sizeof(struct vm_vinteger_t));

    struct vm_vinteger_t *out = (struct vm_vinteger_t *)&obj->varstack.buffer[obj->varstack.nrbytes];
    struct vm_vinteger_t *na = (struct vm_vinteger_t *)&obj->varstack.buffer[a];
    struct vm_vinteger_t *nb = (struct vm_vinteger_t *)&obj->varstack.buffer[b];

    out->ret = 0;
    out->type = VINTEGER;
    out->value = na->value + nb->value;

/* LCOV_EXCL_START*/
#ifdef DEBUG
    printf("%s %d %d\n", __FUNCTION__, na->value, nb->value);
#endif
/* LCOV_EXCL_STOP*/

    obj->varstack.nrbytes = size;
    obj->varstack.bufsize = MAX(obj->varstack.bufsize, alignedvarstack(obj->varstack.nrbytes));
  }

  return 0;
}
