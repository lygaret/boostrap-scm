#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "scheme.h"

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

  ctxt->true_obj = alloc_object();
  ctxt->true_obj->type = BOOLEAN;
  ctxt->true_obj->data.byte.value = 1;

  ctxt->false_obj = alloc_object();
  ctxt->false_obj->type = BOOLEAN;
  ctxt->false_obj->data.byte.value = 0;

  ctxt->nil = alloc_object();
  ctxt->nil->type = NIL;

  return ctxt;
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
  obj->data.string.value = malloc(len);
  if (obj->data.string.value == NULL) {
    fprintf(stderr, "out of memory!\n");
    exit(1);
  }
  strncpy(obj->data.string.value, value, len);
  return obj;
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

object* make_objvector(int size) {
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
