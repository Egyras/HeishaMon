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

int8_t rule_function_coalesce_callback(struct rules_t *obj) {
  float x = 0, z = 0;
  uint8_t nr = rules_gettop(obj), loop = 1, y = 0;

  for(y=1;y<=nr && loop == 1;y++) {
    switch(rules_type(obj, y)) {
      case VNULL: {
      } break;
      case VINTEGER: {
        x = (float)rules_tointeger(obj, y);
        loop = 0;
      } break;
      case VFLOAT: {
        x = rules_tofloat(obj, y);
        loop = 0;
      } break;
    }
  }
  while(nr > 0) {
    rules_remove(obj, nr--);
  }

  if(modff(x, &z) == 0) {
#ifdef DEBUG
    printf("\tcoalesce = %d\n", (int)x);
#endif
    rules_pushinteger(obj, x);
  } else {
#ifdef DEBUG
    printf("\tcoalesce = %f\n", x);
#endif
    rules_pushfloat(obj, x);
  }

  return 0;
}
