/*
  Copyright (C) CurlyMo

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <Arduino.h>
#include <LittleFS.h>

#include "src/common/mem.h"
#include "src/common/stricmp.h"
#include "src/common/strnicmp.h"
#include "src/common/log.h"
#include "src/common/timerqueue.h"
#include "src/rules/rules.h"

#include "dallas.h"
#include "webfunctions.h"
#include "decode.h"
#include "HeishaOT.h"
#include "commands.h"

#define MAXCOMMANDSINBUFFER 10
#define OPTDATASIZE 20

bool send_command(byte* command, int length);

extern int dallasDevicecount;
extern dallasDataStruct *actDallasData;
extern settingsStruct heishamonSettings;
extern char actData[DATASIZE];
extern char actOptData[OPTDATASIZE];
extern char actDataExtra[DATASIZE];
extern String openTherm[2];
static uint8_t parsing = 0;

typedef struct vm_gvchar_t {
  VM_GENERIC_FIELDS
  uint8_t rule;
  char value[];
} __attribute__((packed)) vm_gvchar_t;

typedef struct vm_gvnull_t {
  VM_GENERIC_FIELDS
  uint8_t rule;
} __attribute__((packed)) vm_gvnull_t;

typedef struct vm_gvinteger_t {
  VM_GENERIC_FIELDS
  uint8_t rule;
  int value;
} __attribute__((packed)) vm_gvinteger_t;

typedef struct vm_gvfloat_t {
  VM_GENERIC_FIELDS
  uint8_t rule;
  float value;
} __attribute__((packed)) vm_gvfloat_t;

static struct rules_t **rules = NULL;
static int nrrules = 0;

typedef struct varstack_t {
  unsigned int nrbytes;
  unsigned int bufsize;
  unsigned char *stack;
} varstack_t;

static struct varstack_t global_varstack;

static struct vm_vinteger_t vinteger;
static struct vm_vfloat_t vfloat;
static struct vm_vnull_t vnull;

struct rule_options_t rule_options;
unsigned char *mempool = (unsigned char *)MMU_SEC_HEAP;
unsigned int memptr = 0;

static void vm_global_value_prt(char *out, int size);

// static int readRuleFromFS(int i) {
  // char fname[24];
  // memset(&fname, 0, sizeof(fname));
  // sprintf_P((char *)&fname, PSTR("/rule%d.bc"), i);
  // File f = LittleFS.open(fname, "r");
  // if(!f) {
    // logprintf_P(F("failed to open file: %s"), fname);
    // return -1;
  // }
  // FREE(rules[i]->ast.buffer);
  // if((rules[i]->ast.buffer = (unsigned char *)MALLOC(rules[i]->ast.bufsize)) == NULL) {
    // OUT_OF_MEMORY
  // }
  // memset(rules[i]->ast.buffer, 0, rules[i]->ast.bufsize);
  // f.readBytes((char *)rules[i]->ast.buffer, rules[i]->ast.bufsize);
  // f.close();
  // return 0;
// }

// static int writeRuleToFS(int i) {
  // char fname[24];
  // memset(&fname, 0, sizeof(fname));
  // sprintf_P((char *)&fname, PSTR("/rule%d.bc"), i);
  // File f = LittleFS.open(fname, "w");
  // if(!f) {
    // logprintf_P(F("failed to open file: %s"), fname);
    // return -1;
  // }
  // f.write((char *)rules[i]->ast.buffer, rules[i]->ast.bufsize);
  // f.close();
  // return 0;
// }

static int get_event(struct rules_t *obj) {
  struct vm_tstart_t *start = (struct vm_tstart_t *)&obj->ast.buffer[0];
  if(obj->ast.buffer[start->go] != TEVENT) {
    return -1;
  } else {
    return start->go;
  }
}

static int is_variable(char *text, unsigned int *pos, unsigned int size) {
  int i = 1, x = 0, match = 0;

  if(size == strlen_P(PSTR("ds18b20#2800000000000000")) && strncmp_P((const char *)&text[*pos], PSTR("ds18b20#"), 8) == 0) {
    return 24;
  } else if(text[*pos] == '$' || text[*pos] == '#' || text[*pos] == '@' || text[*pos] == '%' || text[*pos] == '?') {
    while(isalnum(text[*pos+i])) {
      i++;
    }

    if(text[*pos] == '%') {
      if(size == 5 && strnicmp(&text[(*pos)+1], "hour", 4) == 0) {
        return 5;
      }
      if(size == 5 && strnicmp(&text[(*pos)+1], "minute", 6) == 0) {
        return 7;
      }
      if(size == 6 && strnicmp(&text[(*pos)+1], "month", 5) == 0) {
        return 6;
      }
      if(size == 6 && strnicmp(&text[(*pos)+1], "day", 3) == 0) {
        return 4;
      }
    }

    if(text[*pos] == '@') {
      int nrcommands = sizeof(commands)/sizeof(commands[0]);
      for(x=0;x<nrcommands;x++) {
        cmdStruct cmd;
        memcpy_P(&cmd, &commands[x], sizeof(cmd));
        size_t len = strlen(cmd.name);
        if(size-1 == len && strnicmp(&text[(*pos)+1], cmd.name, len) == 0) {
          i = len+1;
          match = 1;
          break;
        }
      }

      int nroptcommands = sizeof(optionalCommands)/sizeof(optionalCommands[0]);
      for(x=0;x<nroptcommands;x++) {
        optCmdStruct cmd;
        memcpy_P(&cmd, &optionalCommands[x], sizeof(cmd));
        size_t len = strlen(cmd.name);
        if(size-1 == len && strnicmp(&text[(*pos)+1], cmd.name, len) == 0) {
          i = len+1;
          match = 1;
          break;
        }
      }
      if(match == 0) {
        int nrtopics = sizeof(topics)/sizeof(topics[0]);
        for(x=0;x<nrtopics;x++) {
          char cpy[MAX_TOPIC_LEN];
          memcpy_P(&cpy, topics[x], MAX_TOPIC_LEN);
          size_t len = strlen(cpy);
          if(size-1 == len && strnicmp(&text[(*pos)+1], cpy, len) == 0) {
            i = len+1;
            match = 1;
            break;
          }
        }
      }
      if(match == 0) {
        int nrtopics = sizeof(optTopics)/sizeof(optTopics[0]);
        for(x=0;x<nrtopics;x++) {
          char cpy[MAX_TOPIC_LEN];
          memcpy_P(&cpy, optTopics[x], MAX_TOPIC_LEN);
          size_t len = strlen(cpy);
          if(size-1 == len && strnicmp(&text[(*pos)+1], cpy, len) == 0) {
            i = len+1;
            match = 1;
            break;
          }
        }
      }
      if(match == 0) {
        int nrtopics = sizeof(xtopics)/sizeof(xtopics[0]);
        for(x=0;x<nrtopics;x++) {
          char cpy[MAX_TOPIC_LEN];
          memcpy_P(&cpy, xtopics[x], MAX_TOPIC_LEN);
          size_t len = strlen(cpy);
          if(size-1 == len && strnicmp(&text[(*pos)+1], cpy, len) == 0) {
            i = len+1;
            match = 1;
            break;
          }
        }
      }
      if(match == 0) {
        return -1;
      }
    }
    if(text[*pos] == '?') {
      int x = 0;
      while(heishaOTDataStruct[x].name != NULL) {
        size_t len = strlen(heishaOTDataStruct[x].name);
        if(size-1 == len && strnicmp(&text[(*pos)+1], heishaOTDataStruct[x].name, len) == 0) {
          i = len+1;
          match = 1;
          break;
        }
        x++;
      }
      if(match == 0) {
        return -1;
      }
    }

    return i;
  }
  return -1;
}

static int is_event(char *text, unsigned int *pos, unsigned int size) {
  int i = 1, x = 0, match = 0;
  if(text[*pos] == '@') {
    int nrcommands = sizeof(commands)/sizeof(commands[0]);
    for(x=0;x<nrcommands;x++) {
      cmdStruct cmd;
      memcpy_P(&cmd, &commands[x], sizeof(cmd));
      size_t len = strlen(cmd.name);
      if(size-1 == len && strnicmp(&text[(*pos)+1], cmd.name, len) == 0) {
        i = len+1;
        match = 1;
        break;
      }
    }

    if(match == 0) {
      int nroptcommands = sizeof(optionalCommands)/sizeof(optionalCommands[0]);
      for(x=0;x<nroptcommands;x++) {
        optCmdStruct cmd;
        memcpy_P(&cmd, &optionalCommands[x], sizeof(cmd));
        size_t len = strlen(cmd.name);
        if(size-1 == len && strnicmp(&text[(*pos)+1], cmd.name, len) == 0) {
          i = len+1;
          match = 1;
          break;
        }
      }
    }
    if(match == 0) {
      int nrtopics = sizeof(topics)/sizeof(topics[0]);
      for(x=0;x<nrtopics;x++) {
        size_t len = strlen_P(topics[x]);
        char cpy[len];
        memcpy_P(&cpy, &topics[x], len);
        if(size-1 == len && strnicmp(&text[(*pos)+1], cpy, len) == 0) {
          i = len+1;
          match = 1;
          break;
        }
      }
    }
    if(match == 0) {
      int nrtopics = sizeof(optTopics)/sizeof(optTopics[0]);
      for(x=0;x<nrtopics;x++) {
        size_t len = strlen_P(optTopics[x]);
        char cpy[len];
        memcpy_P(&cpy, &optTopics[x], len);
        if(size-1 == len && strnicmp(&text[(*pos)+1], cpy, len) == 0) {
          i = len+1;
          match = 1;
          break;
        }
      }
    }
    if(match == 0) {
      int nrtopics = sizeof(xtopics)/sizeof(xtopics[0]);
      for(x=0;x<nrtopics;x++) {
        size_t len = strlen_P(xtopics[x]);
        char cpy[len];
        memcpy_P(&cpy, &xtopics[x], len);
        if(size-1 == len && strnicmp(&text[(*pos)+1], cpy, len) == 0) {
          i = len+1;
          match = 1;
          break;
        }
      }
    }
    if(match == 0) {
      return -1;
    }

    return i;
  }

  if(text[*pos] == '?') {
    int x = 0;
    while(heishaOTDataStruct[x].name != NULL) {
      size_t len = strlen(heishaOTDataStruct[x].name);
      if(size-1 == len && strnicmp(&text[(*pos)+1], heishaOTDataStruct[x].name, len) == 0) {
        i = len+1;
        match = 1;
        break;
      }
      x++;
    }
    if(match == 0) {
      return -1;
    }
    return i;
  }

  if(size == strlen_P(PSTR("ds18b20#2800000000000000")) && strncmp_P((const char *)&text[*pos], PSTR("ds18b20#"), 8) == 0) {
    return 24;
  }

  return size;
}

static int event_cb(struct rules_t *obj, char *name) {
  struct rules_t *called = NULL;
  int i = 0, x = 0;

  if(obj->caller > 0 && name == NULL) {
    called = rules[obj->caller-1];

    obj->caller = 0;

    return rule_run(called, 0);
  } else {
    for(x=0;x<nrrules;x++) {
      if(get_event(rules[x]) > -1) {
        if(strnicmp(name, (char *)&rules[x]->ast.buffer[get_event(rules[x])+5], strlen((char *)&rules[x]->ast.buffer[get_event(rules[x])+5])) == 0) {
          called = rules[x];
          break;
        }
      }
      if(called != NULL) {
        break;
      }
    }

    if(called != NULL) {
      called->caller = obj->nr;

      return rule_run(called, 0);
    } else {
      return rule_run(obj, 0);
    }
  }
}

static void vm_value_clr(struct rules_t *obj, uint16_t token) {
  struct varstack_t *varstack = (struct varstack_t *)obj->userdata;
  struct vm_tvar_t *var = (struct vm_tvar_t *)&obj->ast.buffer[token];

  if(var->token[1] == '$') {
    var->value = 0;
  }
}

static void vm_value_cpy(struct rules_t *obj, uint16_t token) {
  struct varstack_t *varstack = (struct varstack_t *)obj->userdata;
  struct vm_tvar_t *var = (struct vm_tvar_t *)&obj->ast.buffer[token];
  int x = 0;
  if(var->token[0] == '$') {
    varstack = (struct varstack_t *)obj->userdata;
    for(x=4;alignedbytes(x)<varstack->nrbytes;x++) {
      x = alignedbytes(x);
      switch(varstack->stack[x]) {
        case VINTEGER: {
          struct vm_vinteger_t *val = (struct vm_vinteger_t *)&varstack->stack[x];
          struct vm_tvar_t *foo = (struct vm_tvar_t *)&obj->ast.buffer[val->ret];
          if(stricmp((char *)foo->token, (char *)&var->token) == 0 && val->ret != token) {
            var->value = foo->value;
            val->ret = token;
            foo->value = 0;
            return;
          }
          x += sizeof(struct vm_vinteger_t)-1;
        } break;
        case VFLOAT: {
          struct vm_vfloat_t *val = (struct vm_vfloat_t *)&varstack->stack[x];
          struct vm_tvar_t *foo = (struct vm_tvar_t *)&obj->ast.buffer[val->ret];

          if(stricmp((char *)foo->token, (char *)var->token) == 0 && val->ret != token) {
            var->value = foo->value;
            val->ret = token;
            foo->value = 0;
            return;
          }
          x += sizeof(struct vm_vfloat_t)-1;
        } break;
        case VNULL: {
          struct vm_vnull_t *val = (struct vm_vnull_t *)&varstack->stack[x];
          struct vm_tvar_t *foo = (struct vm_tvar_t *)&obj->ast.buffer[val->ret];
          if(stricmp((char *)foo->token, (char *)&var->token) == 0 && val->ret != token) {
            var->value = foo->value;
            val->ret = token;
            foo->value = 0;
            return;
          }
          x += sizeof(struct vm_vnull_t)-1;
        } break;
        default: {
          return;
        } break;
      }
    }
  } else if(var->token[0] == '#') {
    varstack = &global_varstack;

    for(x=4;alignedbytes(x)<varstack->nrbytes;x++) {
      x = alignedbytes(x);
      switch(varstack->stack[x]) {
        case VINTEGER: {
          struct vm_gvinteger_t *val = (struct vm_gvinteger_t *)&varstack->stack[x];
          struct vm_tvar_t *foo = (struct vm_tvar_t *)&rules[val->rule-1]->ast.buffer[val->ret];

          if(stricmp((char *)foo->token, (char *)var->token) == 0 && val->ret != token) {
            var->value = x;
            val->ret = token;
            val->rule = obj->nr;
            return;
          }
          x += sizeof(struct vm_gvinteger_t)-1;
        } break;
        case VFLOAT: {
          struct vm_gvfloat_t *val = (struct vm_gvfloat_t *)&varstack->stack[x];
          struct vm_tvar_t *foo = (struct vm_tvar_t *)&rules[val->rule-1]->ast.buffer[val->ret];

          if(stricmp((char *)foo->token, (char *)var->token) == 0 && val->ret != token) {
            var->value = x;
            val->ret = token;
            val->rule = obj->nr;
            return;
          }
          x += sizeof(struct vm_gvfloat_t)-1;
        } break;
        case VNULL: {
          struct vm_gvnull_t *val = (struct vm_gvnull_t *)&varstack->stack[x];
          struct vm_tvar_t *foo = (struct vm_tvar_t *)&rules[val->rule-1]->ast.buffer[val->ret];

          if(stricmp((char *)foo->token, (char *)var->token) == 0 && val->ret != token) {
            var->value = x;
            val->ret = token;
            val->rule = obj->nr;
            return;
          }
          x += sizeof(struct vm_gvnull_t)-1;
        } break;
        default: {
          return;
        } break;
      }
    }
  }
}

static unsigned char *vm_value_get(struct rules_t *obj, uint16_t token) {
  struct vm_tvar_t *node = (struct vm_tvar_t *)&obj->ast.buffer[token];
  int i = 0;
  if(node->token[0] == '$') {
    struct varstack_t *varstack = (struct varstack_t *)obj->userdata;
    if(node->value == 0) {
      int ret = varstack->nrbytes, suffix = 0;
      unsigned int size = alignedbytes(varstack->nrbytes + sizeof(struct vm_vnull_t));
      if((varstack->stack = (unsigned char *)REALLOC(varstack->stack, alignedbuffer(size))) == NULL) {
        OUT_OF_MEMORY /*LCOV_EXCL_LINE*/
      }
      struct vm_vnull_t *value = (struct vm_vnull_t *)&varstack->stack[ret];
      value->type = VNULL;
      value->ret = token;
      node->value = ret;

      varstack->nrbytes = size;
      varstack->bufsize = alignedbuffer(size);
    }

    const char *key = (char *)node->token;
    switch(varstack->stack[node->value]) {
      case VINTEGER: {
        struct vm_vinteger_t *na = (struct vm_vinteger_t *)&varstack->stack[node->value];
      } break;
      case VFLOAT: {
        struct vm_vfloat_t *na = (struct vm_vfloat_t *)&varstack->stack[node->value];
      } break;
      case VNULL: {
        struct vm_vnull_t *na = (struct vm_vnull_t *)&varstack->stack[node->value];
      } break;
      case VCHAR: {
        struct vm_vchar_t *na = (struct vm_vchar_t *)&varstack->stack[node->value];
      } break;
    }

    return &varstack->stack[node->value];

  }
  if(node->token[0] == '#') {
    struct varstack_t *varstack = &global_varstack;
    if(node->value == 0) {
      int ret = varstack->nrbytes, suffix = 0;

      unsigned int size = alignedbytes(varstack->nrbytes + sizeof(struct vm_gvnull_t));
      if((varstack->stack = (unsigned char *)REALLOC(varstack->stack, alignedbuffer(size))) == NULL) {
        OUT_OF_MEMORY /*LCOV_EXCL_LINE*/
      }
      struct vm_gvnull_t *value = (struct vm_gvnull_t *)&varstack->stack[ret];
      value->type = VNULL;
      value->ret = token;
      value->rule = obj->nr;
      node->value = ret;

      varstack->nrbytes = size;
      varstack->bufsize = alignedbuffer(size);
    }

    const char *key = (char *)node->token;
    switch(varstack->stack[node->value]) {
      case VINTEGER: {
        struct vm_gvinteger_t *na = (struct vm_gvinteger_t *)&varstack->stack[node->value];

        memset(&vinteger, 0, sizeof(struct vm_vinteger_t));
        vinteger.type = VINTEGER;
        vinteger.value = (int)na->value;

        return (unsigned char *)&vinteger;
      } break;
      case VFLOAT: {
        struct vm_gvfloat_t *na = (struct vm_gvfloat_t *)&varstack->stack[node->value];

        memset(&vfloat, 0, sizeof(struct vm_vfloat_t));
        vfloat.type = VFLOAT;
        vfloat.value = na->value;

        return (unsigned char *)&vfloat;
      } break;
      case VNULL: {
        struct vm_gvnull_t *na = (struct vm_gvnull_t *)&varstack->stack[node->value];

        memset(&vnull, 0, sizeof(struct vm_vnull_t));
        vnull.type = VNULL;

        return (unsigned char *)&vnull;
      } break;
      case VCHAR: {
        return NULL;
      } break;
    }

    return NULL;
  }
  if(node->token[0] == '@') {
    for(i=0;i<NUMBER_OF_TOPICS;i++) {
      char cpy[MAX_TOPIC_LEN];
      memcpy_P(&cpy, topics[i], MAX_TOPIC_LEN);
      if(stricmp(cpy, (char *)&node->token[1]) == 0) {
        String dataValue = actData[0] == '\0' ? "" : getDataValue(actData, i);
        char *str = (char *)dataValue.c_str();
        if(strlen(str) == 0) {
          memset(&vnull, 0, sizeof(struct vm_vnull_t));
          vnull.type = VNULL;
          vnull.ret = token;

          return (unsigned char *)&vnull;
        } else {
          float var = atof(str);
          float nr = 0;

          // mosquitto_publish
          if(modff(var, &nr) == 0) {
            memset(&vinteger, 0, sizeof(struct vm_vinteger_t));
            vinteger.type = VINTEGER;
            vinteger.value = (int)var;

            return (unsigned char *)&vinteger;
          } else {
            memset(&vfloat, 0, sizeof(struct vm_vfloat_t));
            vfloat.type = VFLOAT;
            vfloat.value = var;

            return (unsigned char *)&vfloat;
          }
        }
      }
    }
    for(i=0;i<NUMBER_OF_OPT_TOPICS;i++) {
      char cpy[MAX_TOPIC_LEN];
      memcpy_P(&cpy, topics[i], MAX_TOPIC_LEN);
      if(stricmp(cpy, (char *)&node->token[1]) == 0) {
        String dataValue = actOptData[0] == '\0' ? "" : getOptDataValue(actOptData, i);
        char *str = (char *)dataValue.c_str();
        if(strlen(str) == 0) {
          memset(&vnull, 0, sizeof(struct vm_vnull_t));
          vnull.type = VNULL;
          vnull.ret = token;

          return (unsigned char *)&vnull;
        } else {
          float var = atof(str);
          float nr = 0;

          // mosquitto_publish
          if(modff(var, &nr) == 0) {
            memset(&vinteger, 0, sizeof(struct vm_vinteger_t));
            vinteger.type = VINTEGER;
            vinteger.value = (int)var;

            return (unsigned char *)&vinteger;
          } else {
            memset(&vfloat, 0, sizeof(struct vm_vfloat_t));
            vfloat.type = VFLOAT;
            vfloat.value = var;

            return (unsigned char *)&vfloat;
          }
        }
      }
    }
    for(i=0;i<NUMBER_OF_TOPICS_EXTRA;i++) {
      char cpy[MAX_TOPIC_LEN];
      memcpy_P(&cpy, xtopics[i], MAX_TOPIC_LEN);
      if(stricmp(cpy, (char *)&node->token[1]) == 0) {
        String dataValue = actDataExtra[0] == '\0' ? "" : getDataValueExtra(actDataExtra, i);
        char *str = (char *)dataValue.c_str();
        if(strlen(str) == 0) {
          memset(&vnull, 0, sizeof(struct vm_vnull_t));
          vnull.type = VNULL;
          vnull.ret = token;

          return (unsigned char *)&vnull;
        } else {
          float var = atof(str);
          float nr = 0;

          // mosquitto_publish
          if(modff(var, &nr) == 0) {
            memset(&vinteger, 0, sizeof(struct vm_vinteger_t));
            vinteger.type = VINTEGER;
            vinteger.value = (int)var;

            return (unsigned char *)&vinteger;
          } else {
            memset(&vfloat, 0, sizeof(struct vm_vfloat_t));
            vfloat.type = VFLOAT;
            vfloat.value = var;

            return (unsigned char *)&vfloat;
          }
        }
      }
    }
  }
  if(node->token[0] == '%') {
    if(stricmp((char *)&node->token[1], "hour") == 0) {
      time_t now = time(NULL);
      struct tm *tm_struct = localtime(&now);

      memset(&vinteger, 0, sizeof(struct vm_vinteger_t));
      vinteger.type = VINTEGER;
      vinteger.value = (int)tm_struct->tm_hour;
      return (unsigned char *)&vinteger;
    } else if(stricmp((char *)&node->token[1], "minute") == 0) {
      time_t now = time(NULL);
      struct tm *tm_struct = localtime(&now);

      memset(&vinteger, 0, sizeof(struct vm_vinteger_t));
      vinteger.type = VINTEGER;
      vinteger.value = (int)tm_struct->tm_min;
      return (unsigned char *)&vinteger;
    } else if(stricmp((char *)&node->token[1], "month") == 0) {
      time_t now = time(NULL);
      struct tm *tm_struct = localtime(&now);

      memset(&vinteger, 0, sizeof(struct vm_vinteger_t));
      vinteger.type = VINTEGER;
      vinteger.value = (int)tm_struct->tm_mon + 1;
      return (unsigned char *)&vinteger;
    } else if(stricmp((char *)&node->token[1], "day") == 0) {
      time_t now = time(NULL);
      struct tm *tm_struct = localtime(&now);

      memset(&vinteger, 0, sizeof(struct vm_vinteger_t));
      vinteger.type = VINTEGER;
      vinteger.value = (int)tm_struct->tm_wday+1;
      return (unsigned char *)&vinteger;
    }
  }
  if(node->token[0] == '?') {
    int x = 0;
    while(heishaOTDataStruct[x].name != NULL) {
      if(heishaOTDataStruct[x].rw >= 2 && stricmp((char *)&node->token[1], heishaOTDataStruct[x].name) == 0) {
        if(heishaOTDataStruct[x].type == TBOOL) {
          memset(&vinteger, 0, sizeof(struct vm_vinteger_t));
          vinteger.type = VINTEGER;
          vinteger.value = (int)heishaOTDataStruct[x].value.b;
          // printf("%s %s = %g\n", __FUNCTION__, (char *)node->token, var);
          return (unsigned char *)&vinteger;
        }
        if(heishaOTDataStruct[x].type == TFLOAT) {
          memset(&vfloat, 0, sizeof(struct vm_vfloat_t));
          vfloat.type = VFLOAT;
          vfloat.value = heishaOTDataStruct[x].value.f;
          // printf("%s %s = %g\n", __FUNCTION__, (char *)node->token, var);
          return (unsigned char *)&vfloat;
        }
        break;
      }
      x++;
    }
    logprintf_P(F("err: %s %d"), __FUNCTION__, __LINE__);
  }
  if(strncmp_P((const char *)node->token, PSTR("ds18b20#"), 8) == 0) {
    for(i=0;i<dallasDevicecount;i++) {
      if(strncmp(actDallasData[i].address, (const char *)&node->token[8], 16) == 0) {
        vfloat.type = VFLOAT;
        vfloat.value = actDallasData[i].temperature;
        // printf("%s %s = %g\n", __FUNCTION__, (char *)node->token, actDallasData[i].temperature);
        return (unsigned char *)&vfloat;
      }
    }

    memset(&vnull, 0, sizeof(struct vm_vnull_t));
    vnull.type = VNULL;

    return (unsigned char *)&vnull;
  }
  return NULL;
}

static int vm_value_del(struct rules_t *obj, uint16_t idx) {
  struct varstack_t *varstack = (struct varstack_t *)obj->userdata;
  int x = 0, ret = 0;

  if(idx == varstack->nrbytes) {
    return -1;
  }
  switch(varstack->stack[idx]) {
    case VINTEGER: {
      ret = alignedbytes(sizeof(struct vm_vinteger_t));
      memmove(&varstack->stack[idx], &varstack->stack[idx+ret], varstack->nrbytes-idx-ret);
      if((varstack->stack = (unsigned char *)REALLOC(varstack->stack, alignedbuffer(varstack->nrbytes-ret))) == NULL) {
        OUT_OF_MEMORY /*LCOV_EXCL_LINE*/
      }
      varstack->nrbytes -= ret;
      varstack->bufsize = alignedbuffer(varstack->nrbytes);
    } break;
    case VFLOAT: {
      ret = alignedbytes(sizeof(struct vm_vfloat_t));
      memmove(&varstack->stack[idx], &varstack->stack[idx+ret], varstack->nrbytes-idx-ret);
      if((varstack->stack = (unsigned char *)REALLOC(varstack->stack,alignedbuffer(varstack->nrbytes-ret))) == NULL) {
        OUT_OF_MEMORY /*LCOV_EXCL_LINE*/
      }
      varstack->nrbytes -= ret;
      varstack->bufsize = alignedbuffer(varstack->nrbytes);
    } break;
    case VNULL: {
      ret = alignedbytes(sizeof(struct vm_vnull_t));
      memmove(&varstack->stack[idx], &varstack->stack[idx+ret], varstack->nrbytes-idx-ret);
      if((varstack->stack = (unsigned char *)REALLOC(varstack->stack, alignedbuffer(varstack->nrbytes-ret))) == NULL) {
        OUT_OF_MEMORY /*LCOV_EXCL_LINE*/
      }
      varstack->nrbytes -= ret;
      varstack->bufsize = alignedbuffer(varstack->nrbytes);
    } break;
    default: {
      return -1;
    } break;
  }

  /*
   * Values are linked back to their root node,
   * by their absolute position in the bytecode.
   * If a value is deleted, these positions changes,
   * so we need to update all nodes.
   */
  for(x=idx;alignedbytes(x)<varstack->nrbytes;x++) {
    x = alignedbytes(x);
    switch(varstack->stack[x]) {
      case VINTEGER: {
        struct vm_vinteger_t *node = (struct vm_vinteger_t *)&varstack->stack[x];
        if(node->ret > 0) {
          struct vm_tvar_t *tmp = (struct vm_tvar_t *)&obj->ast.buffer[node->ret];
          tmp->value = x;
        }
        x += sizeof(struct vm_vinteger_t)-1;
      } break;
      case VFLOAT: {
        struct vm_vfloat_t *node = (struct vm_vfloat_t *)&varstack->stack[x];
        if(node->ret > 0) {
          struct vm_tvar_t *tmp = (struct vm_tvar_t *)&obj->ast.buffer[node->ret];
          tmp->value = x;
        }
        x += sizeof(struct vm_vfloat_t)-1;
      } break;
      default: {
        return -1;
      } break;
    }
  }

  return ret;
}

static void vm_value_set(struct rules_t *obj, uint16_t token, uint16_t val) {
  struct varstack_t *varstack = NULL;
  struct vm_tvar_t *var = (struct vm_tvar_t *)&obj->ast.buffer[token];
  int ret = 0, x = 0, loop = 1;

  if(var->token[0] == '$') {
    varstack = (struct varstack_t *)obj->userdata;

    const char *key = (char *)var->token;
    switch(obj->varstack.buffer[val]) {
      case VINTEGER: {
        struct vm_vinteger_t *na = (struct vm_vinteger_t *)&obj->varstack.buffer[val];
      } break;
      case VFLOAT: {
        struct vm_vfloat_t *na = (struct vm_vfloat_t *)&obj->varstack.buffer[val];
      } break;
      case VNULL: {
        struct vm_vnull_t *na = (struct vm_vnull_t *)&obj->varstack.buffer[val];
      } break;
      case VCHAR: {
        struct vm_vchar_t *na = (struct vm_vchar_t *)&obj->varstack.buffer[val];
      } break;
    }

    /*
     * Remove previous value linked to
     * the variable being set.
     */
    for(x=4;alignedbytes(x)<varstack->nrbytes && loop == 1;x++) {
      x = alignedbytes(x);
      switch(varstack->stack[x]) {
        case VINTEGER: {
          struct vm_vinteger_t *node = (struct vm_vinteger_t *)&varstack->stack[x];
          struct vm_tvar_t *tmp = (struct vm_tvar_t *)&obj->ast.buffer[node->ret];
          if(stricmp((char *)var->token, (char *)tmp->token) == 0) {
            var->value = 0;
            vm_value_del(obj, x);
            loop = 0;
            break;
          }
          x += sizeof(struct vm_vinteger_t)-1;
        } break;
        case VFLOAT: {
          struct vm_vfloat_t *node = (struct vm_vfloat_t *)&varstack->stack[x];
          struct vm_tvar_t *tmp = (struct vm_tvar_t *)&obj->ast.buffer[node->ret];
          if(stricmp((char *)var->token, (char *)tmp->token) == 0) {
            var->value = 0;
            vm_value_del(obj, x);
            loop = 0;
            break;
          }
          x += sizeof(struct vm_vfloat_t)-1;
        } break;
        case VNULL: {
          struct vm_vnull_t *node = (struct vm_vnull_t *)&varstack->stack[x];
          struct vm_tvar_t *tmp = (struct vm_tvar_t *)&obj->ast.buffer[node->ret];
          if(stricmp((char *)var->token, (char *)tmp->token) == 0) {
            var->value = 0;
            vm_value_del(obj, x);
            loop = 0;
            break;
          }
          x += sizeof(struct vm_vnull_t)-1;
        } break;
        default: {
          return;
        } break;
      }
    }

    var = (struct vm_tvar_t *)&obj->ast.buffer[token];
    if(var->value > 0) {
      vm_value_del(obj, var->value);
    }
    var = (struct vm_tvar_t *)&obj->ast.buffer[token];

    ret = varstack->nrbytes;

    var->value = ret;

    switch(obj->varstack.buffer[val]) {
      case VINTEGER: {
        unsigned int size = alignedbytes(varstack->nrbytes+sizeof(struct vm_vinteger_t));
        if((varstack->stack = (unsigned char *)REALLOC(varstack->stack, alignedbuffer(size))) == NULL) {
          OUT_OF_MEMORY /*LCOV_EXCL_LINE*/
        }
        struct vm_vinteger_t *cpy = (struct vm_vinteger_t *)&obj->varstack.buffer[val];
        struct vm_vinteger_t *value = (struct vm_vinteger_t *)&varstack->stack[ret];
        value->type = VINTEGER;
        value->ret = token;
        value->value = (int)cpy->value;

        varstack->nrbytes = size;
        varstack->bufsize = alignedbuffer(size);
      } break;
      case VFLOAT: {
        unsigned int size = alignedbytes(varstack->nrbytes+sizeof(struct vm_vfloat_t));
        if((varstack->stack = (unsigned char *)REALLOC(varstack->stack, alignedbuffer(size))) == NULL) {
          OUT_OF_MEMORY /*LCOV_EXCL_LINE*/
        }
        struct vm_vfloat_t *cpy = (struct vm_vfloat_t *)&obj->varstack.buffer[val];
        struct vm_vfloat_t *value = (struct vm_vfloat_t *)&varstack->stack[ret];
        value->type = VFLOAT;
        value->ret = token;
        value->value = cpy->value;

        varstack->nrbytes = size;
        varstack->bufsize = alignedbuffer(size);
      } break;
      case VNULL: {
        unsigned int size = alignedbytes(varstack->nrbytes+sizeof(struct vm_vnull_t));
        if((varstack->stack = (unsigned char *)REALLOC(varstack->stack, alignedbuffer(size))) == NULL) {
          OUT_OF_MEMORY /*LCOV_EXCL_LINE*/
        }
        struct vm_vnull_t *value = (struct vm_vnull_t *)&varstack->stack[ret];
        value->type = VNULL;
        value->ret = token;
        varstack->nrbytes = size;
        varstack->bufsize = alignedbuffer(size);
      } break;
      default: {
        return;
      } break;
    }
  } else if(var->token[0] == '#') {
    varstack = &global_varstack;

    const char *key = (char *)var->token;
    switch(obj->varstack.buffer[val]) {
      case VINTEGER: {
        struct vm_vinteger_t *na = (struct vm_vinteger_t *)&obj->varstack.buffer[val];
      } break;
      case VFLOAT: {
        struct vm_vfloat_t *na = (struct vm_vfloat_t *)&obj->varstack.buffer[val];
      } break;
      case VCHAR: {
        struct vm_vchar_t *na = (struct vm_vchar_t *)&obj->varstack.buffer[val];
      } break;
      case VNULL: {
        struct vm_vnull_t *na = (struct vm_vnull_t *)&obj->varstack.buffer[val];
      } break;
    }

    var = (struct vm_tvar_t *)&obj->ast.buffer[token];
    int move = 0;
    for(x=4;alignedbytes(x)<varstack->nrbytes;x++) {
      x = alignedbytes(x);
      switch(varstack->stack[x]) {
        case VINTEGER: {
          struct vm_gvinteger_t *val = (struct vm_gvinteger_t *)&varstack->stack[x];
          struct vm_tvar_t *foo = (struct vm_tvar_t *)&rules[val->rule-1]->ast.buffer[val->ret];

          if(stricmp((char *)foo->token, (char *)var->token) == 0) {
            move = 1;

            ret = alignedbytes(sizeof(struct vm_gvinteger_t));
            memmove(&varstack->stack[x], &varstack->stack[x+ret], varstack->nrbytes-x-ret);
            if((varstack->stack = (unsigned char *)REALLOC(varstack->stack, alignedbuffer(varstack->nrbytes-ret))) == NULL) {
              OUT_OF_MEMORY /*LCOV_EXCL_LINE*/
            }
            varstack->nrbytes -= ret;
            varstack->bufsize = alignedbuffer(varstack->nrbytes);
          }
        } break;
        case VFLOAT: {
          struct vm_gvfloat_t *val = (struct vm_gvfloat_t *)&varstack->stack[x];
          struct vm_tvar_t *foo = (struct vm_tvar_t *)&rules[val->rule-1]->ast.buffer[val->ret];

          if(stricmp((char *)foo->token, (char *)var->token) == 0) {
            move = 1;

            ret = alignedbytes(sizeof(struct vm_gvfloat_t));
            memmove(&varstack->stack[x], &varstack->stack[x+ret], varstack->nrbytes-x-ret);
            if((varstack->stack = (unsigned char *)REALLOC(varstack->stack, alignedbuffer(varstack->nrbytes-ret))) == NULL) {
              OUT_OF_MEMORY /*LCOV_EXCL_LINE*/
            }
            varstack->nrbytes -= ret;
            varstack->bufsize = alignedbuffer(varstack->nrbytes);
          }
        } break;
        case VNULL: {
          struct vm_gvnull_t *val = (struct vm_gvnull_t *)&varstack->stack[x];
          struct vm_tvar_t *foo = (struct vm_tvar_t *)&rules[val->rule-1]->ast.buffer[val->ret];

          if(stricmp((char *)foo->token, (char *)var->token) == 0) {
            move = 1;

            ret = alignedbytes(sizeof(struct vm_gvnull_t));
            memmove(&varstack->stack[x], &varstack->stack[x+ret], varstack->nrbytes-x-ret);
            if((varstack->stack = (unsigned char *)REALLOC(varstack->stack, alignedbuffer(varstack->nrbytes-ret))) == NULL) {
              OUT_OF_MEMORY /*LCOV_EXCL_LINE*/
            }
            varstack->nrbytes -= ret;
            varstack->bufsize = alignedbuffer(varstack->nrbytes);
          }
        } break;
        default: {
          return;
        } break;
      }
      if(x == varstack->nrbytes) {
        break;
      }

      switch(varstack->stack[x]) {
        case VINTEGER: {
          if(move == 1 && x < varstack->nrbytes) {
            struct vm_gvinteger_t *node = (struct vm_gvinteger_t *)&varstack->stack[x];
            if(node->ret > 0) {
              struct vm_tvar_t *tmp = (struct vm_tvar_t *)&rules[node->rule-1]->ast.buffer[node->ret];
              tmp->value = x;
            }
          }
          x += sizeof(struct vm_gvinteger_t)-1;
        } break;
        case VFLOAT: {
          if(move == 1 && x < varstack->nrbytes) {
            struct vm_gvfloat_t *node = (struct vm_gvfloat_t *)&varstack->stack[x];
            if(node->ret > 0) {
              struct vm_tvar_t *tmp = (struct vm_tvar_t *)&rules[node->rule-1]->ast.buffer[node->ret];
              tmp->value = x;
            }
          }
          x += sizeof(struct vm_gvfloat_t)-1;
        } break;
        case VNULL: {
          if(move == 1 && x < varstack->nrbytes) {
            struct vm_gvnull_t *node = (struct vm_gvnull_t *)&varstack->stack[x];
            if(node->ret > 0) {
              struct vm_tvar_t *tmp = (struct vm_tvar_t *)&rules[node->rule-1]->ast.buffer[node->ret];
              tmp->value = x;
            }
          }
          x += sizeof(struct vm_gvnull_t)-1;
        } break;
      }
    }
    var = (struct vm_tvar_t *)&obj->ast.buffer[token];

    ret = varstack->nrbytes;

    var->value = ret;

    switch(obj->varstack.buffer[val]) {
      case VINTEGER: {
        unsigned int size = alignedbytes(varstack->nrbytes + sizeof(struct vm_gvinteger_t));
        if((varstack->stack = (unsigned char *)REALLOC(varstack->stack, alignedbuffer(size))) == NULL) {
          OUT_OF_MEMORY /*LCOV_EXCL_LINE*/
        }
        struct vm_vinteger_t *cpy = (struct vm_vinteger_t *)&obj->varstack.buffer[val];
        struct vm_gvinteger_t *value = (struct vm_gvinteger_t *)&varstack->stack[ret];
        value->type = VINTEGER;
        value->ret = token;
        value->value = (int)cpy->value;
        value->rule = obj->nr;

        varstack->nrbytes = size;
        varstack->bufsize = alignedbuffer(size);
      } break;
      case VFLOAT: {
        unsigned int size = alignedbytes(varstack->nrbytes + sizeof(struct vm_gvfloat_t));
        if((varstack->stack = (unsigned char *)REALLOC(varstack->stack, alignedbuffer(size))) == NULL) {
          OUT_OF_MEMORY /*LCOV_EXCL_LINE*/
        }
        struct vm_vfloat_t *cpy = (struct vm_vfloat_t *)&obj->varstack.buffer[val];
        struct vm_gvfloat_t *value = (struct vm_gvfloat_t *)&varstack->stack[ret];
        value->type = VFLOAT;
        value->ret = token;
        value->value = cpy->value;
        value->rule = obj->nr;

        varstack->nrbytes = size;
        varstack->bufsize = alignedbuffer(size);
      } break;
      case VNULL: {
        unsigned int size = alignedbytes(varstack->nrbytes + sizeof(struct vm_gvnull_t));
        if((varstack->stack = (unsigned char *)REALLOC(varstack->stack, alignedbuffer(size))) == NULL) {
          OUT_OF_MEMORY /*LCOV_EXCL_LINE*/
        }
        struct vm_gvnull_t *value = (struct vm_gvnull_t *)&varstack->stack[ret];
        value->type = VNULL;
        value->ret = token;
        value->rule = obj->nr;

        varstack->nrbytes = size;
        varstack->bufsize = alignedbuffer(size);
      } break;
      default: {
        return;
      } break;
    }
  } else if(var->token[0] == '@') {
    char *payload = NULL;
    unsigned int len = 0;

    switch(obj->varstack.buffer[val]) {
      case VINTEGER: {
        struct vm_vinteger_t *na = (struct vm_vinteger_t *)&obj->varstack.buffer[val];

        len = snprintf_P(NULL, 0, PSTR("%d"), (int)na->value);
        if((payload = (char *)MALLOC(len+1)) == NULL) {
          OUT_OF_MEMORY
        }
        snprintf_P(payload, len+1, PSTR("%d"), (int)na->value);

      } break;
      case VFLOAT: {
        struct vm_vfloat_t *na = (struct vm_vfloat_t *)&obj->varstack.buffer[val];

        len = snprintf_P(NULL, 0, PSTR("%g"), (float)na->value);
        if((payload = (char *)MALLOC(len+1)) == NULL) {
          OUT_OF_MEMORY
        }
        snprintf_P(payload, len+1, PSTR("%g"), (float)na->value);
      } break;
      case VCHAR: {
        struct vm_vchar_t *na = (struct vm_vchar_t *)&obj->varstack.buffer[val];

        len = snprintf_P(NULL, 0, PSTR("%s"), na->value);
        if((payload = (char *)MALLOC(len+1)) == NULL) {
          OUT_OF_MEMORY
        }
        snprintf_P(payload, len+1, PSTR("%s"), na->value);
      } break;
    }

    if(parsing == 0 && !heishamonSettings.listenonly) {
      unsigned char cmd[256] = { 0 };
      char log_msg[256] = { 0 };

      for(uint8_t x = 0; x < sizeof(commands) / sizeof(commands[0]); x++) {
        cmdStruct tmp;
        memcpy_P(&tmp, &commands[x], sizeof(tmp));
        if(stricmp((char *)&var->token[1], tmp.name) == 0) {
          uint16_t len = tmp.func(payload, cmd, log_msg);
          log_message(log_msg);
          send_command(cmd, len);
          break;
        }
      }

      memset(&cmd, 256, 0);
      memset(&log_msg, 256, 0);

      if(heishamonSettings.optionalPCB) {
        //optional commands
        for(uint8_t x = 0; x < sizeof(optionalCommands) / sizeof(optionalCommands[0]); x++) {
          optCmdStruct tmp;
          memcpy_P(&tmp, &optionalCommands[x], sizeof(tmp));
          if(stricmp((char *)&var->token[1], tmp.name) == 0) {
            uint16_t len = tmp.func(payload, log_msg);
            log_message(log_msg);
            break;
          }
        }
      }
    }
    FREE(payload);
  } else if(var->token[0] == '?') {
    int x = 0;
    while(heishaOTDataStruct[x].name != NULL) {
      if(heishaOTDataStruct[x].rw <= 2 && stricmp((char *)&var->token[1], heishaOTDataStruct[x].name) == 0) {
        if(heishaOTDataStruct[x].type == TBOOL) {
          switch(obj->varstack.buffer[val]) {
            case VINTEGER: {
              struct vm_vinteger_t *na = (struct vm_vinteger_t *)&obj->varstack.buffer[val];

              heishaOTDataStruct[x].value.b = (bool)na->value;
            } break;
            case VFLOAT: {
              struct vm_vfloat_t *na = (struct vm_vfloat_t *)&obj->varstack.buffer[val];

              heishaOTDataStruct[x].value.b = (bool)na->value;
            } break;
          }
        } else if(heishaOTDataStruct[x].type == TFLOAT) {
          switch(obj->varstack.buffer[val]) {
            case VINTEGER: {
              struct vm_vinteger_t *na = (struct vm_vinteger_t *)&obj->varstack.buffer[val];

              heishaOTDataStruct[x].value.f = (float)na->value;
            } break;
            case VFLOAT: {
              struct vm_vfloat_t *na = (struct vm_vfloat_t *)&obj->varstack.buffer[val];

              heishaOTDataStruct[x].value.f = na->value;
            } break;
          }
        }
        break;
      }
      x++;
    }
  }
}

static void vm_value_prt(struct rules_t *obj, char *out, int size) {
  struct varstack_t *varstack = (struct varstack_t *)obj->userdata;
  int x = 0, pos = 0;

  for(x=4;alignedbytes(x)<varstack->nrbytes;x++) {
    if(alignedbytes(x) < varstack->nrbytes) {
      x = alignedbytes(x);
      switch(varstack->stack[x]) {
        case VINTEGER: {
          struct vm_vinteger_t *val = (struct vm_vinteger_t *)&varstack->stack[x];
          switch(obj->ast.buffer[val->ret]) {
            case TVAR: {
              struct vm_tvar_t *node = (struct vm_tvar_t *)&obj->ast.buffer[val->ret];
              pos += snprintf_P(&out[pos], size - pos, PSTR("%s = %d\n"), node->token, val->value);
            } break;
            default: {
              return;
            } break;
          }
          x += sizeof(struct vm_vinteger_t)-1;
        } break;
        case VFLOAT: {
          struct vm_vfloat_t *val = (struct vm_vfloat_t *)&varstack->stack[x];
          switch(obj->ast.buffer[val->ret]) {
            case TVAR: {
              struct vm_tvar_t *node = (struct vm_tvar_t *)&obj->ast.buffer[val->ret];
              pos += snprintf_P(&out[pos], size - pos, PSTR("%s = %g\n"), node->token, val->value);
            } break;
            default: {
              return;
            } break;
          }
          x += sizeof(struct vm_vfloat_t)-1;
        } break;
        case VNULL: {
          struct vm_vnull_t *val = (struct vm_vnull_t *)&varstack->stack[x];
          switch(obj->ast.buffer[val->ret]) {
            case TVAR: {
              struct vm_tvar_t *node = (struct vm_tvar_t *)&obj->ast.buffer[val->ret];
              pos += snprintf_P(&out[pos], size - pos, PSTR("%s = NULL\n"), node->token);
            } break;
            default: {
              return;
            } break;
          }
          x += sizeof(struct vm_vnull_t)-1;
        } break;
        default: {
          return;
        } break;
      }
    }
  }
}

static void vm_global_value_prt(char *out, int size) {
  struct varstack_t *varstack = &global_varstack;
  int x = 0, pos = 0;

  for(x=4;alignedbytes(x)<varstack->nrbytes;x++) {
    x = alignedbytes(x);
    switch(varstack->stack[x]) {
      case VINTEGER: {
        struct vm_gvinteger_t *val = (struct vm_gvinteger_t *)&varstack->stack[x];
        switch(rules[val->rule-1]->ast.buffer[val->ret]) {
          case TVAR: {
            struct vm_tvar_t *node = (struct vm_tvar_t *)&rules[val->rule-1]->ast.buffer[val->ret];
            pos += snprintf_P(&out[pos], size - pos, PSTR("%d %s = %d\n"), x, node->token, val->value);
          } break;
          default: {
            return;
          } break;
        }
        x += sizeof(struct vm_gvinteger_t)-1;
      } break;
      case VFLOAT: {
        struct vm_gvfloat_t *val = (struct vm_gvfloat_t *)&varstack->stack[x];
        switch(rules[val->rule-1]->ast.buffer[val->ret]) {
          case TVAR: {
            struct vm_tvar_t *node = (struct vm_tvar_t *)&rules[val->rule-1]->ast.buffer[val->ret];
            pos += snprintf_P(&out[pos], size - pos, PSTR("%d %s = %g\n"), x, node->token, val->value);
          } break;
          default: {
            return;
          } break;
        }
        x += sizeof(struct vm_gvfloat_t)-1;
      } break;
      case VNULL: {
        struct vm_gvnull_t *val = (struct vm_gvnull_t *)&varstack->stack[x];
        switch(rules[val->rule-1]->ast.buffer[val->ret]) {
          case TVAR: {
            struct vm_tvar_t *node = (struct vm_tvar_t *)&rules[val->rule-1]->ast.buffer[val->ret];
            pos += snprintf_P(&out[pos], size - pos, PSTR("%d %s = NULL\n"), x, node->token);
          } break;
          default: {
            return;
          } break;
        }
        x += sizeof(struct vm_gvnull_t)-1;
      } break;
      default: {
        return;
      } break;
    }
  }
}

static void vm_clear_values(struct rules_t *obj) {
  int i = 0, x = 0;
  for(i=x;alignedbytes(i)<obj->ast.nrbytes;i++) {
    i = alignedbytes(i);
    switch(obj->ast.buffer[i]) {
      case TSTART: {
        i+=sizeof(struct vm_tstart_t)-1;
      } break;
      case TEOF: {
        i+=sizeof(struct vm_teof_t)-1;
      } break;
      case VNULL: {
        i+=sizeof(struct vm_vnull_t)-1;
      } break;
      case TIF: {
        i+=sizeof(struct vm_tif_t)-1;
      } break;
      case LPAREN: {
        struct vm_lparen_t *node = (struct vm_lparen_t *)&obj->ast.buffer[i];
        node->value = 0;
        i+=sizeof(struct vm_lparen_t)-1;
      } break;
      case TFALSE:
      case TTRUE: {
        struct vm_ttrue_t *node = (struct vm_ttrue_t *)&obj->ast.buffer[i];
        i+=sizeof(struct vm_ttrue_t)+(sizeof(node->go[0])*node->nrgo)-1;
      } break;
      case TFUNCTION: {
        struct vm_tfunction_t *node = (struct vm_tfunction_t *)&obj->ast.buffer[i];
        node->value = 0;
        i+=sizeof(struct vm_tfunction_t)+(sizeof(node->go[0])*node->nrgo)-1;
      } break;
      case TCEVENT: {
        struct vm_tcevent_t *node = (struct vm_tcevent_t *)&obj->ast.buffer[i];
        i+=sizeof(struct vm_tcevent_t)+strlen((char *)node->token);
      } break;
      case TVAR: {
        struct vm_tvar_t *node = (struct vm_tvar_t *)&obj->ast.buffer[i];
        node->value = 0;
        i+=sizeof(struct vm_tvar_t)+strlen((char *)node->token);
      } break;
      case TEVENT: {
        struct vm_tevent_t *node = (struct vm_tevent_t *)&obj->ast.buffer[i];
        i += sizeof(struct vm_tevent_t)+strlen((char *)node->token);
      } break;
      case TNUMBER: {
        struct vm_tnumber_t *node = (struct vm_tnumber_t *)&obj->ast.buffer[i];
        i+=sizeof(struct vm_tnumber_t)+strlen((char *)node->token);
      } break;
      case VINTEGER: {
        struct vm_vinteger_t *node = (struct vm_vinteger_t *)&obj->ast.buffer[i];
        i+=sizeof(struct vm_vinteger_t)-1;
      } break;
      case VFLOAT: {
        struct vm_vfloat_t *node = (struct vm_vfloat_t *)&obj->ast.buffer[i];
        i+=sizeof(struct vm_vfloat_t)-1;
      } break;
      case TOPERATOR: {
        struct vm_toperator_t *node = (struct vm_toperator_t *)&obj->ast.buffer[i];
        node->value = 0;
        i+=sizeof(struct vm_toperator_t)-1;
      } break;
      default: {
      } break;
    }
  }
}

void rules_timer_cb(int nr) {
  char *name = NULL;
  int x = 0, i = 0;

  i = snprintf_P(NULL, 0, PSTR("timer=%d"), nr);
  if((name = (char *)MALLOC(i+2)) == NULL) {
    OUT_OF_MEMORY
  }
  memset(name, 0, i+2);
  snprintf_P(name, i+1, PSTR("timer=%d"), nr);

  // logprintf_P(F("_______ %s %s"), __FUNCTION__, name);

  for(x=0;x<nrrules;x++) {
    if(get_event(rules[x]) > -1 && stricmp((char *)&rules[x]->ast.buffer[get_event(rules[x])+5], name) == 0) {
      rule_run(rules[x], 0);

      char out[512];
      memset(&out, 0, 512);
      logprintln_P(F("\n>>> local variables\n"));
      vm_value_prt(rules[x], (char *)&out, 512);
      logprintf_P(F("%s"), out);
      logprintf_P(F("\n>>> global variables\n"));
      memset(&out, 0, 512);
      vm_global_value_prt((char *)&out, 512);
      logprintf_P(F("%s"), out);

      break;
    }
  }
  FREE(name);
}

int rules_parse(char *file) {
  File frules = LittleFS.open(file, "r");
  if(frules) {
    parsing = 1;

    if(nrrules > 0) {
      for(int i=0;i<nrrules;i++) {
        if(rules[i]->userdata != NULL) {
          FREE(rules[i]->userdata);
        }
      }
      rules_gc(&rules, nrrules);
      nrrules = 0;
    }
    memset(mempool, 0, MEMPOOL_SIZE);

    FREE(global_varstack.stack);
    global_varstack.stack = NULL;
    global_varstack.nrbytes = 4;

#define BUFFER_SIZE 128
    char content[BUFFER_SIZE];
    memset(content, 0, BUFFER_SIZE);
    int len = frules.size();
    int chunk = 0, len1 = 0;

    unsigned int txtoffset = alignedbuffer(MEMPOOL_SIZE-len-5);

    while(1) {
      memset(content, 0, BUFFER_SIZE);
      frules.seek(chunk*BUFFER_SIZE, SeekSet);
      if (chunk * BUFFER_SIZE <= len) {
        frules.readBytes(content, BUFFER_SIZE);
        len1 = BUFFER_SIZE;
      } else if ((chunk * BUFFER_SIZE) >= len && (chunk * BUFFER_SIZE) <= len + BUFFER_SIZE) {
        frules.readBytes(content, len - ((chunk - 1)*BUFFER_SIZE));
        len1 = len - ((chunk - 1) * BUFFER_SIZE);
      } else {
        break;
      }
      memcpy(&mempool[txtoffset+(chunk*BUFFER_SIZE)], &content, alignedbuffer(len1));
      chunk++;
    }
    frules.close();

    struct varstack_t *varstack = (struct varstack_t *)MALLOC(sizeof(struct varstack_t));
    if(varstack == NULL) {
      OUT_OF_MEMORY
    }
    varstack->stack = NULL;
    varstack->nrbytes = 4;
    varstack->bufsize = 4;

    struct pbuf mem;
    struct pbuf input;
    memset(&mem, 0, sizeof(struct pbuf));
    memset(&input, 0, sizeof(struct pbuf));

    mem.payload = mempool;
    mem.len = 0;
    mem.tot_len = MEMPOOL_SIZE;

    input.payload = &mempool[txtoffset];
    input.len = txtoffset;
    input.tot_len = len;

    int ret = 0;
    char *text = (char *)&mempool[txtoffset];
    while((ret = rule_initialize(&input, &rules, &nrrules, &mem, varstack)) == 0) {
      varstack = (struct varstack_t *)MALLOC(sizeof(struct varstack_t));
      if(varstack == NULL) {
        OUT_OF_MEMORY
      }
      varstack->stack = NULL;
      varstack->nrbytes = 4;
      varstack->bufsize = 4;
      input.payload = &mempool[input.len];
    }

    logprintf_P(F("rules memory used: %d / %d"), mem.len, mem.tot_len);

    if(nrrules > 1) {
      FREE(varstack);
    }

    /*
     * Clear all timers
     */
    struct timerqueue_t *node = NULL;
    while((node = timerqueue_pop()) != NULL) {
      FREE(node);
    }

    FREE(global_varstack.stack);
    global_varstack.stack = NULL;
    global_varstack.nrbytes = 4;

    if(ret == -1) {
      if(nrrules > 0) {
        for(int i=0;i<nrrules-1;i++) {
          if(rules[i]->userdata != NULL) {
            FREE(rules[i]->userdata);
          }
        }
        rules_gc(&rules, nrrules);
      }
      nrrules = 0;
      return -1;
    }

    int i = 0;
    for(i=0;i<nrrules;i++) {
      vm_clear_values(rules[i]);
    }
    parsing = 0;
    return 0;
  } else {
    return -1;
  }
}

void rules_event_cb(const char *prefix, const char *name) {
  uint8_t i = 0, len = strlen(name), len1 = strlen(prefix), tlen = 0;
  for(i=0;i<nrrules;i++) {
    struct vm_tstart_t *start = (struct vm_tstart_t *)&rules[i]->ast.buffer[0];
    if(rules[i]->ast.buffer[start->go] == TEVENT) {
      struct vm_tevent_t *event = (struct vm_tevent_t *)&rules[i]->ast.buffer[start->go];
      tlen = strlen((char *)event->token);
      if(
          (
            len+len1 == tlen &&
            strncmp((char *)event->token, prefix, len1) == 0 &&
            strnicmp((char *)&event->token[len1], name, len) == 0
          )
        ) {
        char out[512];
        logprintf_P(F("%s %s %s"), F("===="), event->token, F("===="));
        logprintf_P(F("%s %d %s %d"), F(">>> rule"), i, F("nrbytes:"), rules[i]->ast.nrbytes);
        logprintf_P(F("%s %d"), F(">>> global stack nrbytes:"), global_varstack.nrbytes);

        rules[i]->timestamp.first = micros();

        rule_run(rules[i], 0);

        rules[i]->timestamp.second = micros();

        logprintf_P(F("%s%d %s %d %s"), F("rule #"), rules[i]->nr, F("was executed in"), rules[i]->timestamp.second - rules[i]->timestamp.first, F("microseconds"));

        logprintln_P(F("\n>>> local variables"));
        memset(&out, 0, sizeof(out));
        vm_value_prt(rules[i], (char *)&out, sizeof(out));
        logprintln(out);
        logprintln_P(F(">>> global variables"));
        memset(&out, 0, sizeof(out));
        vm_global_value_prt((char *)&out, sizeof(out));
        logprintln(out);
        break;
      }
    }
  }
  //for(int i=0;i<nrrules;i++) {
  //  FREE(rules[i]->varstack.buffer);
  //  rules[i]->varstack.nrbytes = 4;
  //  rules[i]->varstack.bufsize = 4;
  //}
  int x = 0;
  for(i=0;i<nrrules;i++) {
    x += rules[i]->ast.nrbytes;
  }
}

void rules_boot(void) {
  for(uint8_t i=0;i<nrrules;i++) {
    struct vm_tstart_t *start = (struct vm_tstart_t *)&rules[i]->ast.buffer[0];
    if(rules[i]->ast.buffer[start->go] == TEVENT) {
      struct vm_tevent_t *event = (struct vm_tevent_t *)&rules[i]->ast.buffer[start->go];
      if(stricmp((char *)&event->token, "System#Boot") == 0) {
        char out[512];
        logprintf_P(F("==== SYSTEM#BOOT ===="));
        logprintf_P(F("%s %d %s %d"), F(">>> rule"), i, F("nrbytes:"), rules[i]->ast.nrbytes);
        logprintf_P(F("%s %d"), F(">>> global stack nrbytes:"), global_varstack.nrbytes);

        rules[i]->timestamp.first = micros();

        rule_run(rules[i], 0);

        rules[i]->timestamp.second = micros();

        logprintf_P(F("%s%d %s %d %s"), F("rule #"), rules[i]->nr, F("was executed in"), rules[i]->timestamp.second - rules[i]->timestamp.first, F("microseconds"));

        logprintln_P(F("\n>>> local variables"));
        memset(&out, 0, sizeof(out));
        vm_value_prt(rules[i], (char *)&out, sizeof(out));
        logprintln(out);
        logprintln_P(F(">>> global variables"));
        memset(&out, 0, sizeof(out));
        vm_global_value_prt((char *)&out, sizeof(out));
        logprintln(out);
        break;
      }
    }
  }

  // unsigned long a = micros();
  // for(int i=0;i<nrrules;i++) {
    // FREE(rules[i]->varstack.buffer);
    // rules[i]->varstack.nrbytes = 4;
    // rules[i]->varstack.bufsize = 4;
  // }
}

void rules_setup(void) {
  if(!LittleFS.begin()) {
    return;
  }
  memset(mempool, 0, MEMPOOL_SIZE);

  logprintf_P(F("rules mempool size: %d"), MEMPOOL_SIZE);

  logprintln_P(F("reading rules"));

  global_varstack.stack = NULL;
  global_varstack.nrbytes = 4;

  memset(&rule_options, 0, sizeof(struct rule_options_t));
  rule_options.is_token_cb = is_variable;
  rule_options.is_event_cb = is_event;
  rule_options.set_token_val_cb = vm_value_set;
  rule_options.get_token_val_cb = vm_value_get;
  rule_options.prt_token_val_cb = vm_value_prt;
  rule_options.cpy_token_val_cb = vm_value_cpy;
  rule_options.clr_token_val_cb = vm_value_clr;
  rule_options.event_cb = event_cb;

  // if(LittleFS.exists("/rules.new")) {
    // logprintln_P(F("new ruleset found, trying to parse it"));
    // if(rules_parse("/rules.new") == -1) {
      // logprintln_P(F("new ruleset failed to parse, using previous ruleset after restart"));
      // LittleFS.remove("/rules.new");
      // ESP.restart();
    // } else {
      // LittleFS.rename("/rules.new", "/rules.txt");
    // }
  // } else if(LittleFS.exists("/rules.txt")) {
    // if(rules_parse("/rules.txt") == -1) {
      // logprintln_P(F("old ruleset failed to parse, restarting without rules"));
      // LittleFS.rename("/rules.txt", "/rules.old");
      // ESP.restart();
    // }
  // } else if(LittleFS.exists("/rules.old")) {
    // LittleFS.rename("/rules.old", "/rules.txt");
  // }

  if(LittleFS.exists("/rules.txt")) {
    rules_parse("/rules.txt");
  }

  rules_boot();
}
