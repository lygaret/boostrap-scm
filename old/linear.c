#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>


#define PINFINITY       0x7FF0000000000000
#define NINFINITY       0xFFF0000000000000
#define NaN             0x7FF0000000000001
#define NaNQ            0x7FF8000000000001

#define NOT_DOUBLE_MASK 0x7FF0000000000000
static inline bool is_double(value v) {
  return ((v.as_uint64 & NOT_DOUBLE_MASK) != NOT_DOUBLE_MASK);
}

#define NIL             0xFFFFFFFFFFFFFFFF

static inline bool is_nil(value v) {
  return v.as_uint64 == NIL;
}

#define PTR_MASK        0xFFF0000000000000
#define PTR_TYPE_MASK   0x000F000000000000
#define PTR_ADDR_MASK   0x0000FFFFFFFFFFFF

#define HND_ROOT_MASK   0x0000FFFF00000000
#define HND_OFFSET_MASK 0x00000000FFFFFFFF

#define BOX_MASK        0x7FF0000000000000
#define BOX_SIZE_MASK   0x000F000000000000
#define BOX_TYPE_MASK   0x0000FFFF00000000
#define BOX_DATA_MASK   0x00000000FFFFFFFF

static inline value make_pointer(void *pointer, ptr_type_t type) {
  value v;
  v.as_uint64 =
    PTR_MASK               |
    ((uint64_t)type << 48) |
    ((uint64_t)pointer);

  return v;
}

static inline bool is_pointer(value v) {
  return ((v.as_uint64 & PTR_MASK) == PTR_MASK);
}

static inline ptr_type_t ptr_type(value v) {
  return (ptr_type_t)((v.as_uint64 & PTR_TYPE_MASK) >> 48);
}

static inline void *ptr_addr(value v) {
  return (void *)(v.as_uint64 & PTR_ADDR_MASK);
}

static inline value make_handle(short root, uint32_t offset, hnd_type_t type) {
  value v;
  v.as_uint64 =
    PTR_MASK               |
    ((uint64_t)type << 48) | 
    ((uint64_t)root << 32) |
    ((uint64_t)offset);

  return v;
}

static inline bool is_handle(value v) {
  return is_pointer(v) && ((ptr_type(v) & 8) != 0);
}

static inline hnd_type_t hnd_type(value v) {
  return (hnd_type_t)(ptr_type(v));
}

static inline uint16_t hnd_root(value v) {
  return (uint16_t)((v.as_uint64 & HND_ROOT_MASK) >> 32);
}

static inline uint32_t hnd_offset(value v) {
  return (uint32_t)(v.as_uint64 & HND_OFFSET_MASK);
}

static inline bool is_box(value v) {
  return ((v.as_uint64 & PTR_MASK) == BOX_MASK);
}

static inline uint8_t box_size(value v) {
  return (uint8_t)((v.as_uint64 & BOX_SIZE_MASK) >> 48);
}

static inline box_type_t box_type(value v) {
  return (box_type_t)((v.as_uint64 & BOX_TYPE_MASK) >> 32);
}

static inline uint32_t box_data(value v) {
  return (uint32_t)(v.as_uint64 & BOX_DATA_MASK);
}

void main() {
  char *buffer = malloc(16);
  value v = make_pointer((void*)buffer, PTR_MALLOC);

  printf("pointer size: %d\n", sizeof(void*));
  printf("actual addr: %p\n", buffer);
  printf("is_pointer: %d\n", is_pointer(v));
  printf("is_hnd: %d\n", is_handle(v));
  printf("ptr_type: %d\n", ptr_type(v));
  printf("ptr_value: %p\n", ptr_addr(v));

  v = make_handle(254, 0x7FFF1234, HND_CONS);
  printf("\n\n");
  printf("is_pointer: %d\n", is_pointer(v));
  printf("is_hnd: %d\n", is_handle(v));
  printf("hnd_type: %d\n", hnd_type(v));
  printf("hnd_root: %d\n", hnd_root(v));
  printf("hnd_offset: %x\n", hnd_offset(v));
}
