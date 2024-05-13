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

#include "../../common/uint32float.h"
#include "../function.h"
#include "../rules.h"

int8_t rule_function_floor_callback(struct rules_t *obj) {
  float x = 0, z = 0;
  uint8_t nr = rules_gettop(obj);

  if(nr > 1) {
    return -1;
  }

  switch(rules_type(obj, nr)) {
    case VNULL: {
      rules_remove(obj, nr--);
      rules_pushnil(obj);
      return 0;
    } break;
    case VINTEGER: {
      x = (float)rules_tointeger(obj, nr);
    } break;
    case VFLOAT: {
      x = rules_tofloat(obj, nr);
    } break;
  }
  rules_remove(obj, nr--);

  if(modff(x, &z) == 0) {
#ifdef DEBUG
    printf("\tfloor = %d\n", (int)x);
#endif
    rules_pushinteger(obj, x);
  } else {
#ifdef DEBUG
    printf("\tfloor = %d\n", (int)floor(x));
#endif
    rules_pushinteger(obj, (int)floor(x));
  }

  return 0;
}
