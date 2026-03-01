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
#include "../../common/log.h"
#include "../function.h"
#include "../rules.h"

int8_t rule_function_max_callback(void) {
  float y = 0, x = 0, z = 0, a = 0;
  uint8_t nr = rules_gettop();

  while(nr > 0) {
    switch(rules_type(nr)) {
       case VCHAR: {
        logprintf_P(F("ERROR: max only takes numbers"));
        while(nr > 0) {
          rules_remove(nr--);
        }
        rules_pushnil();
        return -1;
      } break;
      case VINTEGER: {
        y = (float)rules_tointeger(nr);
      } break;
      case VFLOAT: {
        y = rules_tofloat(nr);
      } break;
      case VNULL: {
      } break;
    }
    rules_remove(nr--);
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
    rules_pushinteger(x);
  } else {
#ifdef DEBUG
    printf("\tmax = %f\n", x);
#endif
    rules_pushfloat(x);
  }

  return 0;
}
