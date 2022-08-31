/*
  Copyright (C) CurlyMo

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _RULES_COALESCE_H_
#define _RULES_COALESCE_H_

#include <stdint.h>
#include "../rules.h"

int rule_function_coalesce_callback(struct rules_t *obj, uint16_t argc, uint16_t *argv, int *ret);

#endif