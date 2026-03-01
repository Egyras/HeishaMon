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

int8_t rule_function_coalesce_callback(void) {
  float x = 0, z = 0;
  const char *a = NULL;
  uint8_t nr = rules_gettop(), loop = 1, y = 0;

  for(y=1;y<=nr && loop == 1;y++) {
    switch(rules_type(y)) {
      case VNULL: {
      } break;
      case VINTEGER: {
        x = (float)rules_tointeger(y);
        loop = 0;
      } break;
      case VFLOAT: {
        x = rules_tofloat(y);
        loop = 0;
      } break;
      case VCHAR: {
        a = rules_tostring(y);
        loop = 0;
      } break;
    }
  }
  while(nr > 0) {
    rules_remove(nr--);
  }

  if(a != NULL) {
#ifdef DEBUG
      printf("\tcoalesce = %s\n", a);
#endif
      rules_pushstring((char *)a);
  } else {
    if(modff(x, &z) == 0) {
#ifdef DEBUG
      printf("\tcoalesce = %d\n", (int)x);
#endif
      rules_pushinteger(x);
    } else {
#ifdef DEBUG
      printf("\tcoalesce = %f\n", (double)x);
#endif
      rules_pushfloat(x);
    }
  }

  return 0;
}
