#include <stdio.h>
#include "scheme.h"

void print_raw(value_t v) {
  if (is_vtruth(v)) {
    printf("#t");
    return;
  }

  if (is_vfalse(v)) {
    printf("#f");
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
    printf("%f", as_double(v));
    return;
  }

  else {
    printf("<???:%lx>", v.as_uint64);
    return; 
  }
}

void print(vm_t *vm, value_t v) {
  printf("[ %lx ]  ", v.as_uint64);
  print_raw(v);
}
