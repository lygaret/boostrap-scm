#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "scheme.h"

#define INTERNED_TABLE_SIZE 97 /* prime! */

object *eq_proc(context *ctxt, object *args) {
  object *obj1;
  object *obj2;
    
  obj1 = eval(ctxt, pair_car(args));
  obj2 = eval(ctxt, pair_cadr(args));
    
  if (obj1->type != obj2->type) {
    return ctxt->false_obj;
  }

  switch (obj1->type) {
  case FIXNUM:
    return (obj1->data.fixnum.value == 
            obj2->data.fixnum.value) ?
      ctxt->true_obj : ctxt->false_obj;

  case CHARACTER:
    return (obj1->data.byte.value == 
            obj2->data.byte.value) ?
      ctxt->true_obj : ctxt->false_obj;

  case STRING:
    if (obj1->data.string.length != obj2->data.string.length) {
      return ctxt->false_obj;
    }
    
    return (strncmp(obj1->data.string.value, 
                    obj2->data.string.value,
                    obj1->data.string.length)
            == 0) ?
      ctxt->true_obj : ctxt->false_obj;

  default:
    return (obj1 == obj2) ? ctxt->true_obj : ctxt->false_obj;
  }
}

object* add_proc(context *ctxt, object *args) {
  object *car;
  long value = 0;
  
  while(!is_nil(args)) {
    car  = eval(ctxt, pair_car(args));
    args = pair_cdr(args);

    value += car->data.fixnum.value;
  }

  return make_fixnum(value);
}

object* sub_proc(context *ctxt, object *args) {
  object *car;
  long value;

  value = pair_car(args)->data.fixnum.value;
  args  = pair_cdr(args);

  /* only one arg, unary negation */
  if (is_nil(args)) {
    return make_fixnum(-1 * value);
  }

  while (!is_nil(args)) {
    car   = eval(ctxt, pair_car(args));
    args  = pair_cdr(args);

    value = value - car->data.fixnum.value;
  }

  return make_fixnum(value);
}

object *cons_proc(context *ctxt, object *args) {
  return make_pair(eval(ctxt, pair_car(args)), eval(ctxt, pair_cadr(args)));
}

object *car_proc(context *ctxt, object *args) {
  return pair_car(eval(ctxt, args));
}

object *cdr_proc(context *ctxt, object *args) {
  return pair_cdr(eval(ctxt, args));
}

/* no gc, so will live forever */
object *alloc_object(void) {
  object *obj;
  obj = malloc(sizeof(object));

  if (obj == NULL) {
    fprintf(stderr, "out of memory!\n");
    exit(1);
  }

  return obj;
}

/* context holds "globals" */
context *alloc_context(void) {
  context *ctxt;
  ctxt = malloc(sizeof(context));

  ctxt->nil = alloc_object();
  ctxt->nil->type = NIL;

  ctxt->true_obj = alloc_object();
  ctxt->true_obj->type = BOOLEAN;
  ctxt->true_obj->data.byte.value = 1;

  ctxt->false_obj = alloc_object();
  ctxt->false_obj->type = BOOLEAN;
  ctxt->false_obj->data.byte.value = 0;

  ctxt->symbols_table = make_objvector(INTERNED_TABLE_SIZE, ctxt->nil);
  ctxt->default_environment = ctxt->nil;
  ctxt->current_environment = ctxt->nil;

  ctxt->quote_sym = make_symbol(ctxt, "quote", 5);

  ctxt->define_sym = make_symbol(ctxt, "define", 6);
  ctxt->set_sym = make_symbol(ctxt, "set!", 4);

  ctxt->if_sym = make_symbol(ctxt, "if", 2);

  ctxt->current_environment =
    environment_update(ctxt, ctxt->current_environment, make_symbol(ctxt, "eq?", 3), make_primitive_proc(&eq_proc));

  ctxt->current_environment =
    environment_update(ctxt, ctxt->current_environment, make_symbol(ctxt, "+", 1), make_primitive_proc(&add_proc));

  ctxt->current_environment =
    environment_update(ctxt, ctxt->current_environment, make_symbol(ctxt, "-", 1), make_primitive_proc(&sub_proc));

  ctxt->current_environment =
    environment_update(ctxt, ctxt->current_environment, make_symbol(ctxt, "cons", 4), make_primitive_proc(&cons_proc));

  ctxt->current_environment =
    environment_update(ctxt, ctxt->current_environment, make_symbol(ctxt, "car", 3), make_primitive_proc(&car_proc));

  ctxt->current_environment =
    environment_update(ctxt, ctxt->current_environment, make_symbol(ctxt, "cdr", 3), make_primitive_proc(&cdr_proc));

  return ctxt;
}

object *environment_update(context *ctxt, object *env, object *key, object *value) {
  return pair_cons(pair_cons(key, value), env);
}

object *environment_get(context *ctxt, object *env, object *key) {
  object *cursor = env;
  while (!is_nil(cursor)) {
    if (key == pair_caar(cursor)) {
      return pair_cdar(cursor);
    }

    cursor = pair_cdr(cursor);
  }

  return ctxt->nil;
}

object *make_primitive_proc(primitive_proc_fn* fn) {
  object *obj; 

  obj = alloc_object();
  obj->type = PRIMITIVE_PROC;
  obj->data.primitive_proc.fn = fn;
  return obj;
}

int is_primitive_proc(object *obj) {
  return obj->type == PRIMITIVE_PROC;
}


/* fixnums */

object *make_fixnum(long value) {
  object *obj;

  obj = alloc_object();
  obj->type = FIXNUM;
  obj->data.fixnum.value = value;
  return obj;
}

char is_fixnum(object *obj) {
  return obj->type == FIXNUM;
}

/* booleans */

char is_boolean(object *obj) {
  return obj->type == BOOLEAN;
}

char is_false(context *ctxt, object *obj) {
  return obj == ctxt->false_obj;
}

char is_truthy(context *ctxt, object *obj) {
  return !is_false(ctxt, obj);
}

/* chars */

object *make_character(char value) {
  object *obj;

  obj = alloc_object();
  obj->type = CHARACTER;
  obj->data.byte.value = value;
  return obj;
}

char is_character(object *obj) {
  return obj->type == CHARACTER;
}

/* strings */

object *make_string(char *value, int len) {
  object *obj;

  obj = alloc_object();
  obj->type = STRING;
  obj->data.string.length = len;
  obj->data.string.value = malloc(len);
  if (obj->data.string.value == NULL) {
    fprintf(stderr, "out of memory!\n");
    exit(1);
  }
  strncpy(obj->data.string.value, value, len);
  return obj;
}

int is_string(object *obj) {
  return obj->type == STRING;
}

/* pairs */

object *make_pair(object *car, object *cdr) {
  object *obj;

  obj = alloc_object();
  obj->type = PAIR;
  obj->data.pair.car = car;
  obj->data.pair.cdr = cdr;

  return obj;
}

char is_pair(object *obj) {
  return obj->type == PAIR;
}

char is_nil(object *obj) {
  return obj->type == NIL;
}

object *pair_car(object *obj) {
  return obj->data.pair.car;
}

void pair_set_car(object *obj, object *val) {
  obj->data.pair.car = val; 
}

object *pair_cdr(object *obj) {
  return obj->data.pair.cdr;
}

void pair_set_cdr(object *obj, object *val) {
  obj->data.pair.cdr = val; 
}

/* obj vector | linear array of object */

object* make_objvector(int size, object *fill) {
  int i;
  object*  obj;
  object** vec;

  vec = malloc(size * sizeof(object*));
  if (vec == NULL) {
    fprintf(stderr, "out of memory!\n");
    exit(1);
  }

  obj = alloc_object();
  obj->type = OBJVECTOR;
  obj->data.objvector.size = size;
  obj->data.objvector.head = vec;

  for (i = 0; i < size; i++) {
    obj->data.objvector.head[i] = fill;
  }

  return obj;
}

object* objvector_get(object* obj, int index) {
  if (index < 0 || index >= obj->data.objvector.size) {
    fprintf(stderr, "out of bounds!");
    exit(1);
  }

  return obj->data.objvector.head[index];
}

void objvector_set(object* obj, int index, object* value) {
  if (index < 0 || index >= obj->data.objvector.size) {
    fprintf(stderr, "out of bounds!");
    exit(1);
  }

  obj->data.objvector.head[index] = value;
}

/* interned strings | easy hash table, built on objvector */

int intern_hash(char *value, int len, int table_size) {
  unsigned int i;
  unsigned int hash = 0;
  for (i = 0; i < len; i++) {
    hash = (hash << 5) + value[i];
  }

  return hash % table_size;
}

int is_symbol(object *obj) {
  return obj->type == SYMBOL;
}

object *make_symbol(context *ctxt, char *value, int len) { 
  object *obj, *pair, *car, *cdr, *table;
  int hash;

  table = ctxt->symbols_table;
  hash  = intern_hash(value, len, table->data.objvector.size);
  pair  = table->data.objvector.head[hash];

  if (pair == ctxt->nil) {
    /* nothing hashed here yet, so set the value */
    obj = make_string(value, len);
    obj->type = SYMBOL;

    pair = make_pair(obj, ctxt->nil);
    objvector_set(table, hash, pair);

    return obj;
  }
  else {
    while (1) {
      car = pair_car(pair);
      cdr = pair_cdr(pair);

      if (is_symbol(car) && 
          car->data.symbol.length == len &&
          strncmp(car->data.string.value, value, len) == 0) {
        /* found it */
        return car;
      }
    
      if (is_nil(cdr)) {
        /* add it */
        obj = make_string(value, len);
        obj->type = SYMBOL;

        cdr = make_pair(obj, ctxt->nil);
        pair_set_cdr(pair, cdr);
        
        return obj;
      }

      pair = cdr;
    }
  }
}
