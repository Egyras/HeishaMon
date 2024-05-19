/*
  Copyright (C) CurlyMo

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <libgen.h>
#include <assert.h>
#include <math.h>

#include "../common/mem.h"
#include "rules.h"
#include "operator.h"

struct rule_operator_t rule_operators[] = {
  { OP_EQ, "==", 30, 1 },
  { OP_NE, "!=", 30, 1 },
  { OP_ADD, "+", 60, 1 },
  { OP_SUB, "-", 60, 1 },
  { OP_MUL, "*", 70, 1 },
  { OP_MOD, "%", 70, 1 },
  { OP_AND, "&&", 20, 1 },
  { OP_OR, "||", 10, 1 },
  { OP_DIV, "/", 70, 1 },
  { OP_GE, ">=", 30, 1 },
  { OP_LE, "<=", 30, 1 },
  { OP_LT, "<", 30, 1, },
  { OP_GT, ">", 30, 1 },
  { OP_POW, "^", 80, 2 },
};

uint8_t nr_rule_operators = sizeof(rule_operators)/sizeof(rule_operators[0]);
