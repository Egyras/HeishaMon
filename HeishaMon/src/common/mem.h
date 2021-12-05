/*
  Copyright (C) CurlyMo

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _MEM_H_
#define _MEM_H_

unsigned int alignedbytes(int v);
unsigned int alignedbuffer(int v);

#define OUT_OF_MEMORY while(0) { }

#define STRDUP strdup
#define REALLOC realloc
#define CALLOC calloc
#define MALLOC malloc
#define FREE(a) do { free(a); (a) = NULL; } while(0)

#endif
