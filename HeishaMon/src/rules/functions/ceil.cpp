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

int8_t rule_function_ceil_callback(void) {
  float x = 0, z = 0;
  uint8_t nr = rules_gettop();

  if(nr > 1) {
    return -1;
  }

  switch(rules_type(nr)) {
    case VCHAR: {
      logprintf_P(F("ERROR: ceil only takes numbers"));
      while(nr > 0) {
        rules_remove(nr--);
      }
      rules_pushnil();
      return -1;
    } break;
    case VNULL: {
      rules_remove(nr--);
      rules_pushnil();
      return 0;
    } break;
    case VINTEGER: {
      x = (float)rules_tointeger(nr);
    } break;
    case VFLOAT: {
      x = rules_tofloat(nr);
    } break;
  }
  rules_remove(nr--);

  if(modff(x, &z) == 0) {
#ifdef DEBUG
    printf("\tround = %d\n", (int)x);
#endif
    rules_pushinteger(x);
  } else {
#ifdef DEBUG
    printf("\tround = %d\n", (int)ceilf(x));
#endif
    rules_pushinteger((int)ceilf(x));
  }

  return 0;
}
