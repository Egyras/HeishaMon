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

int8_t rule_function_concat_callback(void) {
  uint16_t len = 0, offset = 0;
  uint8_t nr = rules_gettop(), y = 0;

  for(y=1;y<=nr;y++) {
    switch(rules_type(y)) {
      case VNULL: {
        len += strlen("NULL");
      } break;
      case VINTEGER: {
        int i = rules_tointeger(y);
        len += snprintf(NULL, 0, "%d", i);
      } break;
      case VFLOAT: {
        float f = rules_tofloat(y);
        len += snprintf(NULL, 0, "%g", (double)f);
      } break;
      case VCHAR: {
        len += strlen(rules_tostring(y));
      } break;
    }
  }

  char *tmp = (char *)MALLOC(len+1);
  if(tmp == NULL) {
    OUT_OF_MEMORY
  }
  memset(tmp, 0, len+1);
  for(y=1;y<=nr;y++) {
    switch(rules_type(y)) {
      case VNULL: {
        offset += snprintf(&tmp[offset], len+1-offset, "NULL");
      } break;
      case VINTEGER: {
        int i = rules_tointeger(y);
        offset += snprintf(&tmp[offset], len+1-offset, "%d", i);
      } break;
      case VFLOAT: {
        float f = rules_tofloat(y);
        offset += snprintf(&tmp[offset], len+1-offset, "%g", (double)f);
      } break;
      case VCHAR: {
        offset += snprintf(&tmp[offset], len+1-offset, "%s", rules_tostring(y));
      } break;
    }
  }

  while(nr > 0) {
    rules_remove(nr--);
  }

  rules_pushstring(tmp);
  FREE(tmp);

  return 0;
}
