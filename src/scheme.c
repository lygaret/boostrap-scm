#include <stdlib.h>
#include <string.h>
#include "scheme.h"

/*
  values are nan-boxed

  ieee 754 double precision floats
  sign | exp 11bits | fraction 52bits (most sig bit = quiet)

  all ones  in exponent     = special case
  all zeros in q + fraction = +- infinity
  any bits in fraction      = NaN

  quiet bit is about errors, just preserve it
  this is vaguely system dependent, but x86 and arm handle it this way

  rest available for signalling inside a NaN

  we need to preserve the ability to distinguish NaNs and infinities
  which requires that we _not_ have all zeros
  so type fields shouldn't use zeros

  additionally, let's use 48bit pointers 
  todo: this'll likely bite me in the ass for systems work

  64 bit systems are 48 bits address spaces
  52 - 48 = 4  possible bits for "native" pointer types
          = 15 choices (0 is unavailable due to distinguishablity)

  additonally, a pool of ids is set aside for _offsets_
  we can build object pools, to alleviate pointer hopping

  a pool is a (growable?) linear array of fixed size units
  a "reference" is a index to a pool, and an offset within
  pools can handle memory however they want to, but must have unique pool ids

  a handle is a type + pool_id + offset
  256 possible pool ids, this is more like page tables, than arbitrary makes

  todo: how can we represent an _actual_ 64byte int?
  definitely need (define flag 0xFFFFFFFFFFFFFFFF)

  we can do the same thing we do for cons pools
  a 64byte int (or arbitrary int maybe?) is a handle to an integer pool

                s | 52 bits (exponent bits are elided)

  +inf          0   00000000 .... 000000 00
  -inf          1   00000000 .... 000000 00
  NaN           0   00000000 .... 000000 01
  NaNq          0   10000000 .... 000000 01
                                          
  PTR           1   0ttt address
  malloc        1   0001 aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
  pool          1   0010 aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
  primitive     1   0011 aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
  etc.              0000 = -infinity / NaN
                    1000 = NaNq

                FFF      poolid           offset
  HND           1   1ttt pppppppppppppppp oooooooooooooooooooooooooooooooo
  cons pool     1   1000 pppppppppppppppp oooooooooooooooooooooooooooooooo
  symbol pool   1   1001 pppppppppppppppp oooooooooooooooooooooooooooooooo
  string pool   1   1010 pppppppppppppppp oooooooooooooooooooooooooooooooo
  etc.              0000 = -infinity / NaN
                    1000 = NaNq
  
  BOX           0   tttt aux              data
  boolean       0   0001 0000000000000000 0000000000000000000000000000000b
  character     0   0010 0000000000000000 000000000000000000000000cccccccc
  integer       0   0011 0000000000000000 dddddddddddddddddddddddddddddddd
  double        0   0100 0000000000000000 dddddddddddddddddddddddddddddddd
  error         0   1110 0000000000000000 dddddddddddddddddddddddddddddddd
  nil           0   1111 1111111111111111 11111111111111111111111111111111
  etc.              0000 = +inifinity / NaN
                    1000 = NaNq

  7  pointer types, 48 bit address
  7  handle types, 16bit pool id, 32bit offset
  14 box types, max 7 byte payload per type

  cons pool:
    linear array of value_t
    car cdr car cdr car cdr

  initialized with all nil (write all ones to the whole block)
*/

/* fixed known globals; extern'd in header */ 

value_t vnan   = (value_t)((uint64_t)0x7FF0000000000001LL);
value_t vnanq  = (value_t)((uint64_t)0x7FF1000000000001LL);
value_t vpinf  = (value_t)((uint64_t)0x7FF0000000000000LL);
value_t vninf  = (value_t)((uint64_t)0xFFF0000000000000LL);

value_t vnil   = (value_t)((uint64_t)0xFFFFFFFFFFFFFFFFLL);
value_t vtrue  = (value_t)((uint64_t)0x7FF1000000000001LL);
value_t vfalse = (value_t)((uint64_t)0x7FF1000000000000LL);

// local forward decls

/* boxes */
static value_t    make_boxed(box_type_t type, uint16_t aux, box_data_t value);
static bool       is_boxed(box_type_t type, value_t v);
static box_type_t boxed_type(value_t v);
static uint16_t   boxed_aux(value_t v);
static box_data_t boxed_data(value_t v);

/* pointers */
static value_t    make_pointer(context_p ctxt, ptr_type_t type, void* addr);
static bool       is_pointer(ptr_type_t type, value_t v);
static ptr_type_t pointer_type(value_t v);
static void*      pointer_addr(value_t v);

/* handles */
static value_t    make_handle(context_p ctxt, hnd_type_t type, uint16_t aux, uint32_t offset);
static bool       is_handle(hnd_type_t type, value_t v);
static hnd_type_t handle_type(value_t v);
static uint16_t   handle_aux(value_t v);
static uint32_t   handle_offset(value_t v);

/* pools and the ctxt */

context_p alloc_context(int initial_size) {
  context_p ctxt = malloc(sizeof(context_t));

  int size      = initial_size * 2 * sizeof(value_t);
  value_t *pool = malloc(size);
  if (pool == NULL) {
    fprintf(stderr, "out of memory!\n");
    exit(1);
  }

  /* initialize to nil (all 0xFF) */
  memset(pool, 0xFF, size);

  ctxt->cons_pool_size  = 1;
  ctxt->cons_pool_limit = initial_size;
  ctxt->cons_pool_ptr   = pool;
  ctxt->cons_free_list  = make_handle(ctxt, HND_CONS, 0, 0);

  size = initial_size;
  char *buffer = malloc(size);
  if (buffer == NULL) {
    fprintf(stderr, "out of memory!\n");
    exit(1);
  }
  
  ctxt->string_buffer_limit  = initial_size;
  ctxt->string_buffer_offset = 0;
  ctxt->string_buffer_ptr    = buffer;

  return ctxt;
}

static value_t alloc_cons(context_p ctxt, value_t car, value_t cdr) {
  int index = ctxt->cons_pool_size;
  
  ctxt->cons_pool_ptr[index]     = car;
  ctxt->cons_pool_ptr[index + 1] = cdr;
  ctxt->cons_pool_size          += 2;

  return make_handle(ctxt, HND_CONS, 0, index);
}

/* comparisons */

/** exact value equality */
bool value_exact(value_t a, value_t b) {
  return a.as_uint64 == b.as_uint64;
}

/** literal equality */
bool is_vtruth(value_t v) {
  return value_exact(v, vtrue);
}

/** literal equality */
bool is_vfalse(value_t v) {
  return value_exact(v, vfalse);
}

/** scheme-like predicate: everything but #f is truthy */
bool is_truthy(value_t v) {
  return !is_vfalse(v);
}

/** scheme-like predicate: only #f is falsey */
bool is_falsey(value_t v) {
  return is_vfalse(v);
}

/* doubles */

#define NOT_DOUBLE_MASK 0x7FF0000000000000
#define NOT_NANINF_MASK 0x0009000000000000

value_t make_double(context_p, double v) {
  return (value_t)v;
}

double as_double(value_t v) {
  return v.as_double;
}

bool is_double(value_t v) {
  return
    ((v.as_uint64 & NOT_DOUBLE_MASK) != NOT_DOUBLE_MASK) ||
    (((v.as_uint64 & NOT_NANINF_MASK) >> 48) == 0)       ||
    (((v.as_uint64 & NOT_NANINF_MASK) >> 48) == 8);
}

bool is_nan(value_t v) {
  return value_exact(v, vnan) || value_exact(v, vnanq);
}

bool is_inf(value_t v) {
  return value_exact(v, vpinf) || value_exact(v, vninf);
}

/* integers */

value_t make_integer(context_p, uint32_t value) {
  return make_boxed(BOX_INTEGER, 0, (box_data_t)value);
}

bool is_integer(value_t v) {
  return is_boxed(BOX_INTEGER, v);
}
  
uint32_t as_integer(value_t v) {
  return boxed_data(v).as_uint32;
}

/* floats */

value_t make_float(context_p, float value) {
  return make_boxed(BOX_FLOAT, 0, (box_data_t)value);
}

bool is_float(value_t v) {
  return is_boxed(BOX_FLOAT, v);
}

float as_float(value_t v) {
  return boxed_data(v).as_float;
}

/* chars */

value_t make_character(context_p, char value) {
  return make_boxed(BOX_CHARACTER, 0, (box_data_t)(uint32_t)value);
}

bool is_character(value_t v) {
  return is_boxed(BOX_CHARACTER, v);
}

char as_character(value_t v) {
  return (char)boxed_data(v).as_uint32;
}

value_t make_error(context_p, uint32_t code) {
  return make_boxed(BOX_ERROR, 0, (box_data_t)code);
}

bool is_error(value_t v) {
  return is_boxed(BOX_ERROR, v);
}

/* strings */

value_t make_string(context_p ctxt, char *str, int len) {
  int offset = ctxt->string_buffer_offset;
  memcpy(ctxt->string_buffer_ptr + offset, str, len);

  // should already be the case, but ensure
  ctxt->string_buffer_ptr[offset + len] = '\0';
  ctxt->string_buffer_offset += len;
  
  // TODO: error handling
  return make_handle(ctxt, HND_STRING, len, offset);
}

bool is_string(value_t v) {
  return is_handle(HND_STRING, v);
}

char* string_ptr(context_p ctxt, value_t v) {
  int offset = handle_offset(v);
  return ctxt->string_buffer_ptr + offset;
}

uint32_t string_len(context_p, value_t v) {
  return handle_aux(v);
}

/* symbols */

/* errors */

/* cons cells */

value_t make_cons(context_p ctxt, value_t car, value_t cdr) {
  return alloc_cons(ctxt, car, cdr);
}

bool is_cons(value_t v) {
  return is_handle(HND_CONS, v);
}

bool is_nil(value_t v) {
  return value_exact(v, vnil);
}

value_t car(context_p ctxt, value_t hnd) {
  int index = handle_offset(hnd);
  return ctxt->cons_pool_ptr[index];
}

value_t cdr(context_p ctxt, value_t hnd) {
  int index = handle_offset(hnd);
  return ctxt->cons_pool_ptr[index + 1];
}

void set_car(context_p ctxt, value_t hnd, value_t v) {
  int index = handle_offset(hnd);
  ctxt->cons_pool_ptr[index] = v;
}

void set_cdr(context_p ctxt, value_t hnd, value_t v) {
  int index = handle_offset(hnd);
  ctxt->cons_pool_ptr[index + 1] = v;
}

/* buffers */

/* vectors */

/* procs */

/* boxes */

#define SPECIAL_MASK    0xFFF0000000000000
#define BOX_MASK        0x7FF0000000000000
#define BOX_TYPE_MASK   0x000F000000000000
#define BOX_AUX_MASK    0x0000FFFF00000000
#define BOX_DATA_MASK   0x00000000FFFFFFFF

static value_t make_boxed(box_type_t type, uint16_t aux, box_data_t value) {
  value_t v;
  v.as_uint64 = BOX_MASK   |
    ((uint64_t)type << 48) |
    ((uint64_t)aux << 32)  |
    ((uint64_t)value.as_uint32);

  return v;
}

static bool is_boxed(box_type_t type, value_t v) {
  uint64_t type_mask = (uint64_t)type << 48;
  return (v.as_uint64 & (SPECIAL_MASK | BOX_TYPE_MASK)) == (BOX_MASK | type_mask);
}

static box_type_t boxed_type(value_t v) {
  return ((box_type_t)((v.as_uint64 & BOX_TYPE_MASK) >> 48));
}

static uint16_t boxed_aux(value_t v) {
  return ((uint16_t)((v.as_uint64 & BOX_AUX_MASK) >> 32));
}

static box_data_t boxed_data(value_t v) {
  return (box_data_t)((uint32_t)(v.as_uint64 & BOX_DATA_MASK));
}

/* arbitrary pointers */

#define PTR_MASK        0xFFF0000000000000
#define PTR_TYPE_MASK   0x000F000000000000
#define PTR_ADDR_MASK   0x0000FFFFFFFFFFFF

static value_t make_pointer(context_p, ptr_type_t type, void* addr) {
  uint64_t as_int = (uint64_t)addr;

  if ((as_int & PTR_ADDR_MASK) != as_int) {
    fprintf(stderr, "unrepresentable address: %lx\nbailing!\n", as_int);
    exit(1);
  }
  
  value_t v;
  v.as_uint64 = PTR_MASK | as_int | ((uint64_t)(type & PTR_TYPE_MASK) << 48);

  return v;
}

static bool is_pointer(ptr_type_t type, value_t v) {
  uint64_t type_mask = (uint64_t)type << 48;
  return (v.as_uint64 & (PTR_MASK | PTR_TYPE_MASK)) == (PTR_MASK | type_mask);
}

static ptr_type_t pointer_type(value_t v) {
  return ((ptr_type_t)((v.as_uint64 & PTR_TYPE_MASK) >> 48));
}

static void* pointer_addr(value_t v) {
  return (void *)(v.as_uint64 & PTR_ADDR_MASK);
}

/* handles */

#define HND_AUX_MASK   0x0000FFFF00000000
#define HND_OFFSET_MASK 0x00000000FFFFFFFF

static value_t make_handle(context_p, hnd_type_t type, uint16_t aux, uint32_t offset) {
  value_t v;
  v.as_uint64 = PTR_MASK | offset | ((uint64_t)type << 48) | ((uint64_t)aux << 32);

  return v;
}

static bool is_handle(hnd_type_t type, value_t v) {
  uint64_t type_mask = (uint64_t)type << 48;
  return (v.as_uint64 & (PTR_MASK | PTR_TYPE_MASK)) == (PTR_MASK | type_mask);
}

static hnd_type_t handle_type(value_t v) {
  return ((hnd_type_t)((v.as_uint64 & PTR_TYPE_MASK) >> 48));
}

static uint16_t handle_aux(value_t v) {
  return ((uint16_t)((v.as_uint64 & HND_AUX_MASK) >> 32));
}

static uint32_t handle_offset(value_t v) {
  return (uint32_t)(v.as_uint64 & HND_OFFSET_MASK);
}
