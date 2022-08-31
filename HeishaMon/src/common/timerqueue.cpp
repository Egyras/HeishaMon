/*
  Copyright (C) CurlyMo

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifdef ESP8266
  #include <Arduino.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#include "mem.h"
#include "timerqueue.h"

static unsigned int lasttime = 0;
static unsigned int *calls = NULL;
static unsigned int nrcalls = 0;

#ifndef ESP8266
static unsigned int micros() {
  struct timeval tv;
  gettimeofday(&tv,NULL);

  return 1000000 * tv.tv_sec + tv.tv_usec;;
}
#endif

static void timerqueue_sort() {
  int matched = 1;
  while(matched) {
    int a = 0;
    matched = 0;
    for(a=0;a<timerqueue_size-1;a++) {
      if(timerqueue[a]->remove < timerqueue[a+1]->remove ||
         (timerqueue[a]->remove == timerqueue[a+1]->remove && timerqueue[a]->sec > timerqueue[a+1]->sec) ||
         (timerqueue[a]->remove == timerqueue[a+1]->remove && timerqueue[a]->sec == timerqueue[a+1]->sec && timerqueue[a]->usec > timerqueue[a+1]->usec)) {
        struct timerqueue_t *node = timerqueue[a+1];
        timerqueue[a+1] = timerqueue[a];
        timerqueue[a] = node;
        matched = 1;
        break;
      }
    }
  }
}

struct timerqueue_t *timerqueue_pop() {
  if(timerqueue_size == 0) {
    return NULL;
  }
  struct timerqueue_t *x = timerqueue[0];
  timerqueue[0] = timerqueue[timerqueue_size-1];

  timerqueue_size--;

  if(timerqueue_size == 0) {
    free(timerqueue);
    timerqueue = NULL;
  } else {
    if((timerqueue = (struct timerqueue_t **)realloc(timerqueue, sizeof(struct timerqueue_t *)*timerqueue_size)) == NULL) {
  #ifdef ESP8266
      Serial.printf("Out of memory %s:#%d\n", __FUNCTION__, __LINE__);
      ESP.restart();
      exit(-1);
  #else
      fprintf(stderr, "Out of memory %s:#%d\n", __FUNCTION__, __LINE__);
      exit(-1);
  #endif
    }
  }

  int a = 0;
  for(a=0;a<timerqueue_size;a++) {
    timerqueue[a]->sec -= x->sec;
    timerqueue[a]->usec -= x->usec;
    if(timerqueue[a]->usec < 0) {
      timerqueue[a]->sec -= 1;
      timerqueue[a]->usec += 1000000;
    }
  }

  timerqueue_sort();

  return x;
}

struct timerqueue_t *timerqueue_peek() {
  if(timerqueue_size == 0) {
    return NULL;
  }
  return timerqueue[0];
}

void timerqueue_insert(int sec, int usec, int nr) {
  struct timerqueue_t *node = NULL;
  int a = 0, matched = 0, x = 0, y = 0;

  for(a=0;a<timerqueue_size;a++) {
    if(timerqueue[a]->nr == nr) {
      timerqueue[a]->sec = sec;
      timerqueue[a]->usec = usec;
      if(sec <= 0 && usec <= 0) {
        timerqueue[a]->remove = 1;
        for(x=0;x<nrcalls;x++) {
          if(calls[x] == timerqueue[a]->nr) {
            if(nrcalls > 0) {
              for(y=x;y<nrcalls-1;y++) {
                calls[y] = calls[y+1];
              }
              nrcalls--;
            }
            break;
          }
        }
      }
      timerqueue_sort();
      matched = 1;
      break;
    }
  }

  if(matched == 1) {
    while((node = timerqueue_peek()) != NULL) {
      if(node->remove == 1) {
        struct timerqueue_t *node = timerqueue_pop();
        free(node);
      } else {
        break;
      }
    }

    return;
  } else if(sec == 0 && usec == 0) {
    return;
  }

  if((timerqueue = (struct timerqueue_t **)realloc(timerqueue, sizeof(struct timerqueue_t *)*(timerqueue_size+1))) == NULL) {
#ifdef ESP8266
    Serial.printf("Out of memory %s:#%d\n", __FUNCTION__, __LINE__);
    ESP.restart();
    exit(-1);
#else
    fprintf(stderr, "Out of memory %s:#%d\n", __FUNCTION__, __LINE__);
    exit(-1);
#endif
  }

  node = (struct timerqueue_t *)malloc(sizeof(struct timerqueue_t));
  if(node == NULL) {
#ifdef ESP8266
    Serial.printf("Out of memory %s:#%d\n", __FUNCTION__, __LINE__);
    ESP.restart();
    exit(-1);
#else
    fprintf(stderr, "Out of memory %s:#%d\n", __FUNCTION__, __LINE__);
    exit(-1);
#endif
  }
  memset(node, 0, sizeof(struct timerqueue_t));
  node->sec = sec;
  node->usec = usec;
  node->nr = nr;

  timerqueue[timerqueue_size++] = node;
  timerqueue_sort();
}

void timerqueue_update(void) {
  struct timeval tv;
  unsigned int curtime = 0;

  curtime = micros();

  unsigned int diff = curtime - lasttime;
  unsigned int sec = diff / 1000000;
  unsigned int usec = diff - ((diff / 1000000) * 1000000);
  int a = 0, x = 0;

  lasttime = curtime;

  for(a=0;a<timerqueue_size;a++) {
    timerqueue[a]->sec -= sec;
    timerqueue[a]->usec -= usec;
    if(timerqueue[a]->usec < 0) {
      timerqueue[a]->usec = 1000000 + timerqueue[a]->usec;
      timerqueue[a]->sec -= 1;
    }

    if(timerqueue[a]->sec < 0 || (timerqueue[a]->sec == 0 && timerqueue[a]->usec <= 0)) {
      int nr = timerqueue[a]->nr;
      if((calls = (unsigned int *)realloc(calls, (nrcalls+1)*sizeof(int))) == NULL) {
#ifdef ESP8266
        Serial.printf("Out of memory %s:#%d\n", __FUNCTION__, __LINE__);
        ESP.restart();
        exit(-1);
#else
        fprintf(stderr, "Out of memory %s:#%d\n", __FUNCTION__, __LINE__);
        exit(-1);
#endif
      }
      calls[nrcalls++] = nr;
    }
  }
  for(a=0;a<timerqueue_size;a++) {
    if(timerqueue[a]->sec < 0 || (timerqueue[a]->sec == 0 && timerqueue[a]->usec == 0)) {
      struct timerqueue_t *node = timerqueue_pop();
      free(node);
      a--;
    }
  }
  while(nrcalls > 0) {
    timer_cb(calls[0]);
    if(nrcalls > 0) {
      for(a=0;a<nrcalls-1;a++) {
        calls[a] = calls[a+1];
      }
      nrcalls--;
    }
  }
  if(calls != NULL) {
    free(calls);
    calls = NULL;
  }
  nrcalls = 0;
}
