#ifndef __scheme_h
#define __scheme_h

typedef enum {
  NIL,
  FIXNUM,
  STRING,
  CHARACTER,
  BOOLEAN,
  PAIR,
  OBJVECTOR,
  SYMBOL
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
      char *value;
      int length;
    } symbol;
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

  object *symbols_table;
  object *default_environment;
  object *current_environment;

  object *quote_sym;
} context;

context *alloc_context(void);
object *alloc_object(void);

object* environment_update(context *ctxt, object *env, object* key, object* value);
object* environment_get(context *ctxt, object *env, object* key);

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
char is_nil(object *obj);
char is_pair(object *obj);

object *pair_car(object *obj);
object *pair_cdr(object *obj);

#define pair_cons(a,b) make_pair(a, b)
#define pair_caar(obj) pair_car(pair_car(obj))
#define pair_cadr(obj) pair_car(pair_cdr(obj))
#define pair_cdar(obj) pair_cdr(pair_car(obj))
#define pair_cddr(obj) pair_cdr(pair_cdr(obj))

void pair_set_car(object *obj, object* val);
void pair_set_cdr(object *obj, object* val);

object *make_objvector(int size, object *fill);
char is_objvector(object *obj);
object *objvector_get(object *obj, int index);
void objvector_set(object *obj, int index, object* val);

object *make_symbol(context *ctxt, char *value, int len);
int is_symbol(object *obj);

/* reader */

object *read(context *ctxt, FILE *in);
object *eval(context *ctxt, object *obj);
void print(object *obj);

#endif
