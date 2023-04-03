#include <stdio.h>
#include "scheme.h"

int main (void) {
  printf("bootstrap scheme v0.01\nuse ctrl-d to exit.\n");

  context *ctxt = alloc_context();
  object  *exp;

  while (1) {
    printf("> ");
    exp = read(ctxt, stdin);
    exp = eval(ctxt, exp);
    print(exp);
    printf("\n");
  }

  return 0;
}
