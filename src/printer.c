#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "scheme.h"

void print_pair(object *obj) {
  print(obj->data.pair.car);
  object *cdr = obj->data.pair.cdr;
  if (is_pair(cdr)) {
    printf(" ");
    print_pair(cdr);
  }
  else if (is_nil(cdr)) {
    /* no-op */
  }
  else {
    printf(" . ");
    print(cdr);
  }
}

void print(object *obj) {
  switch (obj->type) {

  case NIL:
    printf("()");
    break;

  case PAIR:
    printf("(");
    print_pair(obj);
    printf(")");
    break;

  case FIXNUM:
    printf("%ld", obj->data.fixnum.value);
    break;

  case BOOLEAN:
    printf("#%c", obj->data.byte.value ? 't' : 'f');
    break;

  case STRING:
    printf("\"%s\"", obj->data.string.value);
    break;

  case CHARACTER:
    if (obj->data.byte.value == ' ') {
      printf("\\space");
    }
    else if (obj->data.byte.value == '\n') {
      printf("\\newline");
    }
    else if (obj->data.byte.value == '\t') {
      printf("\\tab");
    }
    else if (obj->data.byte.value == '\b') {
      printf("\\backspace");
    }
    else {
      printf("\\%c", obj->data.byte.value);
    }
    break;

  default:
    fprintf(stderr, "cannot write unknown type\n");
    exit(1);
  }
}
