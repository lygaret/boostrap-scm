#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include "scheme.h"
#define BUFFER_MAX 2048

/* reader */

char peek(FILE *in);
int is_delimiter(char c);

int consume_char(char c, FILE *in);
int consume_string(const char* match, int len, FILE *in);
int consume_ws(FILE *in);
int consume_line(FILE *in);

object *read_fixnum(context *ctxt, FILE *in);
object *read_macrochar(context *ctxt, FILE *in);
object *read_pair(context *ctxt, FILE *in);
object *read_slashchar(context *ctxt, FILE *in);
object *read_string(context *ctxt, FILE *in);
object *read_objvector(context *ctxt, FILE *in);

object *read(context *ctxt, FILE *in) {
  int c;
  object *out = 0;

  consume_ws(in);

  c = getc(in);
  if      (c == '#') {
    out = read_macrochar(ctxt, in);
  }
  else if (c == '\\') {
    out = read_slashchar(ctxt, in);
  }
  else if (c == '"') {
    out = read_string(ctxt, in);
  }
  else if (isdigit(c)) {
    ungetc(c, in);
    out = read_fixnum(ctxt, in);
  }
  else if (c == '(') {
    out = read_pair(ctxt, in);
  }
  else {
    fprintf(stderr, "bad input, unexpected '%c'\n", c);
    exit(1);
  }

  /* require a delimiter after input */
  if (is_delimiter(peek(in))) {
    return out;
  }
  else {
    fprintf(stderr, "object not followed by a delimeter!\n");
    exit(1);
  }
}

/* '#' has already been read */
object *read_macrochar(context *ctxt, FILE *in) {
  int c;

  switch(c = getc(in)) {
  case 't':
    return ctxt->true_obj;

  case 'f':
    return ctxt->false_obj;

  case '[':
    return read_objvector(ctxt, in);

  default:
    fprintf(stderr, "(macrochar) bad input, unexpected '%c'\n", c);
    exit(1);
  }
}

/* '\' has already been read */
object *read_slashchar(context *ctxt, FILE *in) {
  int c;

  if (consume_string("newline", 7, in)) {
    return make_character('\n');
  }
  else if (consume_string("tab", 3, in)) {
    return make_character('\t');
  }
  else if (consume_string("space", 5, in)) {
    return make_character(' ');
  }
  else if (consume_string("backspace", 9, in)) {
    return make_character('\b');
  }
  else {
    c = getc(in);
    return make_character(c);
  }
}

/* '"' has already been read */
object *read_string(context *ctxt, FILE *in) {
  char c;
  char buffer[BUFFER_MAX];
  int len = 0;

  while((c = getc(in)) != '"') {
    if (c == EOF) {
      fprintf(stderr, "unterminated string literal\n");
      exit(1);
    }

    if (c == '\\') {
      c = getc(in);
      if (c == 'n') {
        c = '\n';
      } else if (c == 't') {
        c = '\t';
      }
    }

    if (len < BUFFER_MAX - 1) {
      buffer[len++] = c;
    }
    else {
      fprintf(stderr, "string too long, max length is %d", BUFFER_MAX);
      exit(1);
    }
  }
  
  buffer[len++] = '\0';
  return make_string(buffer, len);
}

/* '#[' has already been consumed */
/* read as pairs, convert to vector once we know the length */
object *read_objvector_recur(context *ctxt, FILE *in, int currlength) {
  object *vector;
  object *next_obj;

  consume_ws(in);
  if (consume_char(']', in)) {
    /* we hit the bottom, we know the length */
    return make_objvector(currlength);
  }

  next_obj = read(ctxt, in);
  vector   = read_objvector_recur(ctxt, in, currlength + 1);

  objvector_set(vector, currlength, next_obj);
  return vector;
}

object *read_objvector(context *ctxt, FILE *in) {
  return read_objvector_recur(ctxt, in, 0);
}

object *read_fixnum(context *ctxt, FILE *in) {
  char c;
  long num = 0;

  /* consume binary strings */
  if (consume_string("0b", 2, in)) {
    while((c = getc(in))) {
      if (c == '0') {
        num = (num * 2);
      }
      else if (c == '1') {
        num = (num * 2) + 1;
      }
      else {
        break;
      }
    }

    /* unread the non-digit */
    ungetc(c, in);
  }

  /* consume hex strings */
  else if (consume_string("0x", 2, in)) {
    while((c = getc(in))) {
      if (isdigit(c)) {
        num = (num * 16) + (c - '0');
      }
      else if (c >= 'a' && c <= 'f') {
        num = (num * 16) + ((c - 'a') + 10);
      }
      else if (c >= 'A' && c <= 'F') {
        num = (num * 16) + ((c - 'A') + 10);
      }
      else {
        break;
      }
    }

    /* unread the non-digit */
    ungetc(c, in);
  }

  /* consume decimal strings */
  else {
    while(isdigit(c = getc(in))) {
      num = (num * 10) + (c - '0');
    }

    /* unread the non-digit */
    ungetc(c, in);
  }

  return make_fixnum(num);
}

/* the opening paren has already been read */
object *read_pair(context *ctxt, FILE *in) {
  object *car_obj;
  object *cdr_obj;

  consume_ws(in);
  if (consume_char(')', in)) {
    /* closing paren means (), means nil */
    return ctxt->nil;
  }

  car_obj = read(ctxt, in);

  consume_ws(in);
  if (consume_char('.', in)) {
    /* improper list means explicit cdr, and explicit close paren */
    cdr_obj = read(ctxt, in);

    consume_ws(in);
    if (!consume_char(')', in)) {
      fprintf(stderr, "expecting ')' to close an improper list");
      exit(1);
    }
  }
  else {
    /* proper list means implicit cdr, through recursion */
    cdr_obj = read_pair(ctxt, in);

  }

  return make_pair(car_obj, cdr_obj);
}

/* lexing helpers */

char peek(FILE *in) {
  int c;

  c = getc(in);
  ungetc(c, in);
  return c;
}

int is_delimiter(char c) {
  return isspace(c) || c == EOF ||
           c == '(' || c == ')' ||
           c == '[' || c == ']' ||
           c == '"' || c == ';' ;
}

int consume_char(char c, FILE *in) {
  char inc;
  inc = getc(in);
  if (c != inc) {
    ungetc(inc, in);
  }

  return c == inc;
}

int consume_ws(FILE *in) {
  char c;
  while ((c = getc(in)) != EOF) {
    if (isspace(c)) {
      /* skip whitespace */
      continue;
    }
    else if (c == ';') {
      /* skip comments (to the end of the line) */
      consume_line(in);
      continue;
    }
    else {
      /* otherwise, we didn't get whitespace, bounce */
      ungetc(c, in);
      break;
    }
  }

  return 1;
}

int consume_line(FILE *in) {
  char c;
  while ((c = getc(in)) != EOF && c != '\n') {}

  return 1;
}

int consume_string(const char* match, int len, FILE *in) {
  const char *cursor = match;

  int  i;
  char c;

  for (i = 0; i < len; i++, cursor++) {
    c = getc(in);

    /* put the _last_ character read back on,
       and then undo any match characters we've already read */
    if (c != *cursor) {
      ungetc(c, in);
      for (cursor--; cursor >= match; cursor--) {
        ungetc(*cursor, in);
      }

      return 0;
    }
  }

  return 1;
}
