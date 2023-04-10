#include <stdlib.h>
#include <stdio.h>
#include "scheme.h"

int main (void) {
  printf("bootstrap scheme v0.01\nuse ctrl-d to exit.\n");

  value_t v;
  context_p ctxt = alloc_context(4096);

  value_t a, b, c;

  a = make_cons(ctxt, make_integer(ctxt, 1234), vnil);
  a = make_cons(ctxt, make_integer(ctxt, 5678), a);
  a = make_cons(ctxt, make_double(ctxt, 3.1415926), a);
  a = make_cons(ctxt, make_integer(ctxt, 42), a);
  a = make_cons(ctxt, make_cons(ctxt, make_integer(ctxt, 10), make_integer(ctxt, 20)), a);
  
  printf("test: ");
  print(ctxt, a);
  printf("\n");

  while (1) {
    printf("> ");
    v = read(ctxt, stdin);
    v = eval(ctxt, v);

    if (is_error(v)) {
      printf("!!! error: %lx", v.as_uint64);
    }
    else {
      print(ctxt, v);
    }

    printf("\n");
  }

  return 0;
}
