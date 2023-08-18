#ifndef CUTE_VM_HASHMAP_H
#define CUTE_VM_HASHMAP_H

#include <stddef.h>
#include <malloc.h>
#include <string.h>

#include "clogging/clogging.h"
#include "murmur3_32_hash.h"

#define CHASHMAP_CAP 100


typedef size_t (*chashmap_sizer_t)(void*);

typedef struct chashmap_slot
{
    void* k;
    void* v;
    struct chashmap_slot* next;
} chashmap_slot_t;

typedef struct
{
    size_t cap;
    size_t size;

    void** keys;
    void** values;

    chashmap_sizer_t k_size;
    chashmap_sizer_t v_size;

    chashmap_slot_t* slots;
} chashmap_t;


chashmap_t* chashmap_new(chashmap_sizer_t k_size, chashmap_sizer_t v_size);
void chashmap_free(chashmap_t* hashmap);

void chashmap_realloc(chashmap_t* hashmap);

void chashmap_insert(chashmap_t* hashmap, void* k, void* v);
chashmap_slot_t* chashmap_get_insert_slot(chashmap_t* hashmap, size_t index);

void* chashmap_get(chashmap_t* hashmap, void* k);


static size_t chashmap_ptr_size(void* ptr)
{ return sizeof(ptr); }

static size_t chashmap_cstr_size(void* str)
{ return strlen((char*)str) + 1; }


#endif // CUTE_VM_HASHMAP_H
