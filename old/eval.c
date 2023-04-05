#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "scheme.h"

object *eval(context *ctxt, object *obj) {
 tailcall:
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

    else if (car == ctxt->set_sym || car == ctxt->define_sym) {
      car = pair_car(cdr);
      cdr = eval(ctxt, pair_cadr(cdr));
      ctxt->current_environment = environment_update(ctxt, ctxt->current_environment, car, cdr);

      return cdr;
    }

    else if (car == ctxt->if_sym) {
      car = eval(ctxt, pair_cadr(obj));
      if (is_truthy(ctxt, car)) {
        obj = pair_caddr(obj);
      }
      else {
        if (is_nil(pair_cdddr(obj))) {
          obj = ctxt->false_obj;
        }
        else {
          obj = pair_cadddr(obj);
        }
      }

      goto tailcall;
    }

    else {
      printf("applying:\n  car: ");
      print(car);
      printf("\n  cdr: ");
      print(pair_car(cdr));
      printf("\n");

      car = environment_get(ctxt, ctxt->current_environment, car);
      cdr = eval(ctxt, pair_car(cdr));

      printf("lookup car: ");
      print(car);
      printf("\n  eval'd cdr: ");
      print(cdr);
      printf("\n");
      
      if (is_primitive_proc(car)) {
        obj = car->data.primitive_proc.fn(ctxt, cdr);
        return obj;
      }

      printf("todo: apply compound procedures\n");
      return obj;
    }
      
  case OBJVECTOR:
    return ctxt->nil;

  default:
    fprintf(stderr, "this object isn't handled currently\n");
    exit(1);
  }
}
