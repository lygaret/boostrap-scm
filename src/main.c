#include <stdio.h>
#include <stdlib.h>
#include "scheme.h"

int main() {
  vm_t*  vm = malloc(sizeof(vm_t));
  value_t f = make_integer(vm, 3980);
  
  printf("addition!: %d\n", as_integer(f));
}
