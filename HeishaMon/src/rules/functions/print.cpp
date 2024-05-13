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

int8_t rule_function_print_callback(struct rules_t *obj) {
  uint16_t len = 0, offset = 0;
  uint8_t nr = rules_gettop(obj), y = 0;

  for(y=1;y<=nr;y++) {
    switch(rules_type(obj, y)) {
      case VNULL: {
        len += 4;
      } break;
      case VINTEGER: {
        len += snprintf(NULL, 0, "%d", rules_tointeger(obj, y));
      } break;
      case VFLOAT: {
        len += snprintf(NULL, 0, "%g", rules_tofloat(obj, y));
      } break;
      case VCHAR: {
        len += strlen(rules_tostring(obj, y));
      } break;
    }
  }

  if(len > 0) {
    len++;
    char out[len] = { '\0' };

    for(y=1;y<=nr;y++) {
      switch(rules_type(obj, y)) {
        case VNULL: {
          offset += snprintf(&out[offset], len-offset, "NULL");
        } break;
        case VINTEGER: {
          offset += snprintf(&out[offset], len-offset, "%d", rules_tointeger(obj, y));
        } break;
        case VFLOAT: {
          offset += snprintf(&out[offset], len-offset, "%g", rules_tofloat(obj, y));
        } break;
        case VCHAR: {
          offset += snprintf(&out[offset], len-offset, "%s", rules_tostring(obj, y));
        } break;
      }
    }
    logprintf(out);
  }

  while(nr > 0) {
    rules_remove(obj, nr--);
  }


  return 0;
}
