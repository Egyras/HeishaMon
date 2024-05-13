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

int8_t rule_function_round_callback(struct rules_t *obj) {
  float x = 0, z = 0;
  uint8_t dec = 0;
  uint8_t nr = rules_gettop(obj), y = nr;

  if(nr > 2) {
    return -1;
  } else if(nr == 2) {
    switch(rules_type(obj, nr)) {
      case VINTEGER: {
        dec = rules_tointeger(obj, nr);
      } break;
    }
    rules_remove(obj, nr--);
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
    printf("\tround = %d\n", (int)x);
#endif
    rules_pushinteger(obj, x);
  } else {
    if(y == 2) {
      uint8_t size = snprintf(NULL, 0, "%.*f", dec, x)+1;
      char buf[size] = { '\0' };
      snprintf((char *)&buf, size, "%.*f", dec, x);
#ifdef DEBUG
      printf("\tround = %f\n", atof(buf));
#endif
      rules_pushfloat(obj, atof(buf));
    } else {
#ifdef DEBUG
      printf("\tround = %d\n", (int)x);
#endif
      rules_pushinteger(obj, (int)x);
    }
  }

  return 0;
}
