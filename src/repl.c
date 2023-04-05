#include <stdlib.h>
#include <stdio.h>
#include "scheme.h"

int main (void) {
  printf("bootstrap scheme v0.01\nuse ctrl-d to exit.\n");

  vm_t *vm = malloc(1);
  value_t v;

  while (1) {
    printf("> ");
    v = read(vm, stdin);
    v = eval(vm, v);
    print(vm, v);
    printf("\n");
  }

  return 0;
}
