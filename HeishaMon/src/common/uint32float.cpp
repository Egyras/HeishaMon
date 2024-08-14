/*
  Copyright (C) CurlyMo

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <stdint.h>

void uint322float(uint32_t in, float *out) {
  union {
    uint32_t from;
    float    to;
  } pun = { .from = in };
  *out = pun.to;
}

void float2uint32(float in, uint32_t *out) {
  union {
    float    from;
    uint32_t to;
  } pun = { .from = in };
  *out = pun.to;
}