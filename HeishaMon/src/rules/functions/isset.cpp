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
#include "../rules.h"

int8_t rule_function_isset_callback(struct rules_t *obj) {
  uint8_t x = rules_gettop(obj);
  uint8_t ret = 0;

  if(x < 1 || x > 1) {
    return -1;
  }

  switch(rules_type(obj, -1)) {
    case VNULL: {
      ret = 0;
#ifdef DEBUG
      printf(".. %s NULL -> 0\n", __FUNCTION__);
#endif
    } break;
    case VINTEGER:
    case VFLOAT: {
      ret = 1;
#ifdef DEBUG
      printf(".. %s !NULL -> 1\n", __FUNCTION__);
#endif
    } break;
  }
  rules_remove(obj, -1);
  rules_pushinteger(obj, ret);
  return 0;
}
