#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "scheme.h"

object *eval(context *ctxt, object *obj) {
  switch(obj->type) {
  case NIL:
  case FIXNUM:
  case STRING:
  case CHARACTER:
  case BOOLEAN:
    return obj;

  case SYMBOL:
    return environment_get(ctxt, ctxt->current_environment, obj);

  case PAIR:
    if (pair_car(obj) == ctxt->quote_sym) {
      return pair_cdr(obj);
    }

    return ctxt->nil;

  case OBJVECTOR:
    return ctxt->nil;

  default:
    fprintf(stderr, "this object isn't handled currently\n");
    exit(1);
  }
}
