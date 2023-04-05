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
 */

value_t vnan   = (value_t)((uint64_t)0x7FF0000000000001LL);
value_t vnanq  = (value_t)((uint64_t)0x7FF1000000000001LL);
value_t vpinf  = (value_t)((uint64_t)0x7FF0000000000000LL);
value_t vninf  = (value_t)((uint64_t)0xFFF0000000000000LL);

value_t vnil   = (value_t)((uint64_t)0xFFFFFFFFFFFFFFFFLL);
value_t vtrue  = (value_t)((uint64_t)0x7FF1000000000001LL);
value_t vfalse = (value_t)((uint64_t)0x7FF1000000000000LL);

/* comparisons */

/** exact value equality */
inline bool value_exact(value_t a, value_t b) {
  return a.as_uint64 == b.as_uint64;
}

/** literal equality */
inline bool is_vtruth(value_t v) {
  return value_exact(v, vtrue);
}

/** literal equality */
inline bool is_vfalse(value_t v) {
  return value_exact(v, vfalse);
}

/** scheme-like predicate: everything but #f is truthy */
inline bool is_truthy(value_t v) {
  return !is_vfalse(v);
}

/** scheme-like predicate: only #f is falsey */
inline bool is_falsey(value_t v) {
  return is_vfalse(v);
}

/* doubles */

#define NOT_DOUBLE_MASK 0x7FF0000000000000
#define NOT_NANINF_MASK 0x0009000000000000

inline value_t make_double(vm_t* vm, double v) {
  return (value_t)v;
}

inline double as_double(value_t v) {
  return v.as_double;
}

inline bool is_double(value_t v) {
  return
    ((v.as_uint64 & NOT_DOUBLE_MASK) != NOT_DOUBLE_MASK) ||
    (((v.as_uint64 & NOT_NANINF_MASK) >> 48) == 0)       ||
    (((v.as_uint64 & NOT_NANINF_MASK) >> 48) == 8);
}

inline bool is_nan(value_t v) {
  return value_exact(v, vnan) || value_exact(v, vnanq);
}

inline bool is_inf(value_t v) {
  return value_exact(v, vpinf) || value_exact(v, vninf);
}

/* boxed values (not exported) */

#define SPECIAL_MASK    0xFFF0000000000000
#define BOX_MASK        0x7FF0000000000000
#define BOX_TYPE_MASK   0x000F000000000000
#define BOX_AUX_MASK    0x0000FFFF00000000
#define BOX_DATA_MASK   0x00000000FFFFFFFF

typedef union box_data {
  float    as_float;
  int32_t  as_int32;
  uint32_t as_uint32;
} box_data_t;

static inline value_t make_boxed(box_type_t type, uint16_t aux, box_data_t value) {
  value_t v;
  v.as_uint64 = BOX_MASK   |
    ((uint64_t)type << 48) |
    ((uint64_t)aux << 32)  |
    ((uint64_t)value.as_uint32);

  return v;
}

static inline bool is_boxed(box_type_t type, value_t v) {
  uint64_t type_mask = (uint64_t)type << 48;
  return (v.as_uint64 & (SPECIAL_MASK | BOX_TYPE_MASK)) == (BOX_MASK | type_mask);
}

static inline box_type_t boxed_type(value_t v) {
  return ((box_type_t)((v.as_uint64 & BOX_TYPE_MASK) >> 48));
}

static inline uint16_t boxed_aux(value_t v) {
  return ((uint16_t)((v.as_uint64 & BOX_AUX_MASK) >> 32));
}

static inline box_data_t boxed_data(value_t v) {
  return (box_data_t)((uint32_t)(v.as_uint64 & BOX_DATA_MASK));
}

/* integers */

inline value_t make_integer(vm_t* vm, uint32_t value) {
  return make_boxed(BOX_INTEGER, 0, (box_data_t)value);
}

inline bool is_integer(value_t v) {
  return is_boxed(BOX_INTEGER, v);
}
  
inline uint32_t as_integer(value_t v) {
  return boxed_data(v).as_uint32;
}

/* floats */

inline value_t make_float(vm_t* vm, float value) {
  return make_boxed(BOX_FLOAT, 0, (box_data_t)value);
}

inline bool is_float(value_t v) {
  return is_boxed(BOX_FLOAT, v);
}

inline float as_float(value_t v) {
  return boxed_data(v).as_float;
}

/* chars */

inline value_t make_character(vm_t* vm, char value) {
  return make_boxed(BOX_CHARACTER, 0, (box_data_t)(uint32_t)value);
}

inline bool is_character(value_t v) {
  return is_boxed(BOX_CHARACTER, v);
}

inline char as_character(value_t v) {
  return (char)boxed_data(v).as_uint32;
}

inline value_t make_error(vm_t* vm, uint32_t code) {
  return make_boxed(BOX_ERROR, 0, (box_data_t)code);
}

inline bool is_nil(value_t v) {
  return value_exact(v, vnil);
}

#define PTR_MASK        0xFFF0000000000000
#define PTR_TYPE_MASK   0x000F000000000000
#define PTR_ADDR_MASK   0x0000FFFFFFFFFFFF

#define HND_ROOT_MASK   0x0000FFFF00000000
#define HND_OFFSET_MASK 0x00000000FFFFFFFF
