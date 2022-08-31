/*
  Copyright (C) CurlyMo

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifdef ESP8266
  #pragma GCC diagnostic warning "-fpermissive"
#endif

#ifndef ESP8266
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdarg.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <limits.h>
  #include <errno.h>
  #include <time.h>
  #include <sys/time.h>
  #include <math.h>
  #include <string.h>
  #include <ctype.h>
  #include <stdint.h>
#else
  #include <Arduino.h>
#endif
#include <assert.h>

#include "../common/mem.h"
#include "../common/log.h"
#include "../common/strnicmp.h"
#include "../common/mem.h"
#include "../common/log.h"
#include "rules.h"
#include "operator.h"
#include "function.h"

struct vm_cache_t {
  uint8_t type;
  uint16_t step;
  uint16_t start;
  uint16_t end;
} __attribute__((packed)) **vmcache;
static unsigned int nrcache = 0;

/*LCOV_EXCL_START*/
#ifdef DEBUG
static void print_tree(struct rules_t *obj);
static void print_bytecode(struct rules_t *obj);
#endif
/*LCOV_EXCL_STOP*/

unsigned int alignedvarstack(int v) {
#ifdef ESP8266
  return (v + MAX_VARSTACK_NODE_SIZE) - ((v + MAX_VARSTACK_NODE_SIZE) % MAX_VARSTACK_NODE_SIZE);
#else
  return v;
#endif
}

void rules_gc(struct rules_t ***rules, unsigned int nrrules) {
  unsigned int i = 0;
  for(i=0;i<nrrules;i++) {
    // FREE((*rules)[i]->ast.buffer);
    // FREE((*rules)[i]->varstack.buffer);
    // FREE((*rules)[i]);
  }
  // FREE((*rules));
  // (*rules) = NULL;

#ifndef ESP8266
  /*
   * Should never happen
   */
  /* LCOV_EXCL_START*/
  for(i=0;i<nrcache;i++) {
    FREE(vmcache[i]);
    vmcache[i] = NULL;
  }
  /* LCOV_EXCL_STOP*/
  FREE(vmcache);
  vmcache = NULL;
  nrcache = 0;
#endif
}

static int is_function(char *text, unsigned int *pos, unsigned int size) {
  unsigned int i = 0, len = 0;
  for(i=0;i<nr_rule_functions;i++) {
    len = strlen(rule_functions[i].name);
    if(size == len && strnicmp(&text[*pos], rule_functions[i].name, len) == 0) {
      return i;
    }
  }

  return -1;
}

static int is_operator(char *text, unsigned int *pos, unsigned int size) {
  unsigned int i = 0, len = 0;
  for(i=0;i<nr_rule_operators;i++) {
    len = strlen(rule_operators[i].name);
    if(size == len && strnicmp(&text[*pos], rule_operators[i].name, len) == 0) {
      return i;
    }
  }

  return -1;
}

static int lexer_parse_number(char *text, int *pos) {

  int i = 0, nrdot = 0, len = strlen(text);
  if(isdigit(text[*pos]) || text[*pos] == '-') {
    /*
     * The dot cannot be the first character
     * and we cannot have more than 1 dot
     */
    while(*pos <= len &&
        (
          isdigit(text[*pos]) ||
          (i == 0 && text[*pos] == '-') ||
          (i > 0 && nrdot == 0 && text[*pos] == '.')
        )
      ) {
      if(text[*pos] == '.') {
        nrdot++;
      }
      (*pos)++;
      i++;
    }
    return 0;
  } else {
    return -1;
  }
}

static int lexer_parse_string(char *text, unsigned int *pos) {
  unsigned int len = strlen(&text[*pos]) + *pos;
  while(*pos <= len &&
        text[*pos] != ' ' &&
        text[*pos] != ',' &&
        text[*pos] != ';' &&
        text[*pos] != '(' &&
        text[*pos] != ')') {
    (*pos)++;
  }
  return 0;
}

// static int lexer_parse_quoted_string(struct rules_t *obj, struct lexer_t *lexer, int *start) {

  // int x = lexer->current_char[0];
  // if(lexer->current_char[0] == '\'' || lexer->current_char[0] == '"') {
    // lexer->text[lexer->pos] = '\0';
  // }
  // *start = ++lexer->pos;
  // if(lexer->pos < lexer->len) {

    // while(lexer->pos < lexer->len) {
      // if(x != '\'' && lexer->pos > 0 && lexer->text[lexer->pos] == '\'') {
        // /*
         // * Skip escape
         // */
        // lexer->current_char = &lexer->text[lexer->(*pos)++];
      // } else if(x == '\'' && lexer->pos > 0 && lexer->text[lexer->pos] == '\\' && lexer->text[lexer->pos+1] == '\'') {
        // /*
         // * Remove escape
         // */
        // memmove(&lexer->text[lexer->pos], &lexer->text[lexer->pos+1], lexer->len-lexer->pos-1);
        // lexer->text[lexer->len-1] = '\0';

        // lexer->current_char = &lexer->text[lexer->(*pos)++];
        // lexer->len--;
      // } else if(x == '"' && lexer->pos > 0 && lexer->text[lexer->pos] == '\\' && lexer->text[lexer->pos+1] == '"') {
        // /*
         // * Remove escape
         // */
        // memmove(&lexer->text[lexer->pos], &lexer->text[lexer->pos+1], lexer->len-lexer->pos-1);
        // lexer->text[lexer->len-1] = '\0';

        // lexer->current_char = &lexer->text[lexer->(*pos)++];
        // lexer->len--;
      // } else if(x != '"' && lexer->pos > 0 && lexer->text[lexer->pos] == '"') {
        // /*
         // * Skip escape
         // */
        // lexer->current_char = &lexer->text[lexer->(*pos)++];
      // } else if(lexer->text[lexer->pos] == x) {
        // break;
      // } else {
        // lexer->(*pos)++;
      // }
    // }

    // if(lexer->current_char[0] != '"' && lexer->current_char[0] != '\'') {
      // printf("err: %s %d\n", __FUNCTION__, __LINE__);
      // exit(-1);
    // } else if(lexer->pos <= lexer->len) {
      // lexer_isolate_token(obj, lexer, x, -1);
      // return 0;
    // } else {
      // return -1;
    // }
  // }
  // return -1;
// }

static int lexer_parse_skip_characters(char *text, unsigned int len, unsigned int *pos) {
  while(*pos <= len &&
      (text[*pos] == ' ' ||
      text[*pos] == '\n' ||
      text[*pos] == '\t' ||
      text[*pos] == '\r')) {
    (*pos)++;
  }
  return 0;
}

static int rule_prepare(char **text, unsigned int *nrbytes, unsigned int *len) {
  unsigned int pos = 0, nrblocks = 0, tpos = 0;

  *nrbytes = alignedbytes(sizeof(struct vm_tstart_t));
#ifdef DEBUG
  printf("TSTART: %lu\n", sizeof(struct vm_tstart_t));
#endif

  while(pos < *len) {
    lexer_parse_skip_characters(*text, *len, &pos);

    // if(lexer->current_char[0] == '\'' || lexer->current_char[0] == '"') {
      // ret = lexer_parse_quoted_string(obj, lexer, &tpos);
      // type = TSTRING;
      // lexer->(*pos)++;
      // if(lexer->text[lexer->pos] == ' ') {
        // lexer->text[lexer->pos] = '\0';
      // } else {
        // printf("err: %s %d\n", __FUNCTION__, __LINE__);
        // exit(-1);
      // }
    // } else

    if(isdigit((*text)[pos]) || ((*text)[pos] == '-' && pos < *len && isdigit((*text)[(pos)+1]))) {
      int newlen = 0;
      lexer_parse_number(&(*text)[pos], &newlen);

      char tmp = (*text)[pos+newlen];
      (*text)[pos+newlen] = 0;

      float var = atof((char *)&(*text)[pos]);
      float nr = 0;

      if(modff(var, &nr) == 0) {
        /*
         * This range of integers
         * take less bytes when stored
         * as ascii characters.
         */
        if(var < 100 && var > -9) {
#ifdef DEBUG
          printf("TNUMBER: %lu\n", sizeof(struct vm_tnumber_t)+newlen+1);
#endif
          *nrbytes += alignedbytes(sizeof(struct vm_tnumber_t)+newlen+1);
        } else {
#ifdef DEBUG
          printf("TNUMBER: %lu\n", sizeof(struct vm_vinteger_t));
#endif
          *nrbytes += alignedbytes(sizeof(struct vm_vinteger_t));
        }
      } else {
#ifdef DEBUG
        printf("TNUMBER: %lu\n", sizeof(struct vm_vfloat_t));
#endif
        *nrbytes += alignedbytes(sizeof(struct vm_vfloat_t));
      }

      if(newlen < 4) {
        switch(newlen) {
          case 1: {
            // printf("TNUMBER1: %d\n", tpos);
            (*text)[tpos++] = TNUMBER1;
          } break;
          case 2: {
            // printf("TNUMBER2: %d\n", tpos);
            (*text)[tpos++] = TNUMBER2;
          } break;
          case 3: {
            // printf("TNUMBER3: %d\n", tpos);
            (*text)[tpos++] = TNUMBER3;
          } break;
        }
        memcpy(&(*text)[tpos], &(*text)[pos], newlen);
        tpos += newlen;
      } else {

        if(modff(var, &nr) == 0) {
          /*
           * This range of integers
           * take less bytes when stored
           * as ascii characters.
           */
          // printf("VINTEGER: %d\n", tpos);
          (*text)[tpos++] = VINTEGER;
          int x = var;
          memcpy(&(*text)[tpos], &x, sizeof(int));
          tpos += sizeof(int);
        } else {
          // printf("VFLOAT: %d\n", tpos);
          (*text)[tpos++] = VFLOAT;
          memcpy(&(*text)[tpos], &var, sizeof(float));
          tpos += sizeof(float);
        }
      }

      if(tmp == ' ' || tmp == '\n' || tmp != '\t' || tmp != '\r') {
        (*text)[pos+newlen] = tmp;
        pos += newlen;
      } else {
        int x = 0;
        while(tmp != ' ' && tmp != '\n'  && tmp != '\t' && tmp != '\r') {
          char tmp1 = (*text)[pos+newlen+1];
          (*text)[pos+newlen+1] = tmp;
          tmp = tmp1;
          newlen += 1;
          x++;
        }

        if((*text)[pos+newlen] == ' ' || (*text)[pos+newlen] == '\t' ||
           (*text)[pos+newlen] == '\r' || (*text)[pos+newlen] == '\n') {
          (*text)[pos+newlen] = tmp;
        }
        pos += newlen-x+1;
      }


    } else if(strnicmp((char *)&(*text)[pos], "if", 2) == 0) {
      *nrbytes += alignedbytes(sizeof(struct vm_tif_t));
      *nrbytes += alignedbytes(sizeof(struct vm_ttrue_t));
#ifdef DEBUG
      printf("TIF: %lu\n", sizeof(struct vm_tif_t));
      printf("TTRUE: %lu\n", sizeof(struct vm_ttrue_t));
#endif

      /*
       * An additional TTRUE slot
       */
      *nrbytes += alignedbytes(sizeof(uint16_t));
#ifdef DEBUG
      printf("TTRUE: %lu\n", sizeof(uint16_t));
#endif
      // printf("TIF: %d\n", tpos);
      (*text)[tpos++] = TIF;

      pos+=2;
      nrblocks++;
    } else if(strnicmp(&(*text)[pos], "on", 2) == 0) {
      pos+=2;
      lexer_parse_skip_characters((*text), *len, &pos);
      int s = pos;
      lexer_parse_string((*text), &pos);

      {
        unsigned int len = pos - s;
        *nrbytes += alignedbytes(sizeof(struct vm_tevent_t)+len+1);
        *nrbytes += alignedbytes(sizeof(struct vm_ttrue_t));
#ifdef DEBUG
        printf("TEVENT: %lu\n", sizeof(struct vm_tevent_t)+len+1);
        printf("TTRUE: %lu\n", sizeof(struct vm_ttrue_t));
#endif
        // printf("TEVENT: %d\n", tpos);
        (*text)[tpos++] = TEVENT;
        memcpy(&(*text)[tpos], &(*text)[s], len);
        tpos += len;

        /*
         * An additional TTRUE slot
         */
        *nrbytes += alignedbytes(sizeof(uint16_t));
#ifdef DEBUG
        printf("TTRUE: %lu\n", sizeof(uint16_t));
#endif
      }
      nrblocks++;
    } else if(strnicmp((char *)&(*text)[pos], "else", 4) == 0) {
      *nrbytes += alignedbytes(sizeof(struct vm_ttrue_t));
#ifdef DEBUG
      printf("TTRUE: %lu\n", sizeof(struct vm_ttrue_t));
#endif
      pos+=4;
      (*text)[tpos++] = TELSE;
    } else if(strnicmp((char *)&(*text)[pos], "then", 4) == 0) {
      pos+=4;
      // printf("TTHEN: %d\n", tpos);
      (*text)[tpos++] = TTHEN;
    } else if(strnicmp((char *)&(*text)[pos], "end", 3) == 0) {
      pos+=3;
      nrblocks--;
      // printf("TEND: %d\n", tpos);
      (*text)[tpos++] = TEND;
    } else if(strnicmp((char *)&(*text)[pos], "NULL", 4) == 0) {
      *nrbytes += alignedbytes(sizeof(struct vm_vnull_t));
#ifdef DEBUG
      printf("VNULL: %lu\n", sizeof(struct vm_vnull_t));
#endif
      // printf("VNULL: %d\n", tpos);
      (*text)[tpos++] = VNULL;
      pos+=4;
    } else if((*text)[pos] == ',') {
      /*
       * An additional function argument slot
       */
      *nrbytes += alignedbytes(sizeof(uint16_t));
#ifdef DEBUG
      printf("TFUNCTION: %lu\n", sizeof(uint16_t));
#endif
      // printf("TCOMMA: %d\n", tpos);
      (*text)[tpos++] = TCOMMA;
      pos++;
    } else if((*text)[pos] == '(') {
      *nrbytes += alignedbytes(sizeof(struct vm_lparen_t));
#ifdef DEBUG
      printf("LPAREN: %lu\n", sizeof(struct vm_lparen_t));
#endif
      // printf("LPAREN: %d\n", tpos);
      (*text)[tpos++] = LPAREN;
      pos++;
    } else if((*text)[pos] == ')') {
      pos++;
      // printf("RPAREN: %d\n", tpos);
      (*text)[tpos++] = RPAREN;
    } else if((*text)[pos] == '=' && pos < *len && (*text)[(pos)+1] != '=') {
      pos++;
      // printf("TASSIGN: %d\n", tpos);
      (*text)[tpos++] = TASSIGN;
    } else if((*text)[pos] == ';') {
      /*
       * An additional TTRUE slot
       */
      *nrbytes += alignedbytes(sizeof(uint16_t));
#ifdef DEBUG
      printf("TTRUE: %lu\n", sizeof(uint16_t));
#endif
      pos++;
      // printf("TSEMICOLON: %d\n", tpos);
      (*text)[tpos++] = TSEMICOLON;
    } else {
      unsigned int a = 0, b = *len-(pos)-1;
      int len1 = 0;
      for(a=(pos);a<*len;a++) {
        if((*text)[a] == ' ' || (*text)[a] == '(' || (*text)[a] == ')' ||
           (*text)[a] == ',' || (*text)[a] == ';') {
          b = a-(pos);
          break;
        }
      }

      if((len1 = is_function((*text), &pos, b)) > -1) {
        *nrbytes += alignedbytes(sizeof(struct vm_tfunction_t)+sizeof(uint16_t));
        *nrbytes -= alignedbytes(sizeof(struct vm_lparen_t));
#ifdef DEBUG
        printf("TFUNCTION: %lu\n", sizeof(struct vm_tfunction_t)+sizeof(uint16_t));
#endif

        // printf("TFUNCTION: %d\n", tpos);
        (*text)[tpos++] = TFUNCTION;
        (*text)[tpos++] = len1;
        pos += b;
      } else if((len1 = is_operator((*text), &pos, b)) > -1) {
        *nrbytes += alignedbytes(sizeof(struct vm_toperator_t));
#ifdef DEBUG
        printf("TOPERATOR: %lu\n", sizeof(struct vm_toperator_t));
#endif
        pos += b;

        // printf("TOPERATOR: %d\n", tpos);
        (*text)[tpos++] = TOPERATOR;
        (*text)[tpos++] = len1;
      } else if(rule_options.is_token_cb != NULL && (len1 = rule_options.is_token_cb((*text), &pos, b)) > -1) {
        *nrbytes += alignedbytes(sizeof(struct vm_tvar_t)+len1+1);
#ifdef DEBUG
        printf("TVAR: %lu\n", sizeof(struct vm_tvar_t)+len1+1);
#endif

        // printf("TVAR: %d\n", tpos);
        (*text)[tpos++] = TVAR;
        memcpy(&(*text)[tpos], &(*text)[pos], len1);
        tpos += len1;
        pos += len1;
      } else if(rule_options.is_event_cb != NULL && (len1 = rule_options.is_event_cb((*text), &pos, b)) > -1) {
        if((pos)+b < *len && (*text)[(pos)+b] == '(' && (pos)+b+1 < *len && (*text)[(pos)+b+1] == ')') {
          lexer_parse_skip_characters((*text), *len, &pos);
          int s = pos;
          lexer_parse_string((*text), &pos);

          {
            unsigned int len = pos - s;

            *nrbytes += alignedbytes(sizeof(struct vm_tcevent_t)+len+1);
#ifdef DEBUG
            printf("TCEVENT: %lu\n", sizeof(struct vm_tcevent_t)+len+1);
#endif
            // printf("TCEVENT: %d\n", tpos);
            (*text)[tpos++] = TCEVENT;
            memcpy(&(*text)[tpos], &(*text)[s], len);
            tpos += len;
          }
          pos += 2;
        } else {
          if((*len - pos) > 5) {
            logprintf_P(F("ERROR: unknown token '%.5s...'"), &(*text)[pos]);
          } else {
            logprintf_P(F("ERROR: unknown token '%.5s'"), &(*text)[pos]);
          }
          return -1;
        }
      } else {
        if((*len - pos) > 5) {
          logprintf_P(F("ERROR: unknown token '%.5s...'"), &(*text)[pos]);
        } else {
          logprintf_P(F("ERROR: unknown token '%.5s'"), &(*text)[pos]);
        }
        return -1;
      }
    }

    if(nrblocks == 0) {
      /*
       * Remove one go slot because of the root
       * if or on block
       */
      *nrbytes -= alignedbytes(sizeof(uint16_t));
      *nrbytes += alignedbytes(sizeof(struct vm_teof_t));
#ifdef DEBUG
      printf("TEOF: %lu\n", sizeof(struct vm_teof_t));
#endif

      // printf("TEOF: %d\n", tpos);
      (*text)[tpos++] = TEOF;
      unsigned int oldpos = pos;
      lexer_parse_skip_characters((*text), *len, &pos);
      if(*len == pos) {
        return 0;
      }
      memmove(&(*text)[oldpos], &(*text)[pos-1], *len-(pos)+1);
      (*text)[oldpos] = 0;
      *len -= (pos-oldpos-1);
      (*text)[*len] = 0;
      *len = oldpos;
      return 0;
    }

    lexer_parse_skip_characters((*text), *len, &pos);
  }

  /*
   * Remove one go slot because of the root
   * if or on block
   */
  *nrbytes -= alignedbytes(sizeof(uint16_t));
  *nrbytes += alignedbytes(sizeof(struct vm_teof_t));
#ifdef DEBUG
  printf("TEOF: %lu\n", sizeof(struct vm_teof_t));
#endif
  // printf("TEOF: %d\n", tpos);
  (*text)[tpos++] = TEOF;
  return 0;
}

static int lexer_peek(char **text, int skip, int *type, int *start, int *len) {
  int nr = 0, loop = 1, i = 0;
  while(loop) {
    *type = (*text)[i];
    *start = i;
    *len = 0;
    switch((*text)[i]) {
      case VNULL:
      case TSEMICOLON:
      case TEND:
      case TASSIGN:
      case RPAREN:
      case TCOMMA:
      case LPAREN:
      case TELSE:
      case TTHEN:
      case TIF: {
        i += 1;
      } break;
      case TNUMBER1: {
        *len = 1;
        i += 2;
      } break;
      case TNUMBER2: {
        *len = 2;
        i += 3;
      } break;
      case TNUMBER3: {
        *len = 3;
        i += 4;
      } break;
      case VINTEGER: {
        i += 1+sizeof(int);
      } break;
      case VFLOAT: {
        i += 1+sizeof(float);
      } break;
      case TFUNCTION:
      case TOPERATOR: {
        i += 2;
      } break;
      case TEOF: {
        i += 1;
        loop = 0;
      } break;
      case TCEVENT:
      case TEVENT:
      case TVAR: {
        i++;
        while((*text)[i] > 32 && (*text)[i] < 126) {
          i++;
        }
        *len = i - *start - 1;
      } break;
      default: {
        logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
        return -1;
      } break;
    }
    if(skip == nr++) {
      return i;
    }
  }
  return -1;
}

static int vm_parent(char **text, struct rules_t *obj, int type, int start, int len, unsigned int opt) {
  unsigned int ret = alignedbytes(obj->ast.nrbytes), size = 0, i = 0;

  switch(type) {
    case TSTART: {
      size = alignedbytes(ret+sizeof(struct vm_tstart_t));
      assert(size <= obj->ast.bufsize);
      struct vm_tstart_t *node = (struct vm_tstart_t *)&obj->ast.buffer[ret];
      node->type = type;
      node->go = 0;
      node->ret = 0;

      obj->ast.nrbytes = size;
    } break;
    case TIF: {
      size = alignedbytes(ret+sizeof(struct vm_tif_t));
      assert(size <= obj->ast.bufsize);
      struct vm_tif_t *node = (struct vm_tif_t *)&obj->ast.buffer[ret];
      node->type = type;
      node->ret = 0;
      node->go = 0;
      node->true_ = 0;
      node->false_ = 0;

      obj->ast.nrbytes = size;
    } break;
    case LPAREN: {
      size = alignedbytes(ret+sizeof(struct vm_lparen_t));
      assert(size <= obj->ast.bufsize);
      struct vm_lparen_t *node = (struct vm_lparen_t *)&obj->ast.buffer[ret];
      node->type = type;
      node->ret = 0;
      node->go = 0;
      node->value = 0;

      obj->ast.nrbytes = size;
    } break;
    case TOPERATOR: {
      size = alignedbytes(ret+sizeof(struct vm_toperator_t));
      assert(size <= obj->ast.bufsize);
      struct vm_toperator_t *node = (struct vm_toperator_t *)&obj->ast.buffer[ret];
      node->type = type;
      node->ret = 0;
      node->token = (*text)[start+1];
      node->left = 0;
      node->right = 0;
      node->value = 0;

      obj->ast.nrbytes = size;
    } break;
    case VINTEGER: {
      size = alignedbytes(obj->ast.nrbytes+sizeof(struct vm_vinteger_t));
      assert(size <= obj->ast.bufsize);

      struct vm_vinteger_t *value = (struct vm_vinteger_t *)&obj->ast.buffer[obj->ast.nrbytes];
      value->type = VINTEGER;
      value->ret = ret;
      memcpy(&value->value, &(*text)[start+1], sizeof(int));
      obj->ast.nrbytes = size;
    } break;
    case VFLOAT: {
      size = alignedbytes(obj->ast.nrbytes+sizeof(struct vm_vfloat_t));
      assert(size <= obj->ast.bufsize);

      struct vm_vfloat_t *value = (struct vm_vfloat_t *)&obj->ast.buffer[obj->ast.nrbytes];
      value->type = VFLOAT;
      value->ret = ret;
      memcpy(&value->value, &(*text)[start+1], sizeof(float));
      obj->ast.nrbytes = size;
    } break;
    case TNUMBER1:
    case TNUMBER2:
    case TNUMBER3: {
      char tmp = (*text)[start+1+len];
      (*text)[start+1+len] = 0;
      float var = atof((char *)&(*text)[start+1]);
      float nr = 0;

      if(modff(var, &nr) == 0) {
        /*
         * This range of integers
         * take less bytes when stored
         * as ascii characters.
         */
        if(var < 100 && var > -9) {
          size = alignedbytes(ret+sizeof(struct vm_tnumber_t)+len+1);
          assert(size <= obj->ast.bufsize);
          struct vm_tnumber_t *node = (struct vm_tnumber_t *)&obj->ast.buffer[ret];
          node->type = TNUMBER;
          node->ret = 0;
          memcpy(node->token, &(*text)[start+1], len);

          obj->ast.nrbytes = size;
        } else {
          size = alignedbytes(obj->ast.nrbytes+sizeof(struct vm_vinteger_t));
          assert(size <= obj->ast.bufsize);
          struct vm_vinteger_t *value = (struct vm_vinteger_t *)&obj->ast.buffer[obj->ast.nrbytes];
          value->type = VINTEGER;
          value->ret = ret;
          value->value = (int)var;
          obj->ast.nrbytes = size;
        }
      } else {
        size = alignedbytes(obj->ast.nrbytes + sizeof(struct vm_vfloat_t));
        assert(size <= obj->ast.bufsize);
        struct vm_vfloat_t *value = (struct vm_vfloat_t *)&obj->ast.buffer[obj->ast.nrbytes];
        value->type = VFLOAT;
        value->ret = ret;
        value->value = var;
        obj->ast.nrbytes = size;
      }
      (*text)[start+1+len] = tmp;
    } break;
    case TFALSE:
    case TTRUE: {
      size = alignedbytes(ret+sizeof(struct vm_ttrue_t)+(sizeof(uint16_t)*opt));
      assert(size <= obj->ast.bufsize);
      struct vm_ttrue_t *node = (struct vm_ttrue_t *)&obj->ast.buffer[ret];
      node->type = type;
      node->ret = 0;
      node->nrgo = opt;
      for(i=0;i<opt;i++) {
        node->go[i] = 0;
      }

      obj->ast.nrbytes = size;
    } break;
    case TFUNCTION: {
      size = alignedbytes(ret+sizeof(struct vm_tfunction_t)+(sizeof(uint16_t)*opt));
      assert(size <= obj->ast.bufsize);
      struct vm_tfunction_t *node = (struct vm_tfunction_t *)&obj->ast.buffer[ret];
      node->token = (*text)[start+1];
      node->type = type;
      node->ret = 0;
      node->nrgo = opt;
      for(i=0;i<opt;i++) {
        node->go[i] = 0;
      }
      node->value = 0;

      obj->ast.nrbytes = size;
    } break;
    case VNULL: {
      size = alignedbytes(ret+sizeof(struct vm_vnull_t));
      assert(size <= obj->ast.bufsize);
      struct vm_vnull_t *node = (struct vm_vnull_t *)&obj->ast.buffer[ret];
      node->type = type;
      node->ret = 0;

      obj->ast.nrbytes = size;
    } break;
    case TVAR: {
      size = alignedbytes(ret+sizeof(struct vm_tvar_t)+len+1);
      assert(size <= obj->ast.bufsize);
      struct vm_tvar_t *node = (struct vm_tvar_t *)&obj->ast.buffer[ret];
      node->type = type;
      node->ret = 0;
      node->go = 0;
      node->value = 0;

      memcpy(node->token, &(*text)[start+1], len);

      obj->ast.nrbytes = size;
    } break;
    case TEVENT: {
      size = alignedbytes(ret+sizeof(struct vm_tevent_t)+len+1);
      assert(size <= obj->ast.bufsize);
      struct vm_tevent_t *node = (struct vm_tevent_t *)&obj->ast.buffer[ret];
      node->type = type;
      node->ret = 0;
      node->go = 0;

      memcpy(node->token, &(*text)[start+1], len);

      obj->ast.nrbytes = size;
    } break;
    case TCEVENT: {
      size = alignedbytes(ret+sizeof(struct vm_tcevent_t)+len+1);
      assert(size <= obj->ast.bufsize);
      struct vm_tcevent_t *node = (struct vm_tcevent_t *)&obj->ast.buffer[ret];

      node->type = type;
      node->ret = 0;
      /*
       * memcpy triggers a -Wstringop-overflow here
       */
      memcpy(node->token, &(*text)[start+1], len);

      obj->ast.nrbytes = size;
    } break;
    case TEOF: {
      size = alignedbytes(ret+sizeof(struct vm_teof_t));
      assert(size <= obj->ast.bufsize);
      struct vm_teof_t *node = (struct vm_teof_t *)&obj->ast.buffer[ret];
      node->type = type;

      obj->ast.nrbytes = size;
    } break;
    default: {
      return -1;
    } break;
  }

  return ret;
}

static int vm_rewind2(struct rules_t *obj, int step, int type, int type2) {
  int tmp = step;
  while(1) {
    if((obj->ast.buffer[tmp]) == type || (type2 > -1 && (obj->ast.buffer[tmp]) == type2)) {
      return tmp;
    } else {
      switch(obj->ast.buffer[tmp]) {
        case TSTART: {
          return 0;
        } break;
        case TIF: {
          struct vm_tif_t *node = (struct vm_tif_t *)&obj->ast.buffer[tmp];
          tmp = node->ret;
        } break;
        case TEVENT: {
          struct vm_tevent_t *node = (struct vm_tevent_t *)&obj->ast.buffer[tmp];
          tmp = node->ret;
        } break;
        case LPAREN: {
          struct vm_lparen_t *node = (struct vm_lparen_t *)&obj->ast.buffer[tmp];
          tmp = node->ret;
        } break;
        case TFALSE:
        case TTRUE: {
          struct vm_ttrue_t *node = (struct vm_ttrue_t *)&obj->ast.buffer[tmp];
          tmp = node->ret;
        } break;
        case TVAR: {
          struct vm_tvar_t *node = (struct vm_tvar_t *)&obj->ast.buffer[tmp];
          tmp = node->ret;
        } break;
        case TOPERATOR: {
          struct vm_toperator_t *node = (struct vm_toperator_t *)&obj->ast.buffer[tmp];
          tmp = node->ret;
        } break;
        /* LCOV_EXCL_START*/
        case TCEVENT:
        case TNUMBER1:
        case TNUMBER2:
        case TNUMBER3:
        case VINTEGER:
        case VFLOAT:
        case VNULL:
        default: {
          logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
          return -1;
        } break;
        /* LCOV_EXCL_STOP*/
      }
    }
  }
  return 0;
}

static int vm_rewind(struct rules_t *obj, int step, int type) {
  return vm_rewind2(obj, step, type, -1);
}

static void vm_cache_add(int type, int step, int start, int end) {
  if((vmcache = (struct vm_cache_t **)REALLOC(vmcache, sizeof(struct vm_cache_t *)*((nrcache)+1))) == NULL) {
    OUT_OF_MEMORY /*LCOV_EXCL_LINE*/
  }
  if((vmcache[nrcache] = (struct vm_cache_t *)MALLOC(sizeof(struct vm_cache_t))) == NULL) {
    OUT_OF_MEMORY /*LCOV_EXCL_LINE*/
  }
  vmcache[nrcache]->type = type;
  vmcache[nrcache]->step = step;
  vmcache[nrcache]->start = start;
  vmcache[nrcache]->end = end;

  nrcache++;
#ifdef DEBUG
  printf("cache entries: %d\n", nrcache); /*LCOV_EXCL_LINE*/
#endif
}

static void vm_cache_del(int start) {
  unsigned int x = 0, y = 0;
  for(x=0;x<nrcache;x++) {
    if(vmcache[x]->start == start) {
      /*LCOV_EXCL_START*/
      /*
       * The cache should be popped in a lifo manner.
       * Therefor, moving element after the current
       * one popped should never occur.
       */
      for(y=x;y<nrcache-1;y++) {
        memcpy(vmcache[y], vmcache[y+1], sizeof(struct vm_cache_t));
      }
      /*LCOV_EXCL_STOP*/
      FREE(vmcache[nrcache-1]);
      nrcache--;
      break;
    }
  }
  if(nrcache == 0) {
    FREE(vmcache);
    vmcache = NULL;
  } else {
    if((vmcache = (struct vm_cache_t **)REALLOC(vmcache, sizeof(struct vm_cache_t *)*nrcache)) == NULL) {
      OUT_OF_MEMORY /*LCOV_EXCL_LINE*/
    }
  }
#ifdef DEBUG
  printf("cache entries: %d\n", nrcache); /*LCOV_EXCL_LINE*/
#endif
}

static struct vm_cache_t *vm_cache_get(int type, int start) {
  unsigned int x = 0;
  for(x=0;x<nrcache;x++) {
    if(vmcache[x]->type == type && vmcache[x]->start == start) {
      return vmcache[x];
    }
  }
  return NULL;
}

static int lexer_parse_math_order(char **text, int *length, struct rules_t *obj, int type, int *pos, int *step_out, int offset, int source) {
  int b = 0, c = 0, step = 0, first = 1, start = 0, len = 0;

  while(1) {
    int right = 0;

    if(lexer_peek(text, (*pos), &b, &start, &len) < 0 || b != TOPERATOR) {
      break;
    }
    step = vm_parent(text, obj, b, start, len, 0);
    (*pos)++;

    if(lexer_peek(text, (*pos), &c, &start, &len) >= 0) {
      switch(c) {
        case LPAREN: {
          int oldpos = (*pos);
          struct vm_cache_t *x = vm_cache_get(LPAREN, (*pos));
          /* LCOV_EXCL_START*/
          if(x == NULL) {
            logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
            return -1;
          }
          /* LCOV_EXCL_STOP*/
          (*pos) = x->end;
          right = x->step;
          vm_cache_del(oldpos);
        } break;
        case TFUNCTION: {
          int oldpos = (*pos);
          struct vm_cache_t *x = vm_cache_get(TFUNCTION, (*pos));
          /* LCOV_EXCL_START*/
          if(x == NULL) {
            logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
            return -1;
          }
          /* LCOV_EXCL_STOP*/
          (*pos) = x->end;
          right = x->step;
          vm_cache_del(oldpos);
        } break;
        case TVAR:
        case VNULL:
        case TNUMBER1:
        case TNUMBER2:
        case TNUMBER3:
        case VINTEGER:
        case VFLOAT: {
          right = vm_parent(text, obj, c, start, len, 0);
          (*pos)++;
        } break;
        default: {
          logprintf_P(F("ERROR: Expected a parenthesis block, function, number or variable"));
          return -1;
        } break;
      }
    }

    struct vm_tgeneric_t *node = (struct vm_tgeneric_t *)&obj->ast.buffer[right];
    node->ret = step;

    struct vm_toperator_t *op2 = (struct vm_toperator_t *)&obj->ast.buffer[step];
    op2->left = *step_out;
    op2->right = right;

    switch(obj->ast.buffer[*step_out]) {
      case TNUMBER:
      case TNUMBER1:
      case TNUMBER2:
      case TNUMBER3: {
        struct vm_tnumber_t *node = (struct vm_tnumber_t *)&obj->ast.buffer[*step_out];
        node->ret = step;
      } break;
      case VINTEGER: {
        struct vm_vinteger_t *node = (struct vm_vinteger_t *)&obj->ast.buffer[*step_out];
        node->ret = step;
      } break;
      case VFLOAT: {
        struct vm_vfloat_t *node = (struct vm_vfloat_t *)&obj->ast.buffer[*step_out];
        node->ret = step;
      } break;
      case LPAREN: {
        struct vm_lparen_t *node = (struct vm_lparen_t *)&obj->ast.buffer[*step_out];
        node->ret = step;
      } break;
      case TOPERATOR: {
        struct vm_toperator_t *node = (struct vm_toperator_t *)&obj->ast.buffer[*step_out];
        node->ret = step;
      } break;
      case TVAR: {
        struct vm_tvar_t *node = (struct vm_tvar_t *)&obj->ast.buffer[*step_out];
        node->ret = step;
      } break;
      case TFUNCTION: {
        struct vm_tfunction_t *node = (struct vm_tfunction_t *)&obj->ast.buffer[*step_out];
        node->ret = step;
      } break;
      case VNULL: {
        struct vm_vnull_t *node = (struct vm_vnull_t *)&obj->ast.buffer[*step_out];
        node->ret = step;
      } break;
      /* LCOV_EXCL_START*/
      default: {
        logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
        return -1;
      } break;
      /* LCOV_EXCL_STOP*/
    }

    struct vm_toperator_t *op1 = NULL;

    if(type == LPAREN) {
      if(first == 1/* && *step_out > obj->pos.parsed*/) {
        struct vm_tgeneric_t *node = (struct vm_tgeneric_t *)&obj->ast.buffer[*step_out];
        node->ret = step;
      }
    } else if(type == TOPERATOR) {
      if(first == 1) {
        struct vm_tgeneric_t *node = (struct vm_tgeneric_t *)&obj->ast.buffer[step];
        node->ret = source;

        op2->ret = step;
        if((obj->ast.buffer[source]) == TIF) {
          struct vm_tif_t *node = (struct vm_tif_t *)&obj->ast.buffer[source];
          node->go = step;
        }
      }
    }

    if((obj->ast.buffer[step]) == TOPERATOR && (obj->ast.buffer[*step_out]) == TOPERATOR) {
      struct vm_toperator_t *op3 = NULL;
      op1 = (struct vm_toperator_t *)&obj->ast.buffer[*step_out];

      int idx1 = op1->token;
      int idx2 = op2->token;

      int x = rule_operators[idx1].precedence;
      int y = rule_operators[idx2].precedence;
      int a = rule_operators[idx2].associativity;

      if(y > x || (x == y && a == 2)) {
        if(a == 1) {
          op2->left = op1->right;
          op3 = (struct vm_toperator_t *)&obj->ast.buffer[op2->left];
          op3->ret = step;
          op1->right = step;
          op2->ret = *step_out;
        } else {
          /*
           * Find the last operator with an operator
           * as the last factor.
           */
          int tmp = op1->right;
          if((obj->ast.buffer[tmp]) != LPAREN &&
             (obj->ast.buffer[tmp]) != TFUNCTION &&
             (obj->ast.buffer[tmp]) != TNUMBER &&
             (obj->ast.buffer[tmp]) != VFLOAT &&
             (obj->ast.buffer[tmp]) != VINTEGER &&
             (obj->ast.buffer[tmp]) != VNULL) {
            while((obj->ast.buffer[tmp]) == TOPERATOR) {
              if((obj->ast.buffer[((struct vm_toperator_t *)&obj->ast.buffer[tmp])->right]) == TOPERATOR) {
                tmp = ((struct vm_toperator_t *)&obj->ast.buffer[tmp])->right;
              } else {
                break;
              }
            }
          } else {
            tmp = *step_out;
          }

          op3 = (struct vm_toperator_t *)&obj->ast.buffer[tmp];
          int tright = op3->right;
          op3->right = step;

          switch((obj->ast.buffer[tright])) {
            case TNUMBER: {
              struct vm_tnumber_t *node = (struct vm_tnumber_t *)&obj->ast.buffer[*step_out];
              node->ret = step;
            } break;
            case VINTEGER: {
              struct vm_vinteger_t *node = (struct vm_vinteger_t *)&obj->ast.buffer[*step_out];
              node->ret = step;
            } break;
            case VFLOAT: {
              struct vm_vfloat_t *node = (struct vm_vfloat_t *)&obj->ast.buffer[*step_out];
              node->ret = step;
            } break;
            case TFUNCTION: {
              struct vm_tfunction_t *node = (struct vm_tfunction_t *)&obj->ast.buffer[tright];
              node->ret = step;
            } break;
            case LPAREN: {
              struct vm_lparen_t *node = (struct vm_lparen_t *)&obj->ast.buffer[tright];
              node->ret = step;
            } break;
            /* LCOV_EXCL_START*/
            default: {
              logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
              return -1;
            } break;
            /* LCOV_EXCL_STOP*/
          }

          op2->left = tright;
          op2->ret = tmp;
        }
        step = *step_out;
      } else {
        op1->ret = step;
        *step_out = step;
      }
    }

    *step_out = step;

    first = 0;
  }

  return step;
}

static int rule_parse(char **text, int *length, struct rules_t *obj) {
  int type = 0, type1 = 0, go = -1, step_out = -1, step = 0, pos = 0;
  int has_paren = -1, has_function = -1, has_if = -1, has_on = -1;
  int loop = 1, startnode = 0, r_rewind = -1, offset = 0, start = 0, len = 0;

  /* LCOV_EXCL_START*/
  if(lexer_peek(text, 0, &type, &start, &len) < 0) {
    logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
    return -1;
  }
  /* LCOV_EXCL_STOP*/

  if(type != TIF && type != TEVENT) {
    logprintf_P(F("ERROR: Expected an 'if' or an 'on' statement"));
    return -1;
  }

  startnode = vm_parent(text, obj, TSTART, 0, 0, 0);

  while(loop) {
#ifdef ESP8266
    delay(0);
#endif
    if(go > -1) {
      switch(go) {
        /* LCOV_EXCL_START*/
        case TSTART: {
        /*
         * This should never be reached, see
         * the go = TSTART comment below.
         */
        // loop = 0;
        logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
        return -1;
        /* LCOV_EXCL_STOP*/
        } break;
        case TEVENT: {
          if(lexer_peek(text, pos, &type, &start, &len) < 0) {
            /* LCOV_EXCL_START*/
            logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
            return -1;
            /* LCOV_EXCL_STOP*/
          }
          if((step_out == -1 || (obj->ast.buffer[step_out]) == TTRUE) && type != TTHEN) {
            if(type == TEVENT) {
              if(offset > 0) {
                logprintf_P(F("ERROR: nested 'on' block"));
                return -1;
              }

              step_out = vm_parent(text, obj, TEVENT, start, len, 0);
              pos++;

              if(lexer_peek(text, pos, &type, &start, &len) >= 0 && type == TTHEN) {
                pos++;

                /*
                 * Predict how many go slots we need to
                 * reserve for the TRUE / FALSE nodes.
                 */
                int y = pos, nrexpressions = 0;
                while(lexer_peek(text, y++, &type, &start, &len) >= 0) {
                  if(type == TEOF) {
                    break;
                  }
                  if(type == TIF) {
                    struct vm_cache_t *cache = vm_cache_get(TIF, y-1);
                    nrexpressions++;
                    y = cache->end;
                    continue;
                  }
                  if(type == TEND) {
                    break;
                  }
                  if(type == TSEMICOLON) {
                    nrexpressions++;
                  }
                }

                if(nrexpressions == 0) {
                  logprintf_P(F("ERROR: On block without body"));
                  return -1;
                }
#ifdef DEBUG
                printf("nrexpressions: %d\n", nrexpressions);/*LCOV_EXCL_LINE*/
#endif

                /*
                 * The last parameter is used for
                 * for the number of operations the
                 * TRUE of FALSE will forward to.
                 */
                step = vm_parent(text, obj, TTRUE, start, len, nrexpressions);

                struct vm_ttrue_t *a = (struct vm_ttrue_t *)&obj->ast.buffer[step];
                struct vm_tevent_t *b = (struct vm_tevent_t *)&obj->ast.buffer[step_out];

                b->go = step;
                a->ret = step_out;

                go = TEVENT;

                step_out = step;
              } else {
                logprintf_P(F("ERROR: Expected a 'then' token"));
                return -1;
              }
            } else {
              switch(type) {
                case TVAR: {
                  go = TVAR;
                  continue;
                } break;
                case TCEVENT: {
                  go = TCEVENT;
                  continue;
                } break;
                case TFUNCTION: {
                  struct vm_cache_t *x = vm_cache_get(TFUNCTION, pos);
                  /* LCOV_EXCL_START*/
                  if(x == NULL) {
                    logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                    return -1;
                  }
                  /* LCOV_EXCL_STOP*/
                  if(lexer_peek(text, x->end, &type, &start, &len) >= 0) {
                    switch(type) {
                      case TSEMICOLON: {
                        int tmp = vm_rewind2(obj, step_out, TTRUE, TFALSE);
                        struct vm_tfunction_t *f = (struct vm_tfunction_t *)&obj->ast.buffer[x->step];
                        struct vm_ttrue_t *t = (struct vm_ttrue_t *)&obj->ast.buffer[step_out];
                        f->ret = tmp;

                        int i = 0;
                        for(i=0;i<t->nrgo;i++) {
                          if(t->go[i] == 0) {
                            t->go[i] = x->step;
                            break;
                          }
                        }

                        /* LCOV_EXCL_START*/
                        if(i == t->nrgo) {
                          logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                          return -1;
                        }
                        /* LCOV_EXCL_STOP*/

                        go = TEVENT;
                        step_out = tmp;
                        pos = x->end + 1;
                        vm_cache_del(x->start);
                      } break;
                      case TOPERATOR: {
                        go = type;
                      } break;
                      default: {
                        logprintf_P(F("ERROR: Expected an semicolon or operator"));
                        return -1;
                      } break;
                    }
                  }
                  continue;
                } break;
                case TIF: {
                  struct vm_cache_t *cache = vm_cache_get(TIF, pos);
                  /* LCOV_EXCL_START*/
                  if(cache == NULL) {
                    logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                    return -1;
                  }
                  /* LCOV_EXCL_STOP*/

                  step = cache->step;
                  pos = cache->end;

                  /*
                   * After an cached IF block has been linked
                   * it can be removed from cache
                   */
                  vm_cache_del(cache->start);

                  struct vm_tif_t *i = (struct vm_tif_t *)&obj->ast.buffer[step];

                  /*
                   * Attach IF block to TRUE / FALSE node
                   */

                  i->ret = step_out;
                  switch(obj->ast.buffer[step_out]) {
                    case TFALSE:
                    case TTRUE: {
                      int x = 0;
                      struct vm_ttrue_t *t = (struct vm_ttrue_t *)&obj->ast.buffer[step_out];
                      /*
                       * Attach the IF block to an empty
                       * TRUE / FALSE go slot.
                       */
                      for(x=0;x<t->nrgo;x++) {
                        if(t->go[x] == 0) {
                          t->go[x] = step;
                          break;
                        }
                      }
                      /* LCOV_EXCL_START*/
                      if(x == t->nrgo) {
                        logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                        return -1;
                      }
                      /* LCOV_EXCL_STOP*/
                      continue;
                    } break;
                    /* LCOV_EXCL_START*/
                    default: {
                      logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                      return -1;
                    } break;
                    /* LCOV_EXCL_STOP*/
                  }
                  /* LCOV_EXCL_START*/
                  logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                  return -1;
                  /* LCOV_EXCL_STOP*/
                } break;
                case TEOF:
                case TEND: {
                  go = -1;

                  int tmp = vm_rewind(obj, step_out, TEVENT);
                  /*
                   * Should never happen
                   */
                  /* LCOV_EXCL_START*/
                  if(has_on > 0) {
                    logprintf_P(F("ERROR: On block inside on block"));
                    return -1;
                  }
                  /* LCOV_EXCL_START*/

                  if(has_on == 0) {
                    struct vm_tstart_t *b = (struct vm_tstart_t *)&obj->ast.buffer[startnode];
                    struct vm_tif_t *a = (struct vm_tif_t *)&obj->ast.buffer[tmp];
                    b->go = tmp;
                    a->ret = startnode;

                    step = vm_parent(text, obj, TEOF, 0, 0, 0);
#ifdef DEBUG
                    printf("nr steps: %d\n", step);/*LCOV_EXCL_LINE*/
#endif
                    tmp = vm_rewind(obj, tmp, TSTART);

                    struct vm_tstart_t *node = (struct vm_tstart_t *)&obj->ast.buffer[tmp];
                    node->ret = step;

                    if(lexer_peek(text, pos, &type, &start, &len) < 0) {
                      /* LCOV_EXCL_START*/
                      logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                      return -1;
                      /* LCOV_EXCL_STOP*/
                    }

                    pos++;
                  }

                  /*
                   * If we are the root IF block
                   */
                  if(has_on == 0) {
                    loop = 0;
                  }

                  pos = 0;
                  has_paren = -1;
                  has_function = -1;
                  has_if = -1;
                  has_on = -1;
                  step_out = -1;

                  continue;
                } break;
                default: {
                  logprintf_P(F("ERROR: Unexpected token"));
                  return -1;
                } break;
              }
            }
          } else {
            /* LCOV_EXCL_START*/
            logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
            return -1;
            /* LCOV_EXCL_STOP*/
          }
        } break;
        case TIF: {
          if(lexer_peek(text, pos, &type, &start, &len) < 0) {
            /* LCOV_EXCL_START*/
            logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
            return -1;
            /* LCOV_EXCL_STOP*/
          }

          if(
              (
                step_out == -1 ||
                (obj->ast.buffer[step_out]) == TTRUE ||
                (obj->ast.buffer[step_out]) == TFALSE
              ) && (type != TTHEN && type != TELSE)
            ) {

            /*
             * Link cached IF blocks together
             */
            struct vm_cache_t *cache = vm_cache_get(TIF, pos);
            if(cache != NULL) {
              step = cache->step;

              if(lexer_peek(text, cache->end, &type, &start, &len) < 0) {
                /* LCOV_EXCL_START*/
                logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                return -1;
                /* LCOV_EXCL_STOP*/
              }

              struct vm_tif_t *i = (struct vm_tif_t *)&obj->ast.buffer[step];

              /*
               * Attach IF block to TRUE / FALSE node
               */

              i->ret = step_out;
              pos = cache->end;

              /*
               * After an cached IF block has been linked
               * it can be removed from cache
               */
              vm_cache_del(cache->start);

              switch(obj->ast.buffer[step_out]) {
                case TFALSE:
                case TTRUE: {
                  int x = 0;
                  struct vm_ttrue_t *t = (struct vm_ttrue_t *)&obj->ast.buffer[step_out];
                  /*
                   * Attach the IF block to an empty
                   * TRUE / FALSE go slot.
                   */
                  for(x=0;x<t->nrgo;x++) {
                    if(t->go[x] == 0) {
                      t->go[x] = step;
                      break;
                    }
                  }
                  /* LCOV_EXCL_START*/
                  if(x == t->nrgo) {
                    logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                    return -1;
                  }
                  /* LCOV_EXCL_STOP*/
                } break;
                /* LCOV_EXCL_START*/
                default: {
                  logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                  return -1;
                } break;
                /* LCOV_EXCL_STOP*/
              }

              /*
               * An IF block directly followed by another IF block
               */
              if(lexer_peek(text, pos, &type, &start, &len) >= 0 && type == TIF) {
                go = TIF;
                continue;
              }
            }

            if(type == TIF) {
              step = vm_parent(text, obj, TIF, start, len, 0);
              struct vm_tif_t *a = (struct vm_tif_t *)&obj->ast.buffer[step];

              if(pos == 0) {
                struct vm_tstart_t *b = (struct vm_tstart_t *)&obj->ast.buffer[start];
                b->go = step;
                a->ret = start;
              }

              pos++;

              if(lexer_peek(text, pos+1, &type, &start, &len) >= 0 && type == TOPERATOR) {
                go = TOPERATOR;
                step_out = step;
                continue;
              } else if(lexer_peek(text, pos, &type, &start, &len) >= 0 && type == LPAREN) {
                step_out = step;

                /*
                 * Link cached parenthesis blocks
                 */
                struct vm_cache_t *cache = vm_cache_get(LPAREN, pos);
                /* LCOV_EXCL_START*/
                if(cache == NULL) {
                  logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                  return -1;
                }
                /* LCOV_EXCL_STOP*/

                /*
                 * If this parenthesis is part of a operator
                 * let the operator handle it.
                 */
                if(lexer_peek(text, cache->end, &type, &start, &len) < 0) {
                  /* LCOV_EXCL_START*/
                  logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                  return -1;
                  /* LCOV_EXCL_STOP*/
                }


                struct vm_lparen_t *c = (struct vm_lparen_t *)&obj->ast.buffer[cache->step];
                if(type == TOPERATOR) {
                  go = TOPERATOR;
                  step_out = step;
                  continue;
                } else if(type == TTHEN) {
                  step_out = c->go;
                  go = TIF;
                  a->go = cache->step;
                  c->ret = step;
                } else {
                  logprintf_P(F("ERROR: Unexpected token"));
                  return -1;
                }

                pos = cache->end;

                vm_cache_del(cache->start);
                continue;
              } else if(lexer_peek(text, pos, &type, &start, &len) >= 0 && type == TFUNCTION) {
                step_out = step;

                /*
                 * Link cached parenthesis blocks
                 */
                struct vm_cache_t *cache = vm_cache_get(TFUNCTION, pos);

                if(lexer_peek(text, cache->end, &type, &start, &len) >= 0 && type == TOPERATOR) {
                  go = TOPERATOR;
                  step_out = step;
                  continue;
                } else {
                  logprintf_P(F("ERROR: Function without operator in if condition"));
                  return -1;
                }
              } else {
                logprintf_P(F("ERROR: Expected a parenthesis block, function or operator"));
                return -1;
              }
            } else {
              switch(type) {
                case TVAR: {
                  go = TVAR;
                  continue;
                } break;
                case TCEVENT: {
                  go = TCEVENT;
                  continue;
                } break;
                case TELSE: {
                  go = TIF;
                  continue;
                } break;
                case TFUNCTION: {
                  struct vm_cache_t *x = vm_cache_get(TFUNCTION, pos);
                  /* LCOV_EXCL_START*/
                  if(x == NULL) {
                    logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                    return -1;
                  }
                  /* LCOV_EXCL_STOP*/

                  if(lexer_peek(text, x->end, &type, &start, &len) >= 0) {
                    switch(type) {
                      case TSEMICOLON: {
                        int tmp = vm_rewind2(obj, step_out, TTRUE, TFALSE);
                        struct vm_tfunction_t *f = (struct vm_tfunction_t *)&obj->ast.buffer[x->step];
                        struct vm_ttrue_t *t = (struct vm_ttrue_t *)&obj->ast.buffer[step_out];
                        f->ret = tmp;

                        int i = 0;
                        for(i=0;i<t->nrgo;i++) {
                          if(t->go[i] == 0) {
                            t->go[i] = x->step;
                            break;
                          }
                        }

                        if(i == t->nrgo) {
                          /* LCOV_EXCL_START*/
                          logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                          return -1;
                          /* LCOV_EXCL_STOP*/
                        }

                        go = TIF;
                        step_out = tmp;
                        pos = x->end + 1;
                        vm_cache_del(x->start);
                      } break;
                      case TOPERATOR: {
                        go = type;
                      } break;
                      default: {
                        logprintf_P(F("ERROR: Expected an semicolon or operator"));
                        return -1;
                      } break;
                    }
                  }
                  continue;
                } break;
                case TEOF:
                case TEND: {
                  go = -1;

                  int tmp = vm_rewind(obj, step_out, TIF);
                  if(has_if > 0) {
                    r_rewind = has_if;

                    if(lexer_peek(text, pos, &type, &start, &len) < 0 || type != TEND) {
                      /* LCOV_EXCL_START*/
                      logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                      return -1;
                      /* LCOV_EXCL_STOP*/
                    }

                    pos++;

                    vm_cache_add(TIF, tmp, has_if, pos);
                  }
                  if(has_if == 0) {
                    struct vm_tstart_t *b = (struct vm_tstart_t *)&obj->ast.buffer[startnode];
                    struct vm_tif_t *a = (struct vm_tif_t *)&obj->ast.buffer[tmp];
                    b->go = tmp;
                    a->ret = startnode;

                    step = vm_parent(text, obj, TEOF, 0, 0, 0);
#ifdef DEBUG
                    printf("nr steps: %d\n", step);/*LCOV_EXCL_LINE*/
#endif
                    tmp = vm_rewind(obj, tmp, TSTART);

                    struct vm_tstart_t *node = (struct vm_tstart_t *)&obj->ast.buffer[tmp];
                    node->ret = step;

                    if(lexer_peek(text, pos, &type, &start, &len) < 0) {
                      /* LCOV_EXCL_START*/
                      logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                      return -1;
                      /* LCOV_EXCL_STOP*/
                    }

                    pos++;
                  }

                  /*
                   * If we are the root IF block
                   */
                  if(has_if == 0) {
                    loop = 0;
                  }

                  pos = 0;
                  has_paren = -1;
                  has_function = -1;
                  has_if = -1;
                  has_on = -1;
                  step_out = -1;

                  continue;
                } break;
                default: {
                  logprintf_P(F("ERROR: Unexpected token"));
                  return -1;
                } break;
              }
            }
          } else {
            int t = 0;

            if(lexer_peek(text, pos, &type, &start, &len) < 0) {
              /* LCOV_EXCL_START*/
              logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
              return -1;
              /* LCOV_EXCL_STOP*/
            }

            pos++;

            if(type == TTHEN) {
              t = TTRUE;
            } else if(type == TELSE) {
              t = TFALSE;
            }

            /*
             * Predict how many go slots we need to
             * reserve for the TRUE / FALSE nodes.
             */
            int y = pos, nrexpressions = 0, z = 0;

            while((z = lexer_peek(text, y++, &type, &start, &len)) >= 0) {
              if(type == TEOF) {
                break;
              }
              if(type == TIF) {
                struct vm_cache_t *cache = vm_cache_get(TIF, y-1);
                if(cache == NULL) {
                  /* LCOV_EXCL_START*/
                  logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                  return -1;
                  /* LCOV_EXCL_STOP*/
                }
                nrexpressions++;
                y = cache->end;
                continue;
              }
              if(type == TEND) {
                break;
              }
              if(type == TSEMICOLON) {
                nrexpressions++;
              }
              if(t == TTRUE && type == TELSE) {
                break;
              }
            }

            if(nrexpressions == 0) {
              logprintf_P(F("ERROR: If block without body"));
              return -1;
            }
#ifdef DEBUG
            printf("nrexpressions: %d\n", nrexpressions);/*LCOV_EXCL_LINE*/
#endif
            /*
             * If we came from further in the script
             * make sure we hook our 'then' and 'else'
             * to the last condition.
             */
            step_out = vm_rewind(obj, step_out, TIF);

            if(lexer_peek(text, pos, &type, &start, &len) < 0) {
              /* LCOV_EXCL_START*/
              logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
              return -1;
              /* LCOV_EXCL_STOP*/
            }

            /*
             * The last parameter is used for
             * for the number of operations the
             * TRUE of FALSE will forward to.
             */
            step = vm_parent(text, obj, t, 0, 0, nrexpressions);

            struct vm_ttrue_t *a = (struct vm_ttrue_t *)&obj->ast.buffer[step];
            struct vm_tif_t *b = (struct vm_tif_t *)&obj->ast.buffer[step_out];

            if(t == TTRUE) {
              b->true_ = step;
            } else {
              b->false_ = step;
            }
            a->ret = step_out;

            go = TIF;

            step_out = step;
          }
        } break;
        case TOPERATOR: {
          int a = -1;
          int source = step_out;

          if(lexer_peek(text, pos, &a, &start, &len) >= 0) {
            switch(a) {
              case LPAREN: {
                struct vm_cache_t *x = vm_cache_get(LPAREN, pos);
                /* LCOV_EXCL_START*/
                if(x == NULL) {
                  logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                  return -1;
                }
                /* LCOV_EXCL_STOP*/
                int oldpos = pos;
                pos = x->end;
                step_out = x->step;
                vm_cache_del(oldpos);
              } break;
              case TFUNCTION: {
                struct vm_cache_t *x = vm_cache_get(TFUNCTION, pos);
                /* LCOV_EXCL_START*/
                if(x == NULL) {
                  logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                  return -1;
                }
                /* LCOV_EXCL_STOP*/
                int oldpos = pos;
                pos = x->end;
                step_out = x->step;
                vm_cache_del(oldpos);
              } break;
              case TVAR:
              case VNULL:
              case TNUMBER1:
              case TNUMBER2:
              case TNUMBER3:
              case VFLOAT:
              case VINTEGER: {
                step_out = vm_parent(text, obj, a, start, len, 0);
                pos++;
              } break;
              /* LCOV_EXCL_START*/
              default: {
                logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                return -1;
              } break;
              /* LCOV_EXCL_STOP*/
            }
          }

          /*
           * The return value is the positition
           * of the root operator
           */
          int step = lexer_parse_math_order(text, length, obj, TOPERATOR, &pos, &step_out, offset, source);
          if(step == -1) {
            return -1;
          }

          {
            struct vm_tgeneric_t *node = (struct vm_tgeneric_t *)&obj->ast.buffer[step_out];
            node->ret = source;

            switch(obj->ast.buffer[source]) {
              case TIF: {
                struct vm_tif_t *node = (struct vm_tif_t *)&obj->ast.buffer[source];
                node->go = step;
              } break;
              case TVAR: {
                struct vm_tvar_t *node = (struct vm_tvar_t *)&obj->ast.buffer[source];
                node->go = step;
              } break;
              /*
               * If this operator block is a function
               * argument, attach it to a empty go slot
               */
              case TFUNCTION: {
                struct vm_tfunction_t *node = (struct vm_tfunction_t *)&obj->ast.buffer[source];
                int i = 0;
                for(i=0;i<node->nrgo;i++) {
                  if(node->go[i] == 0) {
                    node->go[i] = step;
                    break;
                  }
                }
                go = TFUNCTION;
                step_out = step;
                continue;
              } break;
              default: {
                logprintf_P(F("ERROR: Unexpected token"));
                return -1;
              } break;
            }
          }
          if(lexer_peek(text, pos, &type, &start, &len) >= 0) {
            switch(type) {
              case TTHEN: {
                go = TIF;
              } break;
              case TSEMICOLON: {
                go = TVAR;
              } break;
              default: {
                logprintf_P(F("ERROR: Unexpected token"));
                return -1;
              } break;
            }
          }
        } break;
        case VNULL: {
          struct vm_vnull_t *node = NULL;
          if(lexer_peek(text, pos, &type, &start, &len) < 0) {
            /* LCOV_EXCL_START*/
            logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
            return -1;
            /* LCOV_EXCL_STOP*/
          }

          step = vm_parent(text, obj, VNULL, start, len, 0);
          pos++;
          switch((obj->ast.buffer[step_out])) {
            case TVAR: {
              struct vm_tvar_t *tmp = (struct vm_tvar_t *)&obj->ast.buffer[step_out];
              tmp->go = step;
            } break;
            /*
             * FIXME: Think of a rule that can trigger this
             */
            /* LCOV_EXCL_START*/
            default: {
              logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
              return -1;
            } break;
            /* LCOV_EXCL_STOP*/
          }

          node = (struct vm_vnull_t *)&obj->ast.buffer[step];
          node->ret = step_out;

          int tmp = vm_rewind2(obj, step_out, TVAR, TOPERATOR);
          go = (obj->ast.buffer[tmp]);
          step_out = step;
        } break;
        /* LCOV_EXCL_START*/
        case TSEMICOLON: {
          logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
          return -1;
        } break;
        /* LCOV_EXCL_STOP*/
        case TVAR: {
          /*
           * We came from the TRUE node from the IF root,
           * which means we start parsing the variable name.
           */
          if((obj->ast.buffer[step_out]) == TTRUE || (obj->ast.buffer[step_out]) == TFALSE) {
            struct vm_tvar_t *node = NULL;
            struct vm_ttrue_t *node1 = NULL;

            if(lexer_peek(text, pos, &type, &start, &len) < 0) {
              /* LCOV_EXCL_START*/
              logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
              return -1;
              /* LCOV_EXCL_STOP*/
            }

            step = vm_parent(text, obj, TVAR, start, len, 0);

            pos++;

            node = (struct vm_tvar_t *)&obj->ast.buffer[step];
            node1 = (struct vm_ttrue_t *)&obj->ast.buffer[step_out];

            int x = 0;
            for(x=0;x<node1->nrgo;x++) {
              if(node1->go[x] == 0) {
                node1->go[x] = step;
                break;
              }
            }

            /* LCOV_EXCL_START*/
            if(x == node1->nrgo) {
              logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
              return -1;
            }
            /* LCOV_EXCL_STOP*/
            node->ret = step_out;

            step_out = step;

            if(lexer_peek(text, pos, &type, &start, &len) < 0 || type != TASSIGN) {
              /* LCOV_EXCL_START*/
              logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
              return -1;
              /* LCOV_EXCL_STOP*/
            }

            pos++;

            if(lexer_peek(text, pos+1, &type, &start, &len) >= 0 && type == TOPERATOR) {
              switch(type) {
                case TOPERATOR: {
                  go = TOPERATOR;
                  step_out = step;
                  continue;
                } break;
              }
            } else if(lexer_peek(text, pos, &type, &start, &len) >= 0) {
              switch(type) {
                case VINTEGER:
                case VFLOAT:
                case TNUMBER1:
                case TNUMBER2:
                case TNUMBER3: {
                  int foo = vm_parent(text, obj, type, start, len, 0);
                  struct vm_tgeneric_t *a = (struct vm_tgeneric_t *)&obj->ast.buffer[foo];
                  node = (struct vm_tvar_t *)&obj->ast.buffer[step];
                  node->go = foo;
                  a->ret = step;

                  pos++;

                  go = TVAR;
                } break;
                case TVAR: {
                  go = TVAR;
                  step_out = step;
                } break;
                case VNULL: {
                  go = VNULL;
                  step_out = step;
                } break;
                case TFUNCTION: {
                  struct vm_cache_t *x = vm_cache_get(TFUNCTION, pos);
                  /* LCOV_EXCL_START*/
                  if(x == NULL) {
                    logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                    return -1;
                  }
                  /* LCOV_EXCL_STOP*/
                  if(lexer_peek(text, x->end, &type, &start, &len) >= 0) {
                    switch(type) {
                      case TSEMICOLON: {
                        struct vm_tfunction_t *f = (struct vm_tfunction_t *)&obj->ast.buffer[x->step];
                        struct vm_tvar_t *v = (struct vm_tvar_t *)&obj->ast.buffer[step_out];
                        f->ret = step;
                        v->go = x->step;

                        int tmp = vm_rewind2(obj, step_out, TTRUE, TFALSE);
                        int tmp1 = vm_rewind2(obj, tmp, TIF, TEVENT);

                        go = obj->ast.buffer[tmp1];
                        step_out = tmp;
                        pos = x->end + 1;
                        vm_cache_del(x->start);
                      } break;
                      case TOPERATOR: {
                        go = type;
                      } break;
                      default: {
                        logprintf_P(F("ERROR: Expected an semicolon or operator"));
                        return -1;
                      } break;
                    }
                  }
                } break;
                case LPAREN: {
                  int oldpos = pos;
                  struct vm_cache_t *x = vm_cache_get(LPAREN, pos);
                  /* LCOV_EXCL_START*/
                  if(x == NULL) {
                    logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                    return -1;
                  }

                  /* LCOV_EXCL_STOP*/
                  if(lexer_peek(text, x->end, &type, &start, &len) >= 0) {
                    if(type == TSEMICOLON) {
                      struct vm_lparen_t *l = (struct vm_lparen_t *)&obj->ast.buffer[x->step];
                      struct vm_tvar_t *v = (struct vm_tvar_t *)&obj->ast.buffer[step_out];
                      l->ret = step;
                      v->go = x->step;

                      int tmp = vm_rewind2(obj, step_out, TTRUE, TFALSE);

                      go = TIF;
                      step_out = tmp;

                      pos = x->end + 1;

                      vm_cache_del(oldpos);
                    } else {
                      go = type;
                    }
                  }
                } break;
                default: {
                  logprintf_P(F("ERROR: Unexpected token"));
                  return -1;
                } break;
              }
            } else {
              /* LCOV_EXCL_START*/
              logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
              return -1;
              /* LCOV_EXCL_STOP*/
            }
          /*
           * The variable has been called as a value
           */
          } else if((obj->ast.buffer[step_out]) == TVAR && lexer_peek(text, pos, &type, &start, &len) >= 0 && type != TSEMICOLON) {
            step = vm_parent(text, obj, TVAR, start, len, 0);
            pos++;

            struct vm_tvar_t *in = (struct vm_tvar_t *)&obj->ast.buffer[step];
            struct vm_tvar_t *out = (struct vm_tvar_t *)&obj->ast.buffer[step_out];
            in->ret = step_out;
            out->go = step;

            if(lexer_peek(text, pos, &type, &start, &len) >= 0) {

              switch(type) {
                case TSEMICOLON: {
                  go = TVAR;
                } break;
                default: {
                  logprintf_P(F("ERROR: Expected a semicolon"));
                  return -1;
                } break;
              }
            }
          } else {
            if(lexer_peek(text, pos, &type, &start, &len) < 0 || type != TSEMICOLON) {
              /* LCOV_EXCL_START*/
              logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
              return -1;
              /* LCOV_EXCL_STOP*/
            }

            int tmp = step_out;
            pos++;
            while(1) {
              if((obj->ast.buffer[tmp]) != TTRUE &&
                 (obj->ast.buffer[tmp]) != TFALSE) {
                struct vm_tgeneric_t *node = (struct vm_tgeneric_t *)&obj->ast.buffer[tmp];
                tmp = node->ret;
              } else {
                int tmp1 = vm_rewind2(obj, tmp, TIF, TEVENT);
                go = obj->ast.buffer[tmp1];
                step_out = tmp;
                break;
              }
            }
          }
        } break;
        case TCEVENT: {
          struct vm_tcevent_t *node = NULL;
          /* LCOV_EXCL_START*/
          if(lexer_peek(text, pos, &type, &start, &len) < 0 || type != TCEVENT) {
            logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
            return -1;
          }
          /* LCOV_EXCL_STOP*/

          step = vm_parent(text, obj, TCEVENT, start, len, 0);

          node = (struct vm_tcevent_t *)&obj->ast.buffer[step];

          pos++;

          if(lexer_peek(text, pos, &type, &start, &len) < 0) {
            /* LCOV_EXCL_START*/
            logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
            return -1;
            /* LCOV_EXCL_STOP*/
          }

          if(type != TSEMICOLON) {
            logprintf_P(F("ERROR: Expected a semicolon"));
            return -1;
          }

          pos++;

          int tmp = vm_rewind2(obj, step_out, TTRUE, TFALSE);
          struct vm_ttrue_t *node1 = (struct vm_ttrue_t *)&obj->ast.buffer[tmp];

          int x = 0;
          for(x=0;x<node1->nrgo;x++) {
            if(node1->go[x] == 0) {
              node1->go[x] = step;
              break;
            }
          }

          /* LCOV_EXCL_START*/
          if(x == node1->nrgo) {
            logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
            return -1;
          }
          /* LCOV_EXCL_STOP*/
          node->ret = step_out;

          int tmp1 = vm_rewind2(obj, step_out, TIF, TEVENT);
          go = obj->ast.buffer[tmp1];
          step_out = tmp;
        } break;
        case LPAREN: {
          int a = -1, b = -1;

          if(lexer_peek(text, has_paren+1, &a, &start, &len) >= 0) {
            switch(a) {
              case LPAREN: {
                struct vm_cache_t *x = vm_cache_get(LPAREN, has_paren+1);
                /* LCOV_EXCL_START*/
                if(x == NULL) {
                  logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                  return -1;
                }
                /* LCOV_EXCL_STOP*/
                pos = x->end;
                step_out = x->step;
                vm_cache_del(has_paren+1);
              } break;
              case TFUNCTION: {
                struct vm_cache_t *x = vm_cache_get(TFUNCTION, has_paren+1);
                /* LCOV_EXCL_START*/
                if(x == NULL) {
                  logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                  return -1;
                }
                /* LCOV_EXCL_STOP*/
                step_out = x->step;
                pos = x->end;
                vm_cache_del(has_paren+1);
              } break;
              case TVAR:
              case VNULL:
              case VINTEGER:
              case VFLOAT:
              case TNUMBER1:
              case TNUMBER2:
              case TNUMBER3: {
                pos = has_paren + 1;
                step_out = vm_parent(text, obj, a, start, len, 0);
                pos++;
              } break;
              case RPAREN: {
                logprintf_P(F("ERROR: Empty parenthesis block"));
                return -1;
              } break;
              case TOPERATOR: {
                logprintf_P(F("ERROR: Unexpected operator"));
                return -1;
              } break;
              default: {
                logprintf_P(F("ERROR: Unexpected token"));
                return -1;
              }
            }
          }

          /*
           * The return value is the positition
           * of the root operator
           */
          int step = lexer_parse_math_order(text, length, obj, LPAREN, &pos, &step_out, offset, 0);

          if(lexer_peek(text, pos, &b, &start, &len) < 0) {
            /* LCOV_EXCL_START*/
            logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
            return -1;
            /* LCOV_EXCL_STOP*/
          }

          if(b != RPAREN) {
            logprintf_P(F("ERROR: Expected a right parenthesis"));
            return -1;
          }

          pos++;

          {
            struct vm_tgeneric_t *node = (struct vm_tgeneric_t *)&obj->ast.buffer[step_out];
            node->ret = 0;
          }

          if(lexer_peek(text, has_paren, &type, &start, &len) < 0 || type != LPAREN) {
            /* LCOV_EXCL_START*/
            logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
            return -1;
            /* LCOV_EXCL_STOP*/
          }

          step = vm_parent(text, obj, LPAREN, start, len, 0);

          struct vm_lparen_t *node = (struct vm_lparen_t *)&obj->ast.buffer[step];
          node->go = step_out;

          vm_cache_add(LPAREN, step, has_paren, pos);

          r_rewind = has_paren;

          struct vm_tgeneric_t *node1 = (struct vm_tgeneric_t *)&obj->ast.buffer[step_out];
          node1->ret = step;

          go = -1;

          pos = 0;

          has_paren = -1;
          has_function = -1;
          has_if = -1;
          has_on = -1;
          step_out = -1;
        } break;
        case TFUNCTION: {
          struct vm_tfunction_t *node = NULL;
          int arg = 0;

          /*
           * We've entered the function name,
           * so we start by creating a proper
           * root node.
           */
          if(step_out == -1) {
            pos = has_function+1;
            if(lexer_peek(text, has_function, &type, &start, &len) < 0) {
              /* LCOV_EXCL_START*/
              logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
              return -1;
              /* LCOV_EXCL_STOP*/
            }

            /*
             * Determine how many arguments this
             * function has.
             */
            int y = pos + 1, nrargs = 0;
            while(lexer_peek(text, y++, &type, &start, &len) >= 0) {
              if(type == TEOF || type == RPAREN) {
                break;
              }
              if(type == LPAREN) {
                struct vm_cache_t *cache = vm_cache_get(LPAREN, y-1);
                if(cache == NULL) {
                  /* LCOV_EXCL_START*/
                  logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                  return -1;
                  /* LCOV_EXCL_STOP*/
                }
                y = cache->end;
                continue;
              }
              if(type == TFUNCTION) {
                struct vm_cache_t *cache = vm_cache_get(TFUNCTION, y-1);
                if(cache == NULL) {
                  /* LCOV_EXCL_START*/
                  logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                  return -1;
                  /* LCOV_EXCL_STOP*/
                }
                y = cache->end;
                continue;
              }
              if(type == TCOMMA) {
                nrargs++;
              }
            }
#ifdef DEBUG
            printf("nrarguments: %d\n", nrargs + 1);/*LCOV_EXCL_LINE*/
#endif
            if(lexer_peek(text, has_function, &type, &start, &len) < 0 || type != TFUNCTION) {
              /* LCOV_EXCL_START*/
              logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
              return -1;
              /* LCOV_EXCL_STOP*/
            }

            step = vm_parent(text, obj, TFUNCTION, start, len, nrargs + 1);

            pos++;

          /*
           * When the root node has been created
           * we parse the different arguments.
           */
          } else {
            int i = 0;
            step = vm_rewind(obj, step_out, TFUNCTION);

            node = (struct vm_tfunction_t *)&obj->ast.buffer[step];

            /*
             * Find the argument we've just
             * looked up.
             */
            for(i=0;i<node->nrgo;i++) {
              if(node->go[i] == step_out) {
                arg = i+1;
                break;
              }
            }

            /*
             * Look for the next token in the list
             * of arguments
             */
            while(1) {
              if(lexer_peek(text, pos, &type, &start, &len) < 0) {
                /* LCOV_EXCL_START*/
                logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                return -1;
                /* LCOV_EXCL_STOP*/
              }

              if(type == TCOMMA) {
                pos++;
                break;
              }
              if(type == RPAREN) {
                break;
              }
            }

            lexer_peek(text, pos, &type, &start, &len);
          }

          /*
           * When this token is a semicolon
           * we're done looking up the argument
           * nodes. In the other case, we try to
           * lookup the other arguments in the
           * list.
           */
          if(type != RPAREN) {
            while(1) {
              if(lexer_peek(text, pos + 1, &type, &start, &len) < 0) {
                /* LCOV_EXCL_START*/
                logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                return -1;
                /* LCOV_EXCL_STOP*/
              } else if(type == TOPERATOR) {
                go = TOPERATOR;
                step_out = step;
                break;
              }

              if(lexer_peek(text, pos, &type, &start, &len) < 0) {
                /* LCOV_EXCL_START*/
                logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                return -1;
                /* LCOV_EXCL_STOP*/
              }

              switch(type) {
                /*
                 * If the arguments are a parenthesis or
                 * another function, link those together.
                 */
                case LPAREN: {
                  struct vm_cache_t *cache = vm_cache_get(LPAREN, pos);
                  /* LCOV_EXCL_START*/
                  if(cache == NULL) {
                    logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                    return -1;
                  }
                  /* LCOV_EXCL_STOP*/

                  node = (struct vm_tfunction_t *)&obj->ast.buffer[step];
                  struct vm_lparen_t *paren = (struct vm_lparen_t *)&obj->ast.buffer[cache->step];
                  int oldpos = pos;
                  pos = cache->end;
                  node->go[arg++] = cache->step;
                  paren->ret = step;
                  vm_cache_del(oldpos);
                } break;
                case TFUNCTION: {
                  struct vm_cache_t *cache = vm_cache_get(TFUNCTION, pos);
                  /* LCOV_EXCL_START*/
                  if(cache == NULL) {
                    logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                    return -1;
                  }

                  node = (struct vm_tfunction_t *)&obj->ast.buffer[step];
                  struct vm_tfunction_t *func = (struct vm_tfunction_t *)&obj->ast.buffer[cache->step];
                  /* LCOV_EXCL_STOP*/
                  int oldpos = pos;
                  pos = cache->end;

                  node->go[arg++] = cache->step;
                  func->ret = step;
                  vm_cache_del(oldpos);
                } break;
                case TNUMBER1:
                case TNUMBER2:
                case TNUMBER3: {
                  int a = vm_parent(text, obj, type, start, len, 0);

                  pos++;

                  node = (struct vm_tfunction_t *)&obj->ast.buffer[step];
                  node->go[arg++] = a;

                  struct vm_tnumber_t *tmp = (struct vm_tnumber_t *)&obj->ast.buffer[a];
                  tmp->ret = step;
                } break;
                case VNULL:
                case TVAR: {
                  int a = vm_parent(text, obj, type, start, len, 0);

                  pos++;

                  node = (struct vm_tfunction_t *)&obj->ast.buffer[step];
                  node->go[arg++] = a;

                  struct vm_tvar_t *tmp = (struct vm_tvar_t *)&obj->ast.buffer[a];
                  tmp->ret = step;
                } break;
                default: {
                  logprintf_P(F("ERROR: Unexpected token"));
                  return -1;
                } break;
              }

              if(lexer_peek(text, pos, &type, &start, &len) < 0) {
                /* LCOV_EXCL_START*/
                logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                return -1;
                /* LCOV_EXCL_STOP*/
              }

              /*
               * A right parenthesis means we've
               * reached the end of the argument list
               */
              if(type == RPAREN) {
                pos++;
                break;
              } else if(type != TCOMMA) {
                logprintf_P(F("ERROR: Expected a closing parenthesis"));
                return -1;
              }

              pos++;
            }
          } else {
            if(lexer_peek(text, pos, &type, &start, &len) < 0) {
              /* LCOV_EXCL_START*/
              logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
              return -1;
              /* LCOV_EXCL_STOP*/
            }

            pos++;
          }

          /*
           * When we're back to the function
           * root, cache it for further linking.
           */
          if(go == TFUNCTION) {
            vm_cache_add(TFUNCTION, step, has_function, pos);

            r_rewind = has_function;

            go = -1;

            pos = 0;

            has_paren = -1;
            has_function = -1;
            has_if = -1;
            has_on = -1;
            step_out = -1;
          }
        } break;
      }
    } else {
      /*
       * Start looking for the furthest
       * nestable token, so we can cache
       * it first.
       */
      if(r_rewind == -1) {
        if(lexer_peek(text, pos++, &type, &start, &len) < 0) {
          /* LCOV_EXCL_START*/
          logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
          return -1;
          /* LCOV_EXCL_STOP*/
        }
      /*
       * If we found the furthest, rewind back step
       * by step until we are at the beginning again.
       */
      } else {
        if(r_rewind > 0 && lexer_peek(text, --r_rewind, &type, &start, &len) < 0) {
          go = -1;
        }
        pos = r_rewind+1;
      }
      if(type == TFUNCTION) {
        has_function = pos-1;
      } else if(type == TIF) {
        has_if = pos-1;
      } else if(type == TEVENT) {
        has_on = pos-1;
      } else if(type == LPAREN &&
        lexer_peek(text, pos-2, &type1, &start, &len) >= 0 &&
         type1 != TFUNCTION && type1 != TCEVENT) {
        has_paren = pos-1;
      }
      if(has_function != -1 || has_paren != -1 || has_if != -1 || has_on != -1) {
        if(type == TEOF || r_rewind > -1) {
          if(MAX(has_function, MAX(has_paren, MAX(has_if, has_on))) == has_function) {
            offset = (pos = has_function)+1;
            go = TFUNCTION;
            continue;
          } else if(MAX(has_function, MAX(has_paren, MAX(has_if, has_on))) == has_if) {
            offset = (pos = has_if);
            if(has_if > 0) {
              offset++;
            }
            go = TIF;
            continue;
          } else if(MAX(has_function, MAX(has_paren, MAX(has_if, has_on))) == has_on) {
            offset = (pos = has_on);
            go = TEVENT;
            if(has_on > 0) {
              offset++;
            }
            continue;
          } else {
            offset = (pos = has_paren)+1;
            go = LPAREN;
            continue;
          }
          has_paren = -1;
          has_if = -1;
          has_on = -1;
          has_function = -1;
          step_out = -1;
          pos = 0;
        }
      } else if(r_rewind <= 0) {
        /*
         * This should never be reached, because at
         * r_rewind = 0 either an on or if token
         * should be found, already parsing the full
         * rule.
         */
        /* LCOV_EXCL_START*/
        /*
          go = TSTART;
          pos = 0;
          continue;
        */
        logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
        return -1;
        /* LCOV_EXCL_STOP*/
      }
    }
  }

/* LCOV_EXCL_START*/
  struct vm_tstart_t *node = (struct vm_tstart_t *)&obj->ast.buffer[0];
  if(node->go == 0) {
    logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
    return -1;
  }
/* LCOV_EXCL_STOP*/
  return 0;
}

/*LCOV_EXCL_START*/
#ifdef DEBUG
static void print_ast(struct rules_t *obj) {
  int i = 0, x = 0;

  for(i=x;alignedbytes(i)<obj->ast.nrbytes;i++) {
    i = alignedbytes(i);
    switch(obj->ast.buffer[i]) {
      case TSTART: {
        struct vm_tstart_t *node = (struct vm_tstart_t *)&obj->ast.buffer[i];
        printf("\"%d\"[label=\"START\"]\n", i);
        printf("\"%d\" -> \"%d\"\n", i, node->go);
        printf("\"%d\" -> \"%d\"\n", i, node->ret);
        i+=sizeof(struct vm_tstart_t)-1;
      } break;
      case TEOF: {
        printf("\"%d\"[label=\"EOF\"]\n", i);
        i+=sizeof(struct vm_teof_t)-1;
      } break;
      case VNULL: {
        printf("\"%d\"[label=\"NULL\"]\n", i);
        i+=sizeof(struct vm_vnull_t)-1;
      } break;
      case TIF: {
        struct vm_tif_t *node = (struct vm_tif_t *)&obj->ast.buffer[i];
        printf("\"%d\"[label=\"IF\"]\n", i);
        printf("\"%d\" -> \"%d\"\n", i, node->go);
        // printf("\"%i\" -> \"%i\"\n", i, node->ret);
        if(node->true_ > 0) {
          printf("\"%d\" -> \"%d\"\n", i, node->true_);
        }
        if(node->false_ > 0) {
          printf("\"%d\" -> \"%d\"\n", i, node->false_);
        }
        printf("{ rank=same edge[style=invis]");
        if(node->true_ > 0) {
          printf(" \"%d\" -> \"%d\";", node->go, node->true_);
        }
        if(node->false_ > 0) {
          printf(" \"%d\" -> \"%d\";", node->true_, node->false_);
        }
        printf(" rankdir = LR}\n");
        i+=sizeof(struct vm_tif_t)-1;
      } break;
      case LPAREN: {
        struct vm_lparen_t *node = (struct vm_lparen_t *)&obj->ast.buffer[i];
        printf("\"%d\"[label=\"(\"]\n", i);
        printf("\"%d\" -> \"%d\"\n", i, node->go);
        // printf("\"%d\" -> \"%d\"\n", i, node->ret);
        i+=sizeof(struct vm_lparen_t)-1;
      } break;
      case TFALSE:
      case TTRUE: {
        int x = 0;
        struct vm_ttrue_t *node = (struct vm_ttrue_t *)&obj->ast.buffer[i];
        if((obj->ast.buffer[i]) == TFALSE) {
          printf("\"%d\"[label=\"FALSE\"]\n", i);
        } else {
          printf("\"%d\"[label=\"TRUE\"]\n", i);
        }
        for(x=0;x<node->nrgo;x++) {
          printf("\"%d\" -> \"%d\"\n", i, node->go[x]);
        }
        printf("{ rank=same edge[style=invis] ");
        for(x=0;x<node->nrgo-1;x++) {
          printf("\"%d\" -> \"%d\"", node->go[x], node->go[x+1]);
          if(x+1 < node->nrgo-1) {
            printf(" -> ");
          }
        }
        printf(" rankdir = LR}\n");
        i+=sizeof(struct vm_ttrue_t)+(sizeof(uint16_t)*node->nrgo)-1;
      } break;
      case TFUNCTION: {
        int x = 0;
        struct vm_tfunction_t *node = (struct vm_tfunction_t *)&obj->ast.buffer[i];
        printf("\"%d\"[label=\"%s\"]\n", i, rule_functions[node->token].name);
        for(x=0;x<node->nrgo;x++) {
          printf("\"%d\" -> \"%d\"\n", i, node->go[x]);
        }
        // printf("\"%d\" -> \"%d\"\n", i, node->ret);
        i+=sizeof(struct vm_tfunction_t)+(sizeof(uint16_t)*node->nrgo)-1;
      } break;
      case TVAR: {
        struct vm_tvar_t *node = (struct vm_tvar_t *)&obj->ast.buffer[i];
        printf("\"%d\"[label=\"%s\"]\n", i, node->token);
        if(node->go > 0) {
          printf("\"%d\" -> \"%d\"\n", i, node->go);
        }
        // printf("\"%d\" -> \"%d\"\n", i, node->ret);
        i+=sizeof(struct vm_tvar_t)+strlen((char *)node->token);
      } break;
      case TNUMBER: {
        struct vm_tnumber_t *node = (struct vm_tnumber_t *)&obj->ast.buffer[i];
        printf("\"%d\"[label=\"%s\"]\n", i, node->token);
        i+=sizeof(struct vm_tnumber_t)+strlen((char *)node->token);
      } break;
      case VINTEGER: {
        struct vm_vinteger_t *node = (struct vm_vinteger_t *)&obj->ast.buffer[i];
        printf("\"%d\"[label=\"%d\"]\n", i, node->value);
        i+=sizeof(struct vm_vinteger_t)-1;
      } break;
      case VFLOAT: {
        struct vm_vfloat_t *node = (struct vm_vfloat_t *)&obj->ast.buffer[i];
        printf("\"%d\"[label=\"%g\"]\n", i, node->value);
        i+=sizeof(struct vm_vfloat_t)-1;
      } break;
      case TEVENT: {
        struct vm_tevent_t *node = (struct vm_tevent_t *)&obj->ast.buffer[i];
        printf("\"%d\"[label=\"%s\"]\n", i, node->token);
        // printf("\"%i\" -> \"%i\"\n", i, node->ret);
        if(node->go > 0) {
          printf("\"%d\" -> \"%d\"\n", i, node->go);
        }
        i+=sizeof(struct vm_tevent_t)+strlen((char *)node->token);
      } break;
      case TCEVENT: {
        struct vm_tcevent_t *node = (struct vm_tcevent_t *)&obj->ast.buffer[i];
        printf("\"%d\"[label=\"%s\"]\n", i, node->token);
        // printf("\"%i\" -> \"%i\"\n", i, node->ret);
        i+=sizeof(struct vm_tcevent_t)+strlen((char *)node->token);
      } break;
      case TOPERATOR: {
        struct vm_toperator_t *node = (struct vm_toperator_t *)&obj->ast.buffer[i];

        printf("\"%d\"[label=\"%s\"]\n", i, rule_operators[node->token].name);
        printf("\"%d\" -> \"%d\"\n", i, node->right);
        printf("\"%d\" -> \"%d\"\n", i, node->left);
        printf("{ rank=same edge[style=invis] \"%d\" -> \"%d\" rankdir = LR}\n", node->left, node->right);
        // printf("\"%d\" -> \"%d\"\n", i, node->ret);
        i+=sizeof(struct vm_toperator_t)-1;
      } break;
      default: {
      } break;
    }
  }
}

static void print_steps(struct rules_t *obj) {
  int i = 0, x = 0;

  for(i=x;alignedbytes(i)<obj->ast.nrbytes;i++) {
    i = alignedbytes(i);
    switch(obj->ast.buffer[i]) {
      case TSTART: {
        struct vm_tstart_t *node = (struct vm_tstart_t *)&obj->ast.buffer[i];
        printf("\"%d-1\"[label=\"%d\" shape=square]\n", i, i);
        printf("\"%d-2\"[label=\"START\"]\n", i);
        printf("\"%d-3\"[label=\"%d\" shape=square]\n", i, node->ret);
        printf("\"%d-4\"[label=\"%d\" shape=square]\n", i, node->go);
        printf("\"%d-1\" -> \"%d-2\"\n", i, i);
        printf("\"%d-2\" -> \"%d-3\"\n", i, i);
        printf("\"%d-2\" -> \"%d-4\"\n", i, i);
        i+=sizeof(struct vm_tstart_t)-1;
      } break;
      case TEOF: {
        printf("\"%d-1\"[label=\"%d\" shape=square]\n", i, i);
        printf("\"%d-2\"[label=\"EOF\"]\n", i);
        printf("\"%d-1\" -> \"%d-2\"\n", i, i);
        i+=sizeof(struct vm_teof_t)-1;
      } break;
      case VNULL: {
        struct vm_vnull_t *node = (struct vm_vnull_t *)&obj->ast.buffer[i];
        printf("\"%d-1\"[label=\"%d\" shape=square]\n", i, i);
        printf("\"%d-2\"[label=\"NULL\"]\n", i);
        printf("\"%d-2\" -> \"%d-3\"\n", i, i);
        printf("\"%d-3\"[label=\"%d\" shape=diamond]\n", i, node->ret);
        printf("\"%d-1\" -> \"%d-2\"\n", i, i);
        i+=sizeof(struct vm_vnull_t)-1;
      } break;
      case TIF: {
        struct vm_tif_t *node = (struct vm_tif_t *)&obj->ast.buffer[i];
        printf("\"%d-2\"[label=\"IF\"]\n", i);
        printf("\"%d-2\" -> \"%d-4\"\n", i, i);
        if(node->true_ > 0) {
          printf("\"%d-2\" -> \"%d-6\"\n", i, i);
        }
        if(node->false_ > 0) {
          printf("\"%d-2\" -> \"%d-7\"\n", i, i);
        }
        printf("\"%d-1\" -> \"%d-2\"\n", i, i);
        printf("\"%d-2\" -> \"%d-3\"\n", i, i);
        if(node->true_ > 0) {
          printf("\"%d-6\"[label=\"%d\" shape=square]\n", i, node->true_);
        }
        if(node->false_ > 0) {
          printf("\"%d-7\"[label=\"%d\" shape=square]\n", i, node->false_);
        }
        printf("\"%d-1\"[label=\"%d\" shape=square]\n", i, i);
        printf("\"%d-3\"[label=\"%d\" shape=diamond]\n", i, node->ret);
        printf("\"%d-4\"[label=\"%d\" shape=square]\n", i, node->go);
        i+=sizeof(struct vm_tif_t)-1;
      } break;
      case LPAREN: {
        struct vm_lparen_t *node = (struct vm_lparen_t *)&obj->ast.buffer[i];
        printf("\"%d-1\"[label=\"%d\" shape=square]\n", i, i);
        printf("\"%d-2\"[label=\"(\"]\n", i);
        printf("\"%d-2\" -> \"%d-4\"\n", i, i);
        printf("\"%d-2\" -> \"%d-3\"\n", i, i);
        printf("\"%d-3\"[label=\"%d\" shape=diamond]\n", i, node->ret);
        printf("\"%d-4\"[label=\"%d\" shape=square]\n", i, node->go);
        printf("\"%d-1\" -> \"%d-2\"\n", i, i);
        i+=sizeof(struct vm_lparen_t)-1;
      } break;
      case TFALSE:
      case TTRUE: {
        int x = 0;
        struct vm_ttrue_t *node = (struct vm_ttrue_t *)&obj->ast.buffer[i];
        printf("\"%d-1\"[label=\"%d\" shape=square]\n", i, i);
        if((obj->ast.buffer[i]) == TFALSE) {
          printf("\"%d-2\"[label=\"FALSE\"]\n", i);
        } else {
          printf("\"%d-2\"[label=\"TRUE\"]\n", i);
        }
        for(x=0;x<node->nrgo;x++) {
          printf("\"%d-2\" -> \"%d-%d\"\n", i, i, x+3);
          printf("\"%d-%d\"[label=\"%d\" shape=square]\n", i, x+3, node->go[x]);
        }
        printf("\"%d-2\" -> \"%d-%d\"\n", i, i, x+4);
        printf("\"%d-%d\"[label=\"%d\" shape=diamond]\n", i, x+4, node->ret);
        printf("\"%d-1\" -> \"%d-2\"\n", i, i);
        i+=sizeof(struct vm_ttrue_t)+(sizeof(uint16_t)*node->nrgo)-1;
      } break;
      case TFUNCTION: {
        int x = 0;
        struct vm_tfunction_t *node = (struct vm_tfunction_t *)&obj->ast.buffer[i];
        printf("\"%d-1\"[label=\"%d\" shape=square]\n", i, i);
        printf("\"%d-2\"[label=\"%s\"]\n", i, rule_functions[node->token].name);
        for(x=0;x<node->nrgo;x++) {
          printf("\"%d-2\" -> \"%d-%d\"\n", i, i, x+3);
          printf("\"%d-%d\"[label=\"%d\" shape=square]\n", i, x+3, node->go[x]);
        }
        printf("\"%d-2\" -> \"%d-%d\"\n", i, i, x+4);
        printf("\"%d-%d\"[label=\"%d\" shape=diamond]\n", i, x+4, node->ret);
        printf("\"%d-1\" -> \"%d-2\"\n", i, i);
        i+=sizeof(struct vm_tfunction_t)+(sizeof(uint16_t)*node->nrgo)-1;
      } break;
      case TCEVENT: {
        struct vm_tcevent_t *node = (struct vm_tcevent_t *)&obj->ast.buffer[i];
        printf("\"%d-1\"[label=\"%d\" shape=square]\n", i, i);
        printf("\"%d-2\"[label=\"%s()\"]\n", i, node->token);
        printf("\"%d-2\" -> \"%d-3\"\n", i, i);
        printf("\"%d-3\"[label=\"%d\" shape=diamond]\n", i, node->ret);
        printf("\"%d-1\" -> \"%d-2\"\n", i, i);
        i+=sizeof(struct vm_tcevent_t)+strlen((char *)node->token);
      } break;
      case TVAR: {
        struct vm_tvar_t *node = (struct vm_tvar_t *)&obj->ast.buffer[i];
        printf("\"%d-1\"[label=\"%d\" shape=square]\n", i, i);
        printf("\"%d-2\"[label=\"%s\"]\n", i, node->token);
        if(node->go > 0) {
          printf("\"%d-2\" -> \"%d-4\"\n", i, i);
        }
        printf("\"%d-2\" -> \"%d-3\"\n", i, i);
        printf("\"%d-3\"[label=\"%d\" shape=diamond]\n", i, node->ret);
        if(node->go > 0) {
          printf("\"%d-4\"[label=\"%d\" shape=square]\n", i, node->go);
        }
        printf("\"%d-1\" -> \"%d-2\"\n", i, i);
        i+=sizeof(struct vm_tvar_t)+strlen((char *)node->token);
      } break;
      case TEVENT: {
        struct vm_tevent_t *node = (struct vm_tevent_t *)&obj->ast.buffer[i];
        printf("\"%d-1\"[label=\"%d\" shape=square]\n", i, i);
        printf("\"%d-2\"[label=\"%s\"]\n", i, node->token);
        if(node->go > 0) {
          printf("\"%d-2\" -> \"%d-4\"\n", i, i);
        }
        printf("\"%d-2\" -> \"%d-3\"\n", i, i);
        printf("\"%d-3\"[label=\"%d\" shape=diamond]\n", i, node->ret);
        if(node->go > 0) {
          printf("\"%d-4\"[label=\"%d\" shape=square]\n", i, node->go);
        }
        printf("\"%d-1\" -> \"%d-2\"\n", i, i);
        i+=sizeof(struct vm_tevent_t)+strlen((char *)node->token);
      } break;
      case TNUMBER: {
        struct vm_tnumber_t *node = (struct vm_tnumber_t *)&obj->ast.buffer[i];
        printf("\"%d-1\"[label=\"%d\" shape=square]\n", i, i);
        printf("\"%d-2\"[label=\"%s\"]\n", i, node->token);
        printf("\"%d-1\" -> \"%d-2\"\n", i, i);
        printf("\"%d-3\"[label=\"%d\" shape=diamond]\n", i, node->ret);
        printf("\"%d-2\" -> \"%d-3\"\n", i, i);
        i+=sizeof(struct vm_tnumber_t)+strlen((char *)node->token)-1;
      } break;
      case VINTEGER: {
        struct vm_vinteger_t *node = (struct vm_vinteger_t *)&obj->ast.buffer[i];
        printf("\"%d-1\"[label=\"%d\" shape=square]\n", i, i);
        printf("\"%d-2\"[label=\"%d\"]\n", i, node->value);
        printf("\"%d-1\" -> \"%d-2\"\n", i, i);
        printf("\"%d-3\"[label=\"%d\" shape=diamond]\n", i, node->ret);
        printf("\"%d-2\" -> \"%d-3\"\n", i, i);
        i+=sizeof(struct vm_vinteger_t)-1;
      } break;
      case VFLOAT: {
        struct vm_vfloat_t *node = (struct vm_vfloat_t *)&obj->ast.buffer[i];
        printf("\"%d-1\"[label=\"%d\" shape=square]\n", i, i);
        printf("\"%d-2\"[label=\"%g\"]\n", i, node->value);
        printf("\"%d-1\" -> \"%d-2\"\n", i, i);
        printf("\"%d-3\"[label=\"%d\" shape=diamond]\n", i, node->ret);
        printf("\"%d-2\" -> \"%d-3\"\n", i, i);
        i+=sizeof(struct vm_vfloat_t)-1;
      } break;
      case TOPERATOR: {
        struct vm_toperator_t *node = (struct vm_toperator_t *)&obj->ast.buffer[i];
        printf("\"%d-1\"[label=\"%d\" shape=square]\n", i, i);
        printf("\"%d-3\"[label=\"%d\" shape=square]\n", i, node->left);
        printf("\"%d-4\"[label=\"%d\" shape=square]\n", i, node->right);
        printf("\"%d-2\"[label=\"%s\"]\n", i, rule_operators[node->token].name);
        printf("\"%d-1\" -> \"%d-2\"\n", i, i);
        printf("\"%d-5\"[label=\"%d\" shape=diamond]\n", i, node->ret);
        printf("\"%d-2\" -> \"%d-3\"\n", i, i);
        printf("\"%d-2\" -> \"%d-4\"\n", i, i);
        printf("\"%d-2\" -> \"%d-5\"\n", i, i);
        i+=sizeof(struct vm_toperator_t)-1;
      } break;
      default: {
      } break;
    }
  }
}

static void print_tree(struct rules_t *obj) {
  print_steps(obj);
  print_ast(obj);
}
#endif
/*LCOV_EXCL_STOP*/

static int vm_value_set(struct rules_t *obj, int step, int ret) {
  int out = obj->varstack.nrbytes;

#ifdef DEBUG
  printf("%s %d %d\n", __FUNCTION__, __LINE__, out);
#endif
  switch(obj->ast.buffer[step]) {
    case TNUMBER: {
      float var = 0;
      struct vm_tnumber_t *node = (struct vm_tnumber_t *)&obj->ast.buffer[step];
      var = atof((char *)node->token);

      unsigned int size = alignedbytes(obj->varstack.nrbytes+sizeof(struct vm_vinteger_t));

      struct vm_vinteger_t *value = (struct vm_vinteger_t *)&obj->varstack.buffer[obj->varstack.nrbytes];
      value->type = VINTEGER;
      value->ret = ret;
      value->value = (int)var;
      obj->varstack.nrbytes = size;
      obj->varstack.bufsize = MAX(obj->varstack.bufsize, alignedvarstack(obj->varstack.nrbytes));
#ifdef DEBUG
      printf("%s %d %d %d\n", __FUNCTION__, __LINE__, out, (int)var);
#endif
    } break;
    case VFLOAT: {
      unsigned int size = alignedbytes(obj->varstack.nrbytes+sizeof(struct vm_vfloat_t));

      struct vm_vfloat_t *value = (struct vm_vfloat_t *)&obj->varstack.buffer[obj->varstack.nrbytes];
      struct vm_vfloat_t *cpy = (struct vm_vfloat_t *)&obj->ast.buffer[step];
      value->type = VFLOAT;
      value->ret = ret;
      value->value = cpy->value;
      obj->varstack.nrbytes = size;
      obj->varstack.bufsize = MAX(obj->varstack.bufsize, alignedvarstack(obj->varstack.nrbytes));
#ifdef DEBUG
      printf("%s %d %d %g\n", __FUNCTION__, __LINE__, out, cpy->value);
#endif
    } break;
    case VINTEGER: {
      unsigned int size = alignedbytes(obj->varstack.nrbytes+sizeof(struct vm_vinteger_t));

      struct vm_vinteger_t *value = (struct vm_vinteger_t *)&obj->varstack.buffer[obj->varstack.nrbytes];
      struct vm_vinteger_t *cpy = (struct vm_vinteger_t *)&obj->ast.buffer[step];
      value->type = VINTEGER;
      value->ret = ret;
      value->value = cpy->value;
      obj->varstack.nrbytes = size;
      obj->varstack.bufsize = MAX(obj->varstack.bufsize, alignedvarstack(obj->varstack.nrbytes));
#ifdef DEBUG
      printf("%s %d %d %d\n", __FUNCTION__, __LINE__, out, cpy->value);
#endif
    } break;
    case VNULL: {
      unsigned int size = alignedbytes(obj->varstack.nrbytes+sizeof(struct vm_vnull_t));

      struct vm_vnull_t *value = (struct vm_vnull_t *)&obj->varstack.buffer[obj->varstack.nrbytes];
      value->type = VNULL;
      value->ret = ret;
      obj->varstack.nrbytes = size;
      obj->varstack.bufsize = MAX(obj->varstack.bufsize, alignedvarstack(obj->varstack.nrbytes));
#ifdef DEBUG
      printf("%s %d %d NULL\n", __FUNCTION__, __LINE__, out);
#endif
    } break;
    /* LCOV_EXCL_START*/
    default: {
      logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
      return -1;
    } break;
    /* LCOV_EXCL_STOP*/
  }

  return out;
}

static int vm_value_upd_pos(struct rules_t *obj, int val, int step) {
  switch(obj->ast.buffer[step]) {
    case TOPERATOR: {
      struct vm_toperator_t *node = (struct vm_toperator_t *)&obj->ast.buffer[step];
      node->value = val;
    } break;
    case TVAR: {
      struct vm_tvar_t *node = (struct vm_tvar_t *)&obj->ast.buffer[step];
      node->value = val;
    } break;
    case TFUNCTION: {
      struct vm_tfunction_t *node = (struct vm_tfunction_t *)&obj->ast.buffer[step];
      node->value = val;
    } break;
    case VNULL: {
    } break;
    /* LCOV_EXCL_START*/
    default: {
      logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
      return -1;
    } break;
    /* LCOV_EXCL_STOP*/
  }
  return 0;
}

static int vm_value_clone(struct rules_t *obj, unsigned char *val) {
  int ret = obj->varstack.nrbytes;

  switch(val[0]) {
    case VINTEGER: {
      unsigned int size = alignedbytes(obj->varstack.nrbytes+sizeof(struct vm_vinteger_t));

      struct vm_vinteger_t *cpy = (struct vm_vinteger_t *)&val[0];
      struct vm_vinteger_t *value = (struct vm_vinteger_t *)&obj->varstack.buffer[obj->varstack.nrbytes];
      value->type = VINTEGER;
      value->ret = 0;
      value->value = (int)cpy->value;
      obj->varstack.nrbytes = size;
      obj->varstack.bufsize = MAX(obj->varstack.bufsize, alignedvarstack(obj->varstack.nrbytes));
    } break;
    case VFLOAT: {
      unsigned int size = alignedbytes(obj->varstack.nrbytes+sizeof(struct vm_vfloat_t));

      struct vm_vfloat_t *cpy = (struct vm_vfloat_t *)&val[0];
      struct vm_vfloat_t *value = (struct vm_vfloat_t *)&obj->varstack.buffer[obj->varstack.nrbytes];
      value->type = VFLOAT;
      value->ret = 0;
      value->value = cpy->value;
      obj->varstack.nrbytes = size;
      obj->varstack.bufsize = MAX(obj->varstack.bufsize, alignedvarstack(obj->varstack.nrbytes));
    } break;
    case VNULL: {
      unsigned int size = alignedbytes(obj->varstack.nrbytes+sizeof(struct vm_vnull_t));

      struct vm_vnull_t *value = (struct vm_vnull_t *)&obj->varstack.buffer[obj->varstack.nrbytes];
      value->type = VNULL;
      value->ret = 0;
      obj->varstack.nrbytes = size;
      obj->varstack.bufsize = MAX(obj->varstack.bufsize, alignedvarstack(obj->varstack.nrbytes));
    } break;
    /* LCOV_EXCL_START*/
    default: {
      logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
      return -1;
    } break;
    /* LCOV_EXCL_STOP*/
  }
  return ret;
}

static int vm_value_del(struct rules_t *obj, unsigned int idx) {
  int x = 0, ret = 0;

  if(idx == obj->varstack.nrbytes) {
    return -1;
  }

#ifdef DEBUG
  printf("%s %d %d\n", __FUNCTION__, __LINE__, idx);
#endif
  switch(obj->varstack.buffer[idx]) {
    case VINTEGER: {
      ret = alignedbytes(sizeof(struct vm_vinteger_t));
      memmove(&obj->varstack.buffer[idx], &obj->varstack.buffer[idx+ret], obj->varstack.nrbytes-idx-ret);

      obj->varstack.nrbytes -= ret;
      obj->varstack.bufsize = MAX(obj->varstack.bufsize, alignedvarstack(obj->varstack.nrbytes));
    } break;
    case VFLOAT: {
      ret = alignedbytes(sizeof(struct vm_vfloat_t));
      memmove(&obj->varstack.buffer[idx], &obj->varstack.buffer[idx+ret], obj->varstack.nrbytes-idx-ret);

      obj->varstack.nrbytes -= ret;
      obj->varstack.bufsize = MAX(obj->varstack.bufsize, alignedvarstack(obj->varstack.nrbytes));
    } break;
    case VNULL: {
      ret = alignedbytes(sizeof(struct vm_vnull_t));
      memmove(&obj->varstack.buffer[idx], &obj->varstack.buffer[idx+ret], obj->varstack.nrbytes-idx-ret);

      obj->varstack.nrbytes -= ret;
      obj->varstack.bufsize = MAX(obj->varstack.bufsize, alignedvarstack(obj->varstack.nrbytes));
    } break;
    /* LCOV_EXCL_START*/
    default: {
      logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
      return -1;
    } break;
    /* LCOV_EXCL_STOP*/
  }

  /*
   * Values are linked back to their root node,
   * by their absolute position in the bytecode.
   * If a value is deleted, these positions changes,
   * so we need to update all nodes.
   */
  for(x=idx;alignedbytes(x)<obj->varstack.nrbytes;x++) {
    x = alignedbytes(x);
#ifdef DEBUG
    printf("%s %d %d\n", __FUNCTION__, __LINE__, x);
#endif
    switch(obj->varstack.buffer[x]) {
      case VINTEGER: {
        struct vm_vinteger_t *node = (struct vm_vinteger_t *)&obj->varstack.buffer[x];
        if(node->ret > 0) {
          vm_value_upd_pos(obj, x, node->ret);
        }
        x += sizeof(struct vm_vinteger_t)-1;
      } break;
      case VFLOAT: {
        struct vm_vfloat_t *node = (struct vm_vfloat_t *)&obj->varstack.buffer[x];
        if(node->ret > 0) {
          vm_value_upd_pos(obj, x, node->ret);
        }
        x += sizeof(struct vm_vfloat_t)-1;
      } break;
      case VNULL: {
        struct vm_vnull_t *node = (struct vm_vnull_t *)&obj->varstack.buffer[x];
        if(node->ret > 0) {
          vm_value_upd_pos(obj, x, node->ret);
        }
        x += sizeof(struct vm_vnull_t)-1;
      } break;
      /* LCOV_EXCL_START*/
      default: {
        logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
        return -1;
      } break;
      /* LCOV_EXCL_STOP*/
    }
  }

  return ret;
}

/*LCOV_EXCL_START*/
void valprint(struct rules_t *obj, char *out, int size) {
  int x = 0, pos = 0;
  memset(out, 0, size);
  /*
   * This is only used for debugging purposes
   */
  for(x=4;alignedbytes(x)<obj->varstack.nrbytes;x++) {
    if(alignedbytes(x) < obj->varstack.nrbytes) {
      x = alignedbytes(x);
      switch(obj->varstack.buffer[x]) {
        case VINTEGER: {
          struct vm_vinteger_t *val = (struct vm_vinteger_t *)&obj->varstack.buffer[x];
          switch(obj->ast.buffer[val->ret]) {
            case TVAR: {
              struct vm_tvar_t *node = (struct vm_tvar_t *)&obj->ast.buffer[val->ret];
              pos += snprintf(&out[pos], size - pos, "%s = %d", node->token, val->value);
            } break;
            case TFUNCTION: {
              struct vm_tfunction_t *node = (struct vm_tfunction_t *)&obj->ast.buffer[val->ret];
              pos += snprintf(&out[pos], size - pos, "%s = %d", rule_functions[node->token].name, val->value);
            } break;
            case TOPERATOR: {
              struct vm_toperator_t *node = (struct vm_toperator_t *)&obj->ast.buffer[val->ret];
              pos += snprintf(&out[pos], size - pos, "%s = %d", rule_operators[node->token].name, val->value);
            } break;
            default: {
              // printf("err: %s %d %d\n", __FUNCTION__, __LINE__, obj->ast.buffer[val->ret]);
              // exit(-1);
            } break;
          }
          x += sizeof(struct vm_vinteger_t)-1;
        } break;
        case VFLOAT: {
          struct vm_vfloat_t *val = (struct vm_vfloat_t *)&obj->varstack.buffer[x];
          switch(obj->ast.buffer[val->ret]) {
            case TVAR: {
              struct vm_tvar_t *node = (struct vm_tvar_t *)&obj->ast.buffer[val->ret];
              pos += snprintf(&out[pos], size - pos, "%s = %g", node->token, val->value);
            } break;
            case TFUNCTION: {
              struct vm_tfunction_t *node = (struct vm_tfunction_t *)&obj->ast.buffer[val->ret];
              pos += snprintf(&out[pos], size - pos, "%s = %g", rule_functions[node->token].name, val->value);
            } break;
            case TOPERATOR: {
              struct vm_toperator_t *node = (struct vm_toperator_t *)&obj->ast.buffer[val->ret];
              pos += snprintf(&out[pos], size - pos, "%s = %g", rule_operators[node->token].name, val->value);
            } break;
            default: {
              // printf("err: %s %d\n", __FUNCTION__, __LINE__);
              // exit(-1);
            } break;
          }
          x += sizeof(struct vm_vfloat_t)-1;
        } break;
        case VNULL: {
          struct vm_vnull_t *val = (struct vm_vnull_t *)&obj->varstack.buffer[x];
          switch(obj->ast.buffer[val->ret]) {
            case TVAR: {
              struct vm_tvar_t *node = (struct vm_tvar_t *)&obj->ast.buffer[val->ret];
              pos += snprintf(&out[pos], size - pos, "%s = NULL", node->token);
            } break;
            case TFUNCTION: {
              struct vm_tfunction_t *node = (struct vm_tfunction_t *)&obj->ast.buffer[val->ret];
              pos += snprintf(&out[pos], size - pos, "%s = NULL", rule_functions[node->token].name);
            } break;
            case TOPERATOR: {
              struct vm_toperator_t *node = (struct vm_toperator_t *)&obj->ast.buffer[val->ret];
              pos += snprintf(&out[pos], size - pos, "%s = NULL", rule_operators[node->token].name);
            } break;
            default: {
              // printf("err: %s %d\n", __FUNCTION__, __LINE__);
              // exit(-1);
            } break;
          }
          x += sizeof(struct vm_vnull_t)-1;
        } break;
        default: {
          logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
        } break;
      }
    }
  }

  if(rule_options.prt_token_val_cb != NULL) {
    rule_options.prt_token_val_cb(obj, out, size);
  }
}
/*LCOV_EXCL_START*/

static void vm_clear_values(struct rules_t *obj) {
  int i = 0;
  for(i=0;alignedbytes(i)<obj->ast.nrbytes;i++) {

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
        i+=sizeof(struct vm_ttrue_t)+(sizeof(uint16_t)*node->nrgo)-1;
      } break;
      case TFUNCTION: {
        struct vm_tfunction_t *node = (struct vm_tfunction_t *)&obj->ast.buffer[i];
        node->value = 0;
        i+=sizeof(struct vm_tfunction_t)+(sizeof(uint16_t)*node->nrgo)-1;
      } break;
      case TCEVENT: {
        struct vm_tcevent_t *node = (struct vm_tcevent_t *)&obj->ast.buffer[i];
        i+=sizeof(struct vm_tcevent_t)+strlen((char *)node->token);
      } break;
      case TVAR: {
        struct vm_tvar_t *node = (struct vm_tvar_t *)&obj->ast.buffer[i];
        if(rule_options.clr_token_val_cb != NULL) {
          rule_options.clr_token_val_cb(obj, i);
        } else {
          node->value = 0;
        }
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
        i+=sizeof(struct vm_vinteger_t)-1;
      } break;
      case VFLOAT: {
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

int rule_run(struct rules_t *obj, int validate) {
#ifdef DEBUG
  printf("----------\n");
  printf("%s %d\n", __FUNCTION__, obj->nr);
  printf("----------\n");
#endif

  int go = 0, ret = -1, i = -1, start = -1;
  go = start = 0;

  while(go != -1) {
#ifdef ESP8266
    ESP.wdtFeed();
#endif

/*LCOV_EXCL_START*/
#ifdef DEBUG
    printf("goto: %d, ret: %d, bytes: %d\n", go, ret, obj->ast.nrbytes);
    printf("AST stack is   %d bytes, local stack is   %d bytes\n", obj->ast.nrbytes, obj->varstack.nrbytes);
    printf("AST bufsize is %d bytes, local bufsize is %d bytes\n", obj->ast.bufsize, obj->varstack.bufsize);

    {
      char out[1024];
      valprint(obj, (char *)&out, 1024);
      printf("%s\n", out);
    }
#endif
/*LCOV_EXCL_STOP*/

    switch(obj->ast.buffer[go]) {
      case TSTART: {
        struct vm_tstart_t *node = (struct vm_tstart_t *)&obj->ast.buffer[go];
        if(ret > -1) {
          go = -1;
        } else {
          if(obj->cont.go > 0) {
            go = obj->cont.go;
            ret = obj->cont.ret;
            obj->cont.go = 0;
            obj->cont.ret = 0;
          } else {
            vm_clear_values(obj);
            go = node->go;
          }
        }
      } break;
      case TEVENT: {
        struct vm_tevent_t *node = (struct vm_tevent_t *)&obj->ast.buffer[go];
        if(node->go == ret) {
          ret = go;
          go = node->ret;
        } else {
          go = node->go;
          ret = go;
        }
      } break;
      case TIF: {
        struct vm_tif_t *node = (struct vm_tif_t *)&obj->ast.buffer[go];
        int val = -1;
        if(ret > -1) {
          switch(obj->ast.buffer[ret]) {
            case TOPERATOR: {
              struct vm_toperator_t *op = (struct vm_toperator_t *)&obj->ast.buffer[ret];
              struct vm_vinteger_t *tmp = (struct vm_vinteger_t *)&obj->varstack.buffer[op->value];

              val = tmp->value;
              vm_value_del(obj, op->value);

              /*
               * Reassign node due to various (unsigned char *)REALLOC's
               */
              node = (struct vm_tif_t *)&obj->ast.buffer[go];
            } break;
            case LPAREN: {
              struct vm_lparen_t *op = (struct vm_lparen_t *)&obj->ast.buffer[ret];
              struct vm_vinteger_t *tmp = (struct vm_vinteger_t *)&obj->varstack.buffer[op->value];

              val = tmp->value;
              vm_value_del(obj, op->value);

              /*
               * Reassign node due to various (unsigned char *)REALLOC's
               */
              node = (struct vm_tif_t *)&obj->ast.buffer[go];
            } break;
            case TTRUE:
            case TFALSE:
            case TIF:
            case TEVENT:
            break;
            /* LCOV_EXCL_START*/
            default: {
              logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
              return -1;
            } break;
            /* LCOV_EXCL_STOP*/
          }
        }

        if(node->false_ == ret && node->false_ > 0) {
          ret = go;
          go = node->ret;
        } else if(node->true_ == ret) {
          if(node->false_ != 0 && (val == 0 || validate == 1)) {
            go = node->false_;
            ret = go;
          } else {
            ret = go;
            go = node->ret;
          }
        } else if(node->go == ret) {
          if(node->false_ != 0 && val == 0 && validate == 0) {
            go = node->false_;
            ret = go;
          } else if(val == 1 || validate == 1) {
            ret = go;
            go = node->true_;
          } else {
            ret = go;
            go = node->ret;
          }
        } else {
          go = node->go;
        }
      } break;
      case TCEVENT: {
        struct vm_tcevent_t *node = (struct vm_tcevent_t *)&obj->ast.buffer[go];

        if(rule_options.event_cb == NULL) {
          /* LCOV_EXCL_START*/
          logprintf_P(F("FATAL: No 'event_cb' set to handle events"));
          return -1;
          /* LCOV_EXCL_STOP*/
        }

        obj->cont.ret = go;
        obj->cont.go = node->ret;

        /*
         * Tail recursive
         */
        return rule_options.event_cb(obj, (char *)node->token);
      } break;
      case TFUNCTION: {
        struct vm_tfunction_t *node = (struct vm_tfunction_t *)&obj->ast.buffer[go];

        int match = 0, tmp = go;

        for(i=0;i<node->nrgo;i++) {
          if(node->go[i] == ret) {
            match = 1;
            if(i+1 < node->nrgo) {
              switch(obj->ast.buffer[node->go[i+1]]) {
                case TNUMBER:
                case VINTEGER:
                case VFLOAT: {
                  ret = go;
                  go = node->go[i+1];
                } break;
                case VNULL: {
                  ret = go;
                  go = node->go[i+1];
                } break;
                case LPAREN: {
                  ret = go;
                  go = node->go[i+1];
                } break;
                case TOPERATOR: {
                  ret = go;
                  go = node->go[i+1];
                } break;
                /* LCOV_EXCL_START*/
                default: {
                  logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                  return -1;
                } break;
                /* LCOV_EXCL_STOP*/
              }
            } else {
              go = 0;
            }
            break;
          }
        }
        if(match == 0) {
          ret = go;
          go = node->go[0];
        }

        if(go == 0) {
          go = tmp;
          unsigned int idx = node->token, i = 0, shift = 0;
          int c = 0;
          uint16_t values[node->nrgo];
          memset(&values, 0, node->nrgo);

          /* LCOV_EXCL_START*/
          if(idx > nr_rule_functions) {
            logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
            return -1;
          }
          /* LCOV_EXCL_STOP*/

          for(i=0;i<node->nrgo;i++) {
            switch(obj->ast.buffer[node->go[i]]) {
              case TNUMBER:
              case VINTEGER:
              case VFLOAT: {
                values[i] = vm_value_set(obj, node->go[i], 0);

                /*
                 * Reassign node due to possible reallocs
                 */
                node = (struct vm_tfunction_t *)&obj->ast.buffer[go];
              } break;
              case LPAREN: {
                struct vm_lparen_t *tmp = (struct vm_lparen_t *)&obj->ast.buffer[node->go[i]];
                struct vm_tgeneric_t *val = (struct vm_tgeneric_t *)&obj->varstack.buffer[tmp->value];
                values[i] = tmp->value;
                tmp->value = 0;
                val->ret = 0;
              } break;
              case TOPERATOR: {
                struct vm_toperator_t *tmp = (struct vm_toperator_t *)&obj->ast.buffer[node->go[i]];
                struct vm_tgeneric_t *val = (struct vm_tgeneric_t *)&obj->varstack.buffer[tmp->value];
                values[i] = tmp->value;
                tmp->value = 0;
                val->ret = 0;
              } break;
              case TFUNCTION: {
                struct vm_tfunction_t *tmp = (struct vm_tfunction_t *)&obj->ast.buffer[node->go[i]];
                struct vm_tgeneric_t *val = (struct vm_tgeneric_t *)&obj->varstack.buffer[tmp->value];
                values[i] = tmp->value;
                tmp->value = 0;
                val->ret = 0;
              } break;
              case VNULL: {
                values[i] = vm_value_set(obj, node->go[i], node->go[i]);
                /*
                * Reassign node due to possible reallocs
                */
                node = (struct vm_tfunction_t *)&obj->ast.buffer[go];
              } break;
              case TVAR: {
                if(rule_options.get_token_val_cb != NULL && rule_options.cpy_token_val_cb != NULL) {
                  rule_options.cpy_token_val_cb(obj, node->go[i]); // TESTME
                  unsigned char *val = rule_options.get_token_val_cb(obj, node->go[i]);
                  /* LCOV_EXCL_START*/
                  if(val == NULL) {
                    logprintf_P(F("FATAL: 'get_token_val_cb' did not return a value"));
                    return -1;
                  }
                  /* LCOV_EXCL_STOP*/
                  values[i] = vm_value_clone(obj, val);

                  /*
                   * Reassign node due to possible reallocs
                   */
                  node = (struct vm_tfunction_t *)&obj->ast.buffer[go];
                } else {
                  /* LCOV_EXCL_START*/
                  logprintf_P(F("FATAL: No '[get|cpy]_token_val_cb' set to handle variables"));
                  return -1;
                  /* LCOV_EXCL_STOP*/
                }
              } break;
              /* LCOV_EXCL_START*/
              default: {
                logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                return -1;
              } break;
              /* LCOV_EXCL_STOP*/
            }
          }

          if(rule_functions[idx].callback(obj, node->nrgo, values, &c) != 0) {
            /* LCOV_EXCL_START*/
            logprintf_P(F("FATAL: function call '%s' failed"), rule_functions[idx].name);
            return -1;
            /* LCOV_EXCL_STOP*/
          }

          /*
           * Reassign node due to possible reallocs
           */
          node = (struct vm_tfunction_t *)&obj->ast.buffer[go];
          if(c > 0) {
            switch(obj->varstack.buffer[c]) {
              case VINTEGER: {
                struct vm_vinteger_t *tmp = (struct vm_vinteger_t *)&obj->varstack.buffer[c];
                tmp->ret = go;
                node->value = c;
              } break;
              case VNULL: {
                struct vm_vnull_t *tmp = (struct vm_vnull_t *)&obj->varstack.buffer[c];
                tmp->ret = go;
                node->value = c;
              } break;
              case VFLOAT: {
                struct vm_vfloat_t *tmp = (struct vm_vfloat_t *)&obj->varstack.buffer[c];
                tmp->ret = go;
                node->value = c;
              } break;
              /* LCOV_EXCL_START*/
              default: {
                logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                return -1;
              } break;
              /* LCOV_EXCL_STOP*/
            }
          } else {
            node->value = 0;
          }

          for(i=0;i<node->nrgo;i++) {
            switch(obj->varstack.buffer[values[i] - shift]) {
              case VFLOAT:
              case VNULL:
              case VINTEGER: {
                shift += vm_value_del(obj, values[i] - shift);

                /*
                 * Reassign node due to possible reallocs
                 */
                node = (struct vm_tfunction_t *)&obj->ast.buffer[go];
              } break;
              /* LCOV_EXCL_START*/
              default: {
                logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                return -1;
              } break;
              /* LCOV_EXCL_STOP*/
            }
          }

          ret = go;
          go = node->ret;
        }
      } break;
      case TOPERATOR: {
        struct vm_toperator_t *node = (struct vm_toperator_t *)&obj->ast.buffer[go];

        if(node->right == ret ||
            (
              (
                obj->ast.buffer[node->left] == VFLOAT ||
                obj->ast.buffer[node->left] == VINTEGER ||
                obj->ast.buffer[node->left] == TNUMBER
              ) &&
              (
                obj->ast.buffer[node->right] == VFLOAT ||
                obj->ast.buffer[node->right] == VINTEGER ||
                obj->ast.buffer[node->right] == TNUMBER
              )
            )
           ) {
          int a = 0, b = 0, c = 0, step = 0;
          step = node->left;

          switch(obj->ast.buffer[step]) {
            case TNUMBER:
            case VFLOAT:
            case VINTEGER: {
              a = vm_value_set(obj, step, step);
              /*
               * Reassign node due to possible reallocs
               */
              node = (struct vm_toperator_t *)&obj->ast.buffer[go];
            } break;
            case TOPERATOR: {
              struct vm_toperator_t *tmp = (struct vm_toperator_t *)&obj->ast.buffer[step];
              a = tmp->value;
              tmp->value = 0;
            } break;
            case LPAREN: {
              struct vm_lparen_t *tmp = (struct vm_lparen_t *)&obj->ast.buffer[step];
              a = tmp->value;
              tmp->value = 0;
            } break;
            case TFUNCTION: {
              struct vm_tfunction_t *tmp = (struct vm_tfunction_t *)&obj->ast.buffer[step];
              a = tmp->value;
              tmp->value = 0;
            } break;
            case VNULL: {
              a = vm_value_set(obj, step, step);
              /*
               * Reassign node due to possible reallocs
               */
              node = (struct vm_toperator_t *)&obj->ast.buffer[go];
            } break;
            case TVAR: {
              /*
               * If vars are seperate steps
               */
              // struct vm_tvar_t *tmp = (struct vm_tvar_t *)&obj->ast.buffer[step];
              // a = tmp->value;

              if(rule_options.get_token_val_cb != NULL && rule_options.cpy_token_val_cb != NULL) {
                rule_options.cpy_token_val_cb(obj, step); // TESTME
                unsigned char *val = rule_options.get_token_val_cb(obj, step);

                /* LCOV_EXCL_START*/
                if(val == NULL) {
                  logprintf_P(F("FATAL: 'get_token_val_cb' did not return a value"));
                  return -1;
                }
                /* LCOV_EXCL_STOP*/
                a = vm_value_clone(obj, val);
                /*
                 * Reassign node due to possible reallocs
                 */
                node = (struct vm_toperator_t *)&obj->ast.buffer[go];
              } else {
                /* LCOV_EXCL_START*/
                logprintf_P(F("FATAL: No '[get|cpy]_token_val_cb' set to handle variables"));
                return -1;
                /* LCOV_EXCL_STOP*/
              }
            } break;
            /* LCOV_EXCL_START*/
            default: {
              logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
              return -1;
            } break;
            /* LCOV_EXCL_STOP*/
          }

          step = node->right;

          switch(obj->ast.buffer[step]) {
            case TNUMBER:
            case VFLOAT:
            case VINTEGER: {
              b = vm_value_set(obj, step, step);
              /*
               * Reassign node due to possible reallocs
               */
              node = (struct vm_toperator_t *)&obj->ast.buffer[go];
            } break;
            case TOPERATOR: {
              struct vm_toperator_t *tmp = (struct vm_toperator_t *)&obj->ast.buffer[step];
              b = tmp->value;
              tmp->value = 0;
            } break;
            case LPAREN: {
              struct vm_lparen_t *tmp = (struct vm_lparen_t *)&obj->ast.buffer[step];
              b = tmp->value;
              tmp->value = 0;
            } break;
            case TFUNCTION: {
              struct vm_tfunction_t *tmp = (struct vm_tfunction_t *)&obj->ast.buffer[step];
              b = tmp->value;
              tmp->value = 0;
            } break;
            case VNULL: {
              b = vm_value_set(obj, step, step);
              /*
               * Reassign node due to possible reallocs
               */
              node = (struct vm_toperator_t *)&obj->ast.buffer[go];
            } break;
            case TVAR: {
              /*
               * If vars are seperate steps
               */
              if(rule_options.get_token_val_cb != NULL && rule_options.cpy_token_val_cb != NULL) {
                rule_options.cpy_token_val_cb(obj, step); // TESTME
                unsigned char *val = rule_options.get_token_val_cb(obj, step);

                /* LCOV_EXCL_START*/
                if(val == NULL) {
                  logprintf_P(F("FATAL: 'get_token_val_cb' did not return a value"));
                  return -1;
                }
                /* LCOV_EXCL_STOP*/
                b = vm_value_clone(obj, val);
                /*
                 * Reassign node due to possible reallocs
                 */
                node = (struct vm_toperator_t *)&obj->ast.buffer[go];
              } else {
                /* LCOV_EXCL_START*/
                logprintf_P(F("FATAL: No '[get|cpy]_token_val_cb' set to handle variables"));
                return -1;
                /* LCOV_EXCL_STOP*/
              }
            } break;
            /* LCOV_EXCL_START*/
            default: {
              logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
              return -1;
            } break;
            /* LCOV_EXCL_STOP*/
          }
          unsigned int idx = node->token;

          /* LCOV_EXCL_START*/
          if(idx > nr_rule_operators) {
            logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
            return -1;
          }
          /* LCOV_EXCL_STOP*/

          if(rule_operators[idx].callback(obj, a, b, &c) != 0) {
            /* LCOV_EXCL_START*/
            logprintf_P(F("FATAL: operator call '%s' failed"), rule_operators[idx].name);
            return -1;
            /* LCOV_EXCL_STOP*/
          }

          /*
           * Reassign node due to possible (unsigned char *)REALLOC's
           * in the callbacks
           */
          node = (struct vm_toperator_t *)&obj->ast.buffer[go];

          switch(obj->varstack.buffer[c]) {
            case VINTEGER: {
              struct vm_vinteger_t *tmp = (struct vm_vinteger_t *)&obj->varstack.buffer[c];
              /*
               * Reassign node due to possible reallocs
               */
              node = (struct vm_toperator_t *)&obj->ast.buffer[go];
              tmp->ret = go;
              node->value = c;
            } break;
            case VFLOAT: {
              struct vm_vfloat_t *tmp = (struct vm_vfloat_t *)&obj->varstack.buffer[c];
              /*
               * Reassign node due to possible reallocs
               */
              node = (struct vm_toperator_t *)&obj->ast.buffer[go];
              tmp->ret = go;
              node->value = c;
            } break;
            case VNULL: {
              struct vm_vnull_t *tmp = (struct vm_vnull_t *)&obj->varstack.buffer[c];
              /*
               * Reassign node due to possible reallocs
               */
              node = (struct vm_toperator_t *)&obj->ast.buffer[go];
              tmp->ret = go;
              node->value = c;
            } break;
            /* LCOV_EXCL_START*/
            default: {
              logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
              return -1;
            } break;
            /* LCOV_EXCL_STOP*/
          }

          vm_value_del(obj, MAX(a, b));

          /*
           * Reassign node due to possible reallocs
           */
          node = (struct vm_toperator_t *)&obj->ast.buffer[go];

          vm_value_del(obj, MIN(a, b));

          /*
           * Reassign node due to possible reallocs
           */
          node = (struct vm_toperator_t *)&obj->ast.buffer[go];

          ret = go;
          go = node->ret;
        } else if(node->left == ret/* || node->left < obj->pos.parsed*/) {
          ret = go;
          go = node->right;
        } else {
          ret = go;
          go = node->left;
        }
      } break;
      case TNUMBER:
      case VFLOAT:
      case VINTEGER: {
        int tmp = ret;
        ret = go;
        go = tmp;
      } break;
      case TSTRING: {
        int tmp = ret;
        ret = go;
        go = tmp;
      } break;
      case VNULL: {
        int tmp = ret;
        ret = go;
        go = tmp;
      } break;
      case LPAREN: {
        struct vm_lparen_t *node = (struct vm_lparen_t *)&obj->ast.buffer[go];
        if(node->go == ret) {
          switch(obj->ast.buffer[node->go]) {
            case TOPERATOR: {
              struct vm_toperator_t *tmp = (struct vm_toperator_t *)&obj->ast.buffer[ret];
              node->value = tmp->value;
              tmp->value = 0;
            } break;
            case LPAREN: {
              struct vm_lparen_t *tmp = (struct vm_lparen_t *)&obj->ast.buffer[ret];
              node->value = tmp->value;
              tmp->value = 0;
            } break;
            /* LCOV_EXCL_START*/
            default: {
              logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
              return -1;
            } break;
            /* LCOV_EXCL_STOP*/
          }
          ret = go;
          go = node->ret;
        } else {
          ret = go;
          go = node->go;
        }
      } break;
      case TFALSE:
      case TTRUE: {
        struct vm_ttrue_t *node = (struct vm_ttrue_t *)&obj->ast.buffer[go];
        switch(obj->ast.buffer[ret]) {
          case TVAR: {
          } break;
          // case TOPERATOR: {
            // struct vm_toperator_t *node = (struct vm_toperator_t *)&obj->ast.buffer[ret];
            // idx = node->value;
            // node->value = 0;
          // } break;
          case TFUNCTION: {
            struct vm_tfunction_t *tmp = (struct vm_tfunction_t *)&obj->ast.buffer[ret];
            if(tmp->value > 0) {
              vm_value_del(obj, tmp->value);
            }
            node = (struct vm_ttrue_t *)&obj->ast.buffer[go];
          } break;
          case TIF:
          case TEVENT:
          case TCEVENT:
          case TTRUE:
          case TFALSE: {
          } break;
          /* LCOV_EXCL_START*/
          default: {
            logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
            return -1;
          } break;
          /* LCOV_EXCL_STOP*/
        }

        // if((obj->ast.buffer[ret]) == TOPERATOR) {
          // vm_value_del(obj, idx);

          // /*
           // * Reassign node due to various (unsigned char *)REALLOC's
           // */
          // node = (struct vm_ttrue_t *)&obj->ast.buffer[go];
        // }

        int match = 0, tmp = go;

        for(i=0;i<node->nrgo;i++) {
          if(node->go[i] == ret) {
            match = 1;
            if(i+1 < node->nrgo) {
              ret = go;
              go = node->go[i+1];
            } else {
              go = 0;
            }
            break;
          }
        }
        if(match == 0) {
          ret = go;
          go = node->go[0];
        }

        if(go == 0) {
          go = node->ret;
          ret = tmp;
        }
      } break;
      case TVAR: {
        struct vm_tvar_t *node = (struct vm_tvar_t *)&obj->ast.buffer[go];

        if(node->go == 0) {
          if(rule_options.cpy_token_val_cb != NULL) {
            rule_options.cpy_token_val_cb(obj, go);
          }

          /*
           * Reassign node due to various (unsigned char *)REALLOC's
           */
          node = (struct vm_tvar_t *)&obj->ast.buffer[go];

          ret = go;
          go = node->ret;
        } else {
          /*
           * When we can find the value in the
           * prepared rule and not as a separate
           * node.
           */
          if(node->go == ret) {
            int idx = 0, shift = 0;

            switch(obj->ast.buffer[node->go]) {
              case TOPERATOR: {
                struct vm_toperator_t *tmp = (struct vm_toperator_t *)&obj->ast.buffer[ret];
                struct vm_tgeneric_t *val = (struct vm_tgeneric_t *)&obj->varstack.buffer[tmp->value];
                idx = tmp->value;
                tmp->value = 0;
                val->ret = go;
              } break;
              case TFUNCTION: {
                struct vm_tfunction_t *tmp = (struct vm_tfunction_t *)&obj->ast.buffer[ret];
                struct vm_tgeneric_t *val = (struct vm_tgeneric_t *)&obj->varstack.buffer[tmp->value];
                idx = tmp->value;
                tmp->value = 0;
                val->ret = go;
              } break;
              case TNUMBER:
              case VFLOAT:
              case VINTEGER: {
                idx = vm_value_set(obj, node->go, go);

                /*
                 * Reassign node due to various (unsigned char *)REALLOC's
                 */
                node = (struct vm_tvar_t *)&obj->ast.buffer[go];
              } break;
              case VNULL: {
                idx = vm_value_set(obj, node->go, go);
                /*
                 * Reassign node due to various (unsigned char *)REALLOC's
                 */
                node = (struct vm_tvar_t *)&obj->ast.buffer[go];
              } break;
              /*
               * Clone variable value to new variable
               */
              case TVAR: {
                if(rule_options.get_token_val_cb != NULL && rule_options.cpy_token_val_cb != NULL) {
                  rule_options.cpy_token_val_cb(obj, ret); // TESTME
                  unsigned char *val = rule_options.get_token_val_cb(obj, ret);
                  /* LCOV_EXCL_START*/
                  if(val == NULL) {
                    logprintf_P(F("FATAL: 'get_token_val_cb' did not return a value"));
                    return -1;
                  }
                  /* LCOV_EXCL_STOP*/
                  idx = vm_value_clone(obj, val);
                } else {
                  /* LCOV_EXCL_START*/
                  logprintf_P(F("FATAL: No '[get|cpy]_token_val_cb' set to handle variables"));
                  return -1;
                  /* LCOV_EXCL_STOP*/
                }
                /*
                 * Reassign node due to possible reallocs
                 */
                node = (struct vm_tvar_t *)&obj->ast.buffer[go];
              } break;
              case LPAREN: {
                struct vm_lparen_t *tmp = (struct vm_lparen_t *)&obj->ast.buffer[ret];
                struct vm_tgeneric_t *val = (struct vm_tgeneric_t *)&obj->varstack.buffer[tmp->value - shift];
                idx = tmp->value - shift;
                tmp->value = 0;
                val->ret = go;
              } break;
              /* LCOV_EXCL_START*/
              default: {
                logprintf_P(F("FATAL: Internal error in %s #%d"), __FUNCTION__, __LINE__);
                return -1;
              } break;
              /* LCOV_EXCL_STOP*/
            }

            if(idx > -1) {
              if(rule_options.set_token_val_cb == NULL) {
                /* LCOV_EXCL_START*/
                logprintf_P(F("FATAL: No 'set_token_val_cb' set to handle variables"));
                return -1;
                /* LCOV_EXCL_STOP*/
              }

              rule_options.set_token_val_cb(obj, go, idx);

              vm_value_del(obj, idx);

              /*
               * Reassign node due to various (unsigned char *)REALLOC's
               */
              node = (struct vm_tvar_t *)&obj->ast.buffer[go];
            } else {
              node->value = 0;
            }

            ret = go;
            go = node->ret;
          } else {
            ret = go;
            go = node->go;
          }
        }
      } break;
    }
  }

  /*
   * Tail recursive
   */
  if(obj->caller > 0) {
    return rule_options.event_cb(obj, NULL);
  }

  return 0;
}

/*LCOV_EXCL_START*/
#ifdef DEBUG
void print_bytecode(struct rules_t *obj) {
  unsigned int i = 0;

  for(i=0;i<obj->ast.nrbytes;i++) {
    i = alignedbytes(i);
    printf("%d", i);
    switch(obj->ast.buffer[i]) {
      case TIF: {
        struct vm_tif_t *node = (struct vm_tif_t *)&obj->ast.buffer[i];
        printf("(TIF)[9][");
        printf("type: %d, ", node->type);
        printf("ret: %d, ", node->ret);
        printf("go: %d, ", node->go);
        printf("true_: %d, ", node->true_);
        printf("false: %d]\n", node->false_);
        i += sizeof(struct vm_tif_t)-1;
      } break;
      case LPAREN: {
        struct vm_lparen_t *node = (struct vm_lparen_t *)&obj->ast.buffer[i];
        printf("(LPAREN)[7][");
        printf("type: %d, ", node->type);
        printf("ret: %d, ", node->ret);
        printf("go: %d, ", node->go);
        printf("value: %d]\n", node->value);
        i += sizeof(struct vm_lparen_t)-1;
      } break;
      case TVAR: {
        struct vm_tvar_t *node = (struct vm_tvar_t *)&obj->ast.buffer[i];
        printf("(TVAR)[%lu][", 7+strlen((char *)node->token)+1);
        printf("type: %d, ", node->type);
        printf("ret: %d, ", node->ret);
        printf("go: %d, ", node->go);
        printf("value: %d, ", node->value);
        printf("token: %s]\n", node->token);
        i += sizeof(struct vm_tvar_t)+strlen((char *)node->token);
      } break;
      case TEVENT: {
        struct vm_tevent_t *node = (struct vm_tevent_t *)&obj->ast.buffer[i];
        printf("(TEVENT)[%lu][", 5+strlen((char *)node->token)+1);
        printf("type: %d, ", node->type);
        printf("ret: %d, ", node->ret);
        printf("go: %d, ", node->go);
        printf("token: %s]\n", node->token);
        i += sizeof(struct vm_tevent_t)+strlen((char *)node->token);
      } break;
      case TCEVENT: {
        struct vm_tcevent_t *node = (struct vm_tcevent_t *)&obj->ast.buffer[i];
        printf("(TCEVENT)[%lu][", 3+strlen((char *)node->token)+1);
        printf("type: %d, ", node->type);
        printf("ret: %d, ", node->ret);
        printf("token: %s]\n", node->token);
        i += sizeof(struct vm_tcevent_t)+strlen((char *)node->token);
      } break;
      case TSTART: {
        struct vm_tstart_t *node = (struct vm_tstart_t *)&obj->ast.buffer[i];
        printf("(TSTART)[5][");
        printf("type: %d, ", node->type);
        printf("ret: %d, ", node->ret);
        printf("go: %d]\n", node->go);
        i += sizeof(struct vm_tstart_t)-1;
      } break;
      /* LCOV_EXCL_START*/
      case TNUMBER: {
        struct vm_tnumber_t *node = (struct vm_tnumber_t *)&obj->ast.buffer[i];
        printf("(TNUMBER)[%lu][", 3+strlen((char *)node->token)+1);
        printf("type: %d, ", node->type);
        printf("ret: %d, ", node->ret);
        printf("token: %s]\n", node->token);
        i += sizeof(struct vm_tnumber_t)+strlen((char *)node->token);
      } break;
      /* LCOV_EXCL_STOP*/
      case VINTEGER: {
        struct vm_vinteger_t *node = (struct vm_vinteger_t *)&obj->ast.buffer[i];
        printf("(VINTEGER)[%lu][", sizeof(struct vm_vinteger_t));
        printf("type: %d, ", node->type);
        printf("ret: %d, ", node->ret);
        printf("value: %d]\n", node->value);
        i += sizeof(struct vm_vinteger_t)-1;
      } break;
      case VFLOAT: {
        struct vm_vfloat_t *node = (struct vm_vfloat_t *)&obj->ast.buffer[i];
        printf("(VFLOAT)[%lu][", sizeof(struct vm_vfloat_t));
        printf("type: %d, ", node->type);
        printf("ret: %d, ", node->ret);
        printf("value: %g]\n", node->value);
        i += sizeof(struct vm_vfloat_t)-1;
      } break;
      case TFALSE:
      case TTRUE: {
        struct vm_ttrue_t *node = (struct vm_ttrue_t *)&obj->ast.buffer[i];
        printf("(TTRUE)[%lu][", 4+(node->nrgo*sizeof(node->go[0])));
        printf("type: %d, ", node->type);
        printf("ret: %d, ", node->ret);
        printf("nrgo: %d, ", node->nrgo);
        printf("go[");
        int x = 0;
        for(x=0;x<node->nrgo;x++) {
          printf("%d: %d", x, node->go[x]);
          if(node->nrgo-1 > x) {
            printf(", ");
          }
        }
        printf("]]\n");
        i += sizeof(struct vm_ttrue_t)+(sizeof(node->go[0])*node->nrgo)-1;
      } break;
      case TFUNCTION: {
        struct vm_tfunction_t *node = (struct vm_tfunction_t *)&obj->ast.buffer[i];
        printf("(TFUNCTION)[%lu][", 8+(node->nrgo*sizeof(node->go[0])));
        printf("type: %d, ", node->type);
        printf("ret: %d, ", node->ret);
        printf("token: %d, ", node->token);
        printf("value: %d, ", node->value);
        printf("nrgo: %d, ", node->nrgo);
        printf("go[");
        int x = 0;
        for(x=0;x<node->nrgo;x++) {
          printf("%d: %d", x, node->go[x]);
          if(node->nrgo-1 > x) {
            printf(", ");
          }
        }
        printf("]]\n");
        i += sizeof(struct vm_tfunction_t)+(sizeof(node->go[0])*node->nrgo)-1;
      } break;
      case TOPERATOR: {
        struct vm_toperator_t *node = (struct vm_toperator_t *)&obj->ast.buffer[i];
        printf("(TOPERATOR)[10][");
        printf("type: %d, ", node->type);
        printf("ret: %d, ", node->ret);
        printf("token: %d, ", node->token);
        printf("left: %d, ", node->left);
        printf("right: %d, ", node->right);
        printf("value: %d]\n", node->value);
        i += sizeof(struct vm_toperator_t)-1;
      } break;
      case TEOF: {
        struct vm_teof_t *node = (struct vm_teof_t *)&obj->ast.buffer[i];
        printf("(TEOF)[1][type: %d]\n", node->type);
        i += sizeof(struct vm_teof_t)-1;
      } break;
      case VNULL: {
        struct vm_vnull_t *node = (struct vm_vnull_t *)&obj->ast.buffer[i];
        printf("(VNULL)[3][");
        printf("type: %d, ", node->type);
        printf("ret: %d]\n", node->ret);
        i += sizeof(struct vm_vnull_t)-1;
      } break;
    }
  }
}
#endif
/*LCOV_EXCL_STOP*/

int rule_initialize(struct pbuf *input, struct rules_t ***rules, int *nrrules, struct pbuf *mempool, void *userdata) {
  unsigned int nrbytes = 0, len = strlen((char *)input->payload), newlen = len;
  unsigned int suggested_varstack_size = 0;

  if(mempool->len < 512) {
    mempool->len = 512;
  }
  if(*nrrules >= 64) {
#ifdef ESP8266
    Serial1.println(PSTR("more than the maximum of 64 rule blocks defined"));
#else
    printf("more than the maximum of 64 rule blocks defined\n");
#endif
  }

  if(input->len < alignedbuffer(mempool->len)) {
#ifdef ESP8266
    Serial1.println(PSTR("not enough free space in rules mempool"));
#else
    printf("not enough free space in rules mempool\n");
#endif
  }

  if(len == 0) {
    return 1;
  }
  *rules = (struct rules_t **)&((unsigned char *)mempool->payload)[0];
  (*rules)[*nrrules] = (struct rules_t *)&((unsigned char *)mempool->payload)[alignedbuffer(mempool->len)];
  memset((*rules)[*nrrules], 0, sizeof(struct rules_t));
  mempool->len += sizeof(struct rules_t);

  (*rules)[*nrrules]->userdata = userdata;

  struct rules_t *obj = (*rules)[*nrrules];
  obj->nr = (*nrrules)+1;
  (*nrrules)++;

  obj->ast.nrbytes = 0;
  obj->ast.bufsize = 0;
  obj->varstack.nrbytes = 4;
  obj->varstack.bufsize = 4;

/*LCOV_EXCL_START*/
#if defined(DEBUG) or defined(ESP8266)
  #ifdef ESP8266
    obj->timestamp.first = micros();
  #else
    clock_gettime(CLOCK_MONOTONIC, &obj->timestamp.first);
  #endif
#endif
/*LCOV_EXCL_STOP*/

  if(rule_prepare((char **)&input->payload, &nrbytes, &newlen) == -1) {
    return -1;
  }

/*LCOV_EXCL_START*/
#if defined(DEBUG) or defined(ESP8266)
  #ifdef ESP8266
  obj->timestamp.second = micros();

  logprintf_P(F("rule #%d was prepared in %d microseconds"), obj->nr, obj->timestamp.second - obj->timestamp.first);
  #else
  clock_gettime(CLOCK_MONOTONIC, &obj->timestamp.second);

  printf("rule #%d was prepared in %.6f seconds\n", obj->nr,
    ((double)obj->timestamp.second.tv_sec + 1.0e-9*obj->timestamp.second.tv_nsec) -
    ((double)obj->timestamp.first.tv_sec + 1.0e-9*obj->timestamp.first.tv_nsec));
  #endif
#endif
/*LCOV_EXCL_STOP*/


/*LCOV_EXCL_START*/
#if defined(DEBUG) or defined(ESP8266)
  #ifdef ESP8266
  obj->timestamp.first = micros();
  #else
  clock_gettime(CLOCK_MONOTONIC, &obj->timestamp.first);
  #endif
#endif
/*LCOV_EXCL_STOP*/

  {
    obj->ast.bufsize = alignedbuffer(nrbytes);

    obj->ast.buffer = (unsigned char *)&((unsigned char *)mempool->payload)[alignedbuffer(mempool->len)];
    mempool->len += obj->ast.bufsize;

    suggested_varstack_size = (input->len-mempool->len);

    /*
     * The memoffset will be increased below
     * as soon as we know how many bytes
     * we maximally need.
     */
    obj->varstack.buffer = &((unsigned char *)mempool->payload)[mempool->len];

    memset(obj->ast.buffer, 0, obj->ast.bufsize);
    memset(obj->varstack.buffer, 0, suggested_varstack_size);

    if(rule_parse((char **)&input->payload, (int *)&newlen, obj) == -1) {
      return -1;
    }

    input->len += newlen;
    if(((char *)input->payload)[newlen] == 0 && len > newlen) {
      input->len += 1;
    }
  }

/*LCOV_EXCL_START*/
#if defined(DEBUG) or defined(ESP8266)
  #ifdef ESP8266
  obj->timestamp.second = micros();

  logprintf_P(F("rule #%d was parsed in %d microseconds"), obj->nr, obj->timestamp.second - obj->timestamp.first);
  logprintf_P(F("bytecode is %d bytes"), obj->ast.nrbytes);
  #else
  clock_gettime(CLOCK_MONOTONIC, &obj->timestamp.second);

  printf("rule #%d was parsed in %.6f seconds\n", obj->nr,
    ((double)obj->timestamp.second.tv_sec + 1.0e-9*obj->timestamp.second.tv_nsec) -
    ((double)obj->timestamp.first.tv_sec + 1.0e-9*obj->timestamp.first.tv_nsec));

  printf("bytecode is %d bytes\n", obj->ast.nrbytes);
  #endif
#endif
/*LCOV_EXCL_STOP*/

/*LCOV_EXCL_START*/
#ifdef DEBUG
  #ifndef ESP8266
  print_bytecode(obj);
  printf("\n");
  print_tree(obj);
  #endif
#endif
/*LCOV_EXCL_STOP*/

/*LCOV_EXCL_START*/
#if defined(DEBUG) or defined(ESP8266)
  #ifdef ESP8266
    obj->timestamp.first = micros();
  #else
    clock_gettime(CLOCK_MONOTONIC, &obj->timestamp.first);
  #endif
#endif
/*LCOV_EXCL_STOP*/

  if(rule_run(obj, 1) == -1) {
    return -1;
  }

  /*
   * Reserve space for the actual maximum
   * varstack buffer size
   */
  mempool->len += alignedbuffer(obj->varstack.bufsize);

/*LCOV_EXCL_START*/
#if defined(DEBUG) or defined(ESP8266)
  #ifdef ESP8266
    obj->timestamp.second = micros();

    logprintf_P(F("rule #%d was executed in %d microseconds"), obj->nr, obj->timestamp.second - obj->timestamp.first);
    logprintf_P(F("bytecode is %d bytes"), obj->ast.nrbytes);
  #else
    clock_gettime(CLOCK_MONOTONIC, &obj->timestamp.second);

    printf("rule #%d was executed in %.6f seconds\n", obj->nr,
      ((double)obj->timestamp.second.tv_sec + 1.0e-9*obj->timestamp.second.tv_nsec) -
      ((double)obj->timestamp.first.tv_sec + 1.0e-9*obj->timestamp.first.tv_nsec));

    printf("bytecode is %d bytes\n", obj->ast.nrbytes);
  #endif
#endif
/*LCOV_EXCL_STOP*/

  return 0;
}
