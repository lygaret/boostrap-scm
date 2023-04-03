#ifndef __scheme_h
#define __scheme_h

typedef enum {
  NIL,
  FIXNUM,
  STRING,
  CHARACTER,
  BOOLEAN,
  PAIR,
  OBJVECTOR
} object_type;

typedef struct object {
  object_type type;
  union {
    struct {
      char value;
    } byte;
    struct {
      char *value;
      int length;
    } string;
    struct {
      long value;
    } fixnum;
    struct {
      struct object* car;
      struct object* cdr;
    } pair;
    struct {
      int size;
      struct object** head;
    } objvector;
  } data;
} object;

typedef struct context {
  object *nil;
  object *true_obj;
  object *false_obj;
  object *interned_strs;
} context;

context *alloc_context(void);
object *alloc_object(void);

object *make_fixnum(long value);
char is_fixnum(object *obj);

object *make_boolean(char value);
char is_boolean(object *obj);
char is_false(context *ctxt, object *obj);
char is_truthy(context *ctxt, object *obj);

object *make_character(char value);
char is_character(object *obj);

object *make_string(char *value, int len);
int is_string(object *obj);

object *make_pair(object *car, object *cdr);
object *make_objvector(int size, object *fill);

char is_nil(object *obj);
char is_pair(object *obj);
char is_objvector(object *obj);

object *pair_car(object *obj);
void pair_set_car(object *obj, object* val);

object *pair_cdr(object *obj);
void pair_set_cdr(object *obj, object* val);

object *objvector_get(object *obj, int index);
void objvector_set(object *obj, int index, object* val);

object *intern_string(context *ctxt, char *value, int len);

/* reader */

object *read(context *ctxt, FILE *in);
object *eval(context *ctxt, object *obj);
void print(object *obj);

#endif
