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
#include "../../common/mem.h"
#include "../function.h"
#include "../rules.h"

int8_t rule_function_print_callback(void) {
  uint16_t len = 0, offset = 0;
  uint8_t nr = rules_gettop(), y = 0;

  for(y=1;y<=nr;y++) {
    switch(rules_type(y)) {
      case VNULL: {
        len += 4;
      } break;
      case VINTEGER: {
        len += snprintf(NULL, 0, "%d", rules_tointeger(y));
      } break;
      case VFLOAT: {
        len += snprintf(NULL, 0, "%g", (double)rules_tofloat(y));
      } break;
      case VCHAR: {
        len += strlen(rules_tostring(y));
      } break;
    }
  }

  if(len > 0) {
    len++;
    char *out = (char *)MALLOC(len);
    if(out == NULL) {
      OUT_OF_MEMORY
    }
    memset(out, 0, len);

    for(y=1;y<=nr;y++) {
      switch(rules_type(y)) {
        case VNULL: {
          offset += snprintf(&out[offset], len-offset, "NULL");
        } break;
        case VINTEGER: {
          offset += snprintf(&out[offset], len-offset, "%d", rules_tointeger(y));
        } break;
        case VFLOAT: {
          offset += snprintf(&out[offset], len-offset, "%g", (double)rules_tofloat(y));
        } break;
        case VCHAR: {
          offset += snprintf(&out[offset], len-offset, "%s", rules_tostring(y));
        } break;
      }
    }
    logprintf(out);
    FREE(out);
  }

  while(nr > 0) {
    rules_remove(nr--);
  }


  return 0;
}
