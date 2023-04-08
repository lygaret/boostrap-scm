#ifndef __scheme_h
#define __scheme_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define unused(v) ((void)v);

typedef union value {
  double   as_double;
  uint64_t as_uint64;
} value_t;

typedef struct context {
  int cons_pool_size;
  int cons_pool_limit;
  value_t *cons_pool_ptr;
  value_t cons_free_list;
} context_t;

typedef context_t* context_p;

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
  BOX_FLOAT,

  BOX_ERROR = 0xE,
  BOX_NIL   = 0xF
} box_type_t;

/* convenience to avoid casting */
typedef union box_data {
  float    as_float;
  int32_t  as_int32;
  uint32_t as_uint32;
} box_data_t;

context_p alloc_context(int initial_size);

extern value_t vnan;
extern value_t vnanq;
extern value_t vpinf;
extern value_t vninf;

extern value_t vnil;
extern value_t vtrue;  
extern value_t vfalse; 

/* the machine */
context_p  alloc_context();

value_t    read(context_p ctxt, FILE *in);
value_t    eval(context_p ctxt, value_t v);
void       print(context_p ctxt, value_t v);

/* boxes */
value_t    make_boxed(box_type_t type, uint16_t aux, box_data_t value);
bool       is_boxed(box_type_t type, value_t v);
box_type_t boxed_type(value_t v);
uint16_t   boxed_aux(value_t v);
box_data_t boxed_data(value_t v);

/* pointers */
value_t    make_pointer(context_p ctxt, ptr_type_t type, void* addr);
bool       is_pointer(ptr_type_t type, value_t v);
ptr_type_t pointer_type(value_t v);
void*      pointer_addr(value_t v);

/* handles */
value_t    make_handle(context_p ctxt, hnd_type_t type, uint16_t aux, uint32_t offset);
bool       is_handle(hnd_type_t type, value_t v);
hnd_type_t handle_type(value_t v);
uint16_t   handle_aux(value_t v);
uint32_t   handle_offset(value_t v);

/* comparisons */
bool       value_exact(value_t a, value_t b);

/* refs (both pointers & handles) */
bool       is_reference(value_t v);

/* bools */
bool       is_vtruth(value_t v);
bool       is_vfalse(value_t v);

/* schemey bools (false = #f, everything else = #t) */
bool       is_truthy(value_t v);
bool       is_falsey(value_t v);

/* doubles */
value_t    make_double(context_p ctxt, double v);
double     as_double(value_t v);
bool       is_double(value_t v);
bool       is_nan(value_t v);
bool       is_inf(value_t v);

/* ints */
value_t    make_integer(context_p ctxt, uint32_t v);
bool       is_integer(value_t v);
uint32_t   as_integer(value_t v);

/* floats */
value_t    make_float(context_p ctxt, float v);
bool       is_float(value_t v);
float      as_float(value_t v);

/* chars */
value_t    make_character(context_p ctxt, char c);
bool       is_character(value_t v);
char       as_character(value_t v);


/* strings */
value_t    make_string(context_p ctxt, char *str, int len);
bool       is_string(value_t v);
char*      as_string(value_t v);

/* symbols */
value_t    make_symbol(context_p ctxt, char *str, int len);
bool       is_symbol(value_t v);

/* errors */
value_t    make_error(context_p ctxt, uint32_t code);
bool       is_error(value_t v);

/* cons cells */
value_t    make_cons(context_p ctxt, value_t car, value_t cdr);
bool       is_cons(value_t v);
bool       is_nil(value_t v);

value_t    car(context_p ctxt, value_t hnd);
value_t    cdr(context_p ctxt, value_t hnd);
#define caar(ctxt, hnd) car(ctxt, car(ctxt, hnd))
#define cadr(ctxt, hnd) car(ctxt, cdr(ctxt, hnd))
#define cdar(ctxt, hnd) cdr(ctxt, car(ctxt, hnd))
#define cddr(ctxt, hnd) cdr(ctxt, cdr(ctxt, hnd))

void       set_car(context_p ctxt, value_t hnd, value_t v);
void       set_cdr(context_p ctxt, value_t hnd, value_t v);
#define set_caar(ctxt, hnd, v) set_car(ctxt, car(ctxt, hnd), v)
#define set_cadr(ctxt, hnd, v) set_car(ctxt, cdr(ctxt, hnd), v)
#define set_cdar(ctxt, hnd, v) set_cdr(ctxt, car(ctxt, hnd), v)
#define set_cddr(ctxt, hnd, v) set_cdr(ctxt, cdr(ctxt, hnd), v)

/* buffers */
value_t    make_buffer(context_p ctxt, int size, char fill);
bool       is_buffer(value_t v);

/* vectors */
value_t    make_vector(context_p ctxt, int size, value_t fill);
bool       is_vector(value_t v);

/* procs */

typedef value_t (*native_proc_fn)(context_p ctxt, value_t args);

value_t    make_native_proc(context_p ctxt, native_proc_fn fn);
value_t    make_compound_proc(context_p ctxt, value_t body, value_t env);

bool       is_proc(value_t v);
bool       is_native_proc(value_t v);
bool       is_compound_proc(value_t v);

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
