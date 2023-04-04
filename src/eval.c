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
    object *car = pair_car(obj); 
    object *cdr = pair_cdr(obj);

    if (car == ctxt->quote_sym) {
      return cdr;
    }

    if (car == ctxt->cons_sym) {
      car = eval(ctxt, pair_car(cdr));
      cdr = eval(ctxt, pair_cdr(cdr));
      return make_pair(car, cdr);
    }

    if (car == ctxt->car_sym) {
      cdr = eval(ctxt, pair_car(cdr));
      return pair_car(obj);
    }

    if (car == ctxt->cdr_sym) {
      cdr = eval(ctxt, pair_car(cdr));
      return pair_cdr(obj);
    }

    else if (car == ctxt->set_sym || car == ctxt->define_sym) {
      car = pair_car(cdr);
      cdr = eval(ctxt, pair_cadr(cdr));
      ctxt->current_environment = environment_update(ctxt, ctxt->current_environment, car, cdr);

      return cdr;
    }
      
    printf("todo: apply");
    return obj;

  case OBJVECTOR:
    return ctxt->nil;

  default:
    fprintf(stderr, "this object isn't handled currently\n");
    exit(1);
  }
}
