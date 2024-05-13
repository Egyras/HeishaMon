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
  uint8_t opcode;
  const char *name;
  uint8_t precedence;
  uint8_t associativity;
} __attribute__((packed));

extern struct rule_operator_t rule_operators[];
extern uint8_t nr_rule_operators;

#endif
