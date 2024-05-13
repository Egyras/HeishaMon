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

int8_t rule_function_max_callback(struct rules_t *obj) {
  float y = 0, x = 0, z = 0, a = 0;
  uint8_t nr = rules_gettop(obj);

  while(nr > 0) {
    switch(rules_type(obj, nr)) {
      case VINTEGER: {
        y = (float)rules_tointeger(obj, nr);
      } break;
      case VFLOAT: {
        y = rules_tofloat(obj, nr);
      } break;
      case VNULL: {
      } break;
    }
    rules_remove(obj, nr--);
    if(a == 0) {
      a = 1;
      x = y;
    } else {
      x = MAX(x, y);
    }
  }

  if(modff(x, &z) == 0) {
#ifdef DEBUG
    printf("\tmax = %d\n", (int)x);
#endif
    rules_pushinteger(obj, x);
  } else {
#ifdef DEBUG
    printf("\tmax = %f\n", x);
#endif
    rules_pushfloat(obj, x);
  }

  return 0;
}
