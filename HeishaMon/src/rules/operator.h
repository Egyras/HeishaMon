/*
  Copyright (C) CurlyMo

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _RULE_OPERATOR_H_
#define _RULE_OPERATOR_H_

#include "rules.h" /* rewrite */

struct rule_operator_t {
  const char *name;
  int precedence;
  int associativity;
  int (*callback)(struct rules_t *obj, int a, int b, int *ret);
} __attribute__((packed));

extern struct rule_operator_t rule_operators[];
extern unsigned int nr_rule_operators;

#endif
