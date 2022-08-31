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
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <libgen.h>
#include <assert.h>
#include <math.h>

#include "rules.h"
#include "function.h"

#include "functions/max.h"
#include "functions/min.h"
#include "functions/coalesce.h"
#include "functions/settimer.h"
#include "functions/isset.h"
#include "functions/round.h"

struct rule_function_t rule_functions[] = {
  { "max", rule_function_max_callback },
  { "min", rule_function_min_callback },
  { "coalesce", rule_function_coalesce_callback },
  { "settimer", rule_function_set_timer_callback },
  { "isset", rule_function_isset_callback },
  { "round", rule_function_round_callback }
};

unsigned int nr_rule_functions = sizeof(rule_functions)/sizeof(rule_functions[0]);
