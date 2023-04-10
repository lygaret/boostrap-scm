#include <stdio.h>
#include <float.h>
#include "scheme.h"

static void print_raw(context_p ctxt, value_t v);
static void print_cons(context_p ctxt, value_t v);

static void print_raw(context_p ctxt, value_t v) {
  if (is_nil(ctxt, v)) {
    printf("()");
    return;
  }

  if (is_vtruth(ctxt, v)) {
    printf("#t");
    return;
  }

  if (is_vfalse(ctxt, v)) {
    printf("#f");
    return;
  }

  if (is_cons(ctxt, v)) {
    printf("(");
    print_cons(ctxt, v);
    return;
  }

  if (is_symbol(ctxt, v)) {
    int len = string_len(ctxt, v);
    char *ptr = string_ptr(ctxt, v);
    printf("%.*s", len, ptr);
    return;
  }

  if (is_string(ctxt, v)) {
    int len = string_len(ctxt, v);
    char *ptr = string_ptr(ctxt, v);
    printf("\"%.*s\"", len, ptr);
    return;
  }

  if (is_character(v)) {
    char value;
    
    switch((value = as_character(v))) {
    case ' ':
      printf("\\space");
      return;
    case '\n':
      printf("\\newline");
      return; 
    case '\t':
      printf("\\tab");
      return;
    case '\b':
      printf("\\backspace");
      return;
    default:
      printf("\\%c", value);
      return;
    }
  }

  if (is_integer(v)) {
    printf("%d", as_integer(v));
    return;
  }

  if (is_double(v)) {
    printf("%.*f" , DBL_DIG, as_double(v));
    return;
  }

  else {
    printf("<???:%lx>", v.as_uint64);
    return; 
  }
}

static void print_cons(context_p ctxt, value_t v) {
  print_raw(ctxt, cons_car(ctxt, v));

  value_t tail = cons_cdr(ctxt, v);
  if (is_nil(ctxt, tail)) {
    printf(")");
    return;
  }
  if (is_cons(ctxt, tail)) {
    printf(" ");
    print_cons(ctxt, tail);
    return;
  }
  printf(" . ");
  print_raw(ctxt, tail);
  printf(")");
}


void print(context_p ctxt, value_t v) {
  printf("[ %lx ]: ", v.as_uint64);
  print_raw(ctxt, v);
  printf("\n");
}
