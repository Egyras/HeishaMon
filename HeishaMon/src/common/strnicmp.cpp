/*
  Copyright (C) CurlyMo

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#include "../rules/rules.h"

#ifdef ESP8266
#include <Arduino.h>
#endif

int strnicmp(char const *a, char const *b, size_t len) {
  unsigned int i = 0;

  if(a == NULL || b == NULL) {
    return -1;
  }
  if(len == 0) {
    return 0;
  }

  for(;i++<len; a++, b++) {
    uint8_t x = 0, y = 0;
#if (!defined(NON32XFER_HANDLER) && defined(MMU_SEC_HEAP)) || defined(COVERALLS)
    if((void *)a >= (void *)MMU_SEC_HEAP) {
      x = mmu_get_uint8((void *)&(*a));
    } else {
#endif
      x = *a;
#if (!defined(NON32XFER_HANDLER) && defined(MMU_SEC_HEAP)) || defined(COVERALLS)
    }
    if((void *)b >= (void *)MMU_SEC_HEAP) {
      y = mmu_get_uint8((void *)&(*b));
    } else {
#endif
      y = *b;
#if (!defined(NON32XFER_HANDLER) && defined(MMU_SEC_HEAP)) || defined(COVERALLS)
    }
#endif
    int d = tolower(x) - tolower(y);
    if(d != 0 || !x || i == len) {
      return d;
    }
  }
  return -1;
}