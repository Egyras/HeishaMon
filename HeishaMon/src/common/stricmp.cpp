/*
  Copyright (C) CurlyMo

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <stdlib.h>
#include <ctype.h>

int stricmp(char const *a, char const *b) {
  if(a == NULL || b == NULL) {
    return -1;
  }

  for(;a; a++, b++) {
    int d = tolower(*a) - tolower(*b);
    if(d != 0 || !*a) {
      return d;
    }
  }
  return -1;
}
