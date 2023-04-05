#ifndef __scheme_h
#define __scheme_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct pool {
  int   size;
  void* buffer;
} pool_t;

typedef struct vm {
  /* pools */
} vm_t;

typedef union value {
  double   as_double;
  uint64_t as_uint64;
} value_t;

typedef enum {
  PTR_INVALID = 0,
  PTR_MALLOC,
  PTR_POOL,
  PTR_PRIMITIVE,
} ptr_type_t;

typedef enum {
  HND_INVALID = 8,
  HND_CONS,
  HND_SYMBOL,
  HND_STRING,
} hnd_type_t;

typedef enum {
  BOX_INVALID = 0,
  BOX_BOOLEAN,
  BOX_CHARACTER,
  BOX_INTEGER,
  BOX_DOUBLE,

  BOX_ERROR = 0xE,
  BOX_NIL   = 0xF
} box_type_t;

vm_t* alloc_vm();

extern value_t vnan;
extern value_t vnanq;
extern value_t vpinf;
extern value_t vninf;

extern value_t vnil;
extern value_t vtrue;  
extern value_t vfalse; 

/* comparisons */
bool     value_exact(value_t a, value_t b);

/* bools */
bool     is_vtruth(value_t v);
bool     is_vfalse(value_t v);

/* schemey bools (false = #f, everything else = #t) */
bool     is_truthy(value_t v);
bool     is_falsey(value_t v);

/* doubles */
value_t  make_double(vm_t* vm, double v);
double   as_double(value_t v);
bool     is_double(value_t v);
bool     is_nan(value_t v);
bool     is_inf(value_t v);

/* ints */
value_t  make_integer(vm_t* vm, uint32_t v);
bool     is_integer(value_t v);
uint32_t as_integer(value_t v);

/* floats */
value_t  make_float(vm_t* vm, float v);
bool     is_float(value_t v);
float    as_float(value_t v);

/* chars */
value_t  make_character(vm_t* vm, char c);
bool     is_character(value_t v);
char     as_character(value_t v);

/* strings */
value_t  make_string(vm_t* vm, char *str, int len);
bool     is_string(value_t v);
char*    as_string(value_t v);

/* symbols */
value_t  make_symbol(vm_t* vm, char *str, int len);
bool     is_symbol(value_t v);

/* errors */
value_t  make_error(vm_t* vm, uint32_t code);
bool     is_error(value_t v);

/* cons cells */
value_t  make_cons(vm_t* vm, value_t car, value_t cdr);
bool     is_cons(value_t v);
bool     is_nil(value_t v);

/* buffers */
value_t  make_buffer(vm_t* vm, int size, char fill);
bool     is_buffer(value_t v);

/* vectors */
value_t  make_vector(vm_t* vm, int size, value_t fill);
bool     is_vector(value_t v);

/* procs */

typedef value_t (*native_proc_fn)(vm_t* vm, value_t args);

value_t  make_native_proc(vm_t* vm, native_proc_fn fn);
value_t  make_compound_proc(vm_t* vm, value_t body, value_t env);

bool     is_proc(value_t v);
bool     is_native_proc(value_t v);
bool     is_compound_proc(value_t v);

/* the machine */

value_t read(vm_t* vm, FILE *in);
value_t eval(vm_t* vm, value_t v);
void    print(vm_t* vm, value_t v);

#endif

/*

object* alloc_object(void);

object *pair_car(object *obj);
object *pair_cdr(object *obj);

#define pair_cons(a,b) make_pair(a, b)
#define pair_caar(obj) pair_car(pair_car(obj))
#define pair_cadr(obj) pair_car(pair_cdr(obj))
#define pair_cdar(obj) pair_cdr(pair_car(obj))
#define pair_cddr(obj) pair_cdr(pair_cdr(obj))
#define pair_caddr(obj) pair_car(pair_cddr(obj))
#define pair_cdddr(obj) pair_cdr(pair_cdr(pair_cdr(obj)))
#define pair_cadddr(obj) pair_car(pair_cdr(pair_cdr(pair_cdr(obj))))

void pair_set_car(object *obj, object* val);
void pair_set_cdr(object *obj, object* val);

#endif

*/
