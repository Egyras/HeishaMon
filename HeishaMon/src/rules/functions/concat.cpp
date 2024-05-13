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

int8_t rule_function_concat_callback(struct rules_t *obj) {
  uint16_t len = 0, offset = 0;
  uint8_t nr = rules_gettop(obj), y = 0;

  for(y=1;y<=nr;y++) {
    switch(rules_type(obj, y)) {
      case VNULL: {
        len += strlen("NULL");
      } break;
      case VINTEGER: {
        int i = rules_tointeger(obj, y);
        len += snprintf(NULL, 0, "%d", i);
      } break;
      case VFLOAT: {
        float f = rules_tofloat(obj, y);
        len += snprintf(NULL, 0, "%g", f);
      } break;
      case VCHAR: {
        len += strlen(rules_tostring(obj, y));
      } break;
    }
  }

  char tmp[len+1] = { '\0' };
  for(y=1;y<=nr;y++) {
    switch(rules_type(obj, y)) {
      case VNULL: {
        offset += snprintf(&tmp[offset], len+1-offset, "NULL");
      } break;
      case VINTEGER: {
        int i = rules_tointeger(obj, y);
        offset += snprintf(&tmp[offset], len+1-offset, "%d", i);
      } break;
      case VFLOAT: {
        float f = rules_tofloat(obj, y);
        offset += snprintf(&tmp[offset], len+1-offset, "%g", f);
      } break;
      case VCHAR: {
        offset += snprintf(&tmp[offset], len+1-offset, "%s", rules_tostring(obj, y));
      } break;
    }
  }

  while(nr > 0) {
    rules_remove(obj, nr--);
  }

  rules_pushstring(obj, tmp);

  return 0;
}
