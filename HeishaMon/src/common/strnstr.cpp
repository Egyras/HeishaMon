/*
  Copyright (C) CurlyMo

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <stdint.h>
#include <string.h>
#include <stdio.h>

unsigned char *strnstr(unsigned char *str1, const char *str2, uint16_t size) {
  uint16_t a = 0, b = 0, c = 0;
  uint16_t len = strlen(str2);
  for(a=0;a<size;a++) {
    char ch = str1[a];
    if(ch == str2[0] && a+len <= size) {
      c = a;
      a++;
      for(b=1;b<len;a++, b++) {
        ch = str1[a];
        if(str2[b] != ch) {
          break;
        }
      }
      if(b == len) {
        return &str1[a-len];
      }
      a = c;
    }
  }
  return NULL;
}
