/*
  Copyright (C) CurlyMo

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _RULE_FUNCTION_H_
#define _RULE_FUNCTION_H_

#include "rules.h" /* rewrite */

struct rule_function_t {
  const char *name;
  int8_t (*callback)(struct rules_t *obj);
} __attribute__((packed));

extern struct rule_function_t rule_functions[];
extern uint16_t nr_rule_functions;

#endif
