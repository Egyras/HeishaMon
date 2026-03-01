/*
  Copyright (C) CurlyMo

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../function.h"
#include "../rules.h"

int8_t rule_function_gpio_callback(void) {
  int8_t gpio = 0, state = 0;
  uint8_t nr = rules_gettop(), x = 1;

  if(nr == 1 || nr == 2) {
    if(rules_type(x) == VINTEGER) {
      gpio = rules_tointeger(x);
      rules_remove(x);
#ifdef DEBUG
      printf("\tgpio = %d\n", gpio);
#endif
      rules_pushinteger(digitalRead(gpio));
      if(nr == 1) {
        return 0;
      }
    } else {
      return -1;
    }
  }
  if(nr == 2) {
    if(rules_type(x) == VINTEGER) {
      state = rules_tointeger(x);
      if(state < 0 || state > 1) {
        return -1;
      }
#ifdef DEBUG
      printf("\tstate = %d\n", state);
#endif
      rules_remove(x++);
    } else {
      return -1;
    }
    digitalWrite(gpio, state);
  } else {
    return -1;
  }

  return 0;
}
