/*
  Copyright (C) CurlyMo

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/


#ifndef _RULES_H_
#define _RULES_H_

#include <stdint.h>
#include <unistd.h>
#include "stack.h"

#ifndef ESP8266
  #define F
  #define MEMPOOL_SIZE 16000
  typedef struct pbuf {
    struct pbuf *next;
    void *payload;
    uint16_t tot_len;
    uint16_t len;
    uint8_t type;
    uint8_t flags;
    uint16_t ref;
  } pbuf;

  typedef struct serial_t {
    void (*printf)(const char *fmt, ...);
    void (*println)(const char *val);
    void (*flush)(void);
  } serial_t;
  extern struct serial_t Serial;
  extern void *MMU_SEC_HEAP;
  uint8_t mmu_set_uint8(void *ptr, uint8_t src);
  uint8_t mmu_get_uint8(void *ptr);
  uint16_t mmu_set_uint16(void *ptr, uint16_t src);
  uint16_t mmu_get_uint16(void *ptr);
#else
  #include <Arduino.h>
  #include "lwip/pbuf.h"
  #ifdef MMU_SEC_HEAP_SIZE
    #define MEMPOOL_SIZE MMU_SEC_HEAP_SIZE
  #else
    #define MEMPOOL_SIZE 16000
  #endif
#endif

#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

/*
 * Max 32 tokens are allowed
 */
typedef enum {
  TOPERATOR = 1,
  TFUNCTION = 2,
  TSTRING = 3,
  TNUMBER = 4,
  TNUMBER1 = 5,
  TNUMBER2 = 6,
  TNUMBER3 = 7,
  TEOF = 8,
  LPAREN = 9,
  RPAREN = 10,
  TCOMMA = 11,
  TIF = 12,
  TELSE = 13,
  TELSEIF = 14,
  TTHEN = 15,
  TEVENT = 16,
  TEND = 17,
  TVAR = 18,
  TASSIGN = 19,
  TSEMICOLON = 20,
  TTRUE = 21,
  TFALSE = 22,
  TSTART = 23,
  TVALUE = 24,
  VCHAR = 25,
  VPTR = 26,
  VINTEGER = 27,
  VFLOAT = 28,
  VNULL = 29
} token_types;

typedef enum {
  OP_EQ = 1,
  OP_NE = 2,
  OP_LT = 3,
  OP_LE = 4,
  OP_GT = 5,
  OP_GE = 6,
  OP_AND = 7,
  OP_OR = 8,
  OP_SUB = 9,
  OP_ADD = 10,
  OP_DIV = 11,
  OP_MUL = 12,
  OP_POW = 13,
  OP_MOD = 14,
  OP_TEST = 15,
  OP_JMP = 16,
  OP_SETVAL = 17,
  OP_GETVAL = 18,
  OP_PUSH = 19,
  OP_CALL = 20,
  OP_CLEAR = 21,
  OP_RET = 22
} opcodes;

typedef struct rules_t {
  /* --- PUBLIC MEMBERS --- */

   /* To what rule do we return after
    * being called from another rule.
    */
  struct {
    struct rules_t *go;
    struct rules_t *ret;
  } __attribute__((aligned(4))) ctx;
#ifndef NON32XFER_HANDLER
  uint32_t nr;
#else
  uint8_t nr;
#endif

  const char *name;

  /* --- PRIVATE MEMBERS --- */

  /* Continue here after we processed
   * another rule call.
   */
  uint16_t cont;

  void *userdata;

  struct rule_stack_t bc;
  struct rule_stack_t *heap;

} __attribute__((aligned(4))) rules_t;

typedef struct rule_options_t {
  /*
   * Identifying callbacks
   */
  int8_t (*is_variable_cb)(char *text, uint16_t size);
  int8_t (*is_event_cb)(char *text, uint16_t size);

  int8_t (*vm_value_set)(struct rules_t *obj);
  int8_t (*vm_value_get)(struct rules_t *obj);

  /*
   * Events
   */
  int8_t (*event_cb)(struct rules_t *obj, char *name);
  void (*done_cb)(struct rules_t *obj);
} rule_options_t;

extern struct rule_options_t rule_options;

int8_t rule_token(struct rule_stack_t *obj, uint16_t pos, unsigned char **out);
const char *rule_by_nr(struct rules_t **rule, uint8_t nrrules, uint8_t nr);
int8_t rule_by_name(struct rules_t **rule, uint8_t nrrules, char *name);
int8_t rule_initialize(struct pbuf *input, struct rules_t ***rules, uint8_t *nrrules, struct pbuf *mempool, void *userdata);
int8_t rule_run(struct rules_t *rule, uint8_t validate);
void rules_gc(struct rules_t ***rules, uint8_t *nrrules);

int8_t rules_pushnil(struct rules_t *obj);
int8_t rules_pushfloat(struct rules_t *obj, float nr);
int8_t rules_pushinteger(struct rules_t *obj, int nr);
int8_t rules_pushstring(struct rules_t *obj, char *str);

void rules_ref(const char *str);
void rules_unref(const char *str);

int rules_tointeger(struct rules_t *obj, int8_t pos);
float rules_tofloat(struct rules_t *obj, int8_t pos);
const char *rules_tostring(struct rules_t *obj, int8_t pos);

void rules_remove(struct rules_t *rule, int8_t pos);
uint8_t rules_gettop(struct rules_t *rule);
uint8_t rules_type(struct rules_t *rule, int8_t pos);

#if defined(DEBUG) || defined(COVERALLS)
uint16_t rules_memused(void);
#endif

#endif
