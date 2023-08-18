#include "chashmap.h"


chashmap_t* chashmap_new(chashmap_sizer_t k_size, chashmap_sizer_t v_size)
{
    chashmap_t* hashmap = malloc(sizeof(chashmap_t));
    if (NULL == hashmap)
        PANIC("unable to allocate memory for hashmap");

    if (NULL == k_size || NULL == v_size)
        PANIC("key or value sizer is NULL");

    hashmap->size = 0;
    hashmap->cap = CHASHMAP_CAP;
    hashmap->k_size = k_size;
    hashmap->v_size = v_size;
    hashmap->slots = malloc(hashmap->cap * sizeof(chashmap_slot_t));


    if (NULL == hashmap->slots)
        PANIC("chashmap_new: unable to allocate memory for hashmap slots");

    for (size_t i = 0; i < hashmap->cap; ++i)
    {
        chashmap_slot_t* slot = &hashmap->slots[i];

        slot->k = NULL;
        slot->v = NULL;
        slot->next = NULL;
    }

    return hashmap;
}

void chashmap_free(chashmap_t *hashmap)
{
    if (NULL == hashmap)
        PANIC("chashmap_free: hashmap is NULL");

    if (NULL != hashmap->slots)
    {
        for (size_t i = 0; i < hashmap->cap; ++i)
        {
            chashmap_slot_t* slot = &hashmap->slots[i];

            if (NULL == slot->v) continue;

            free(slot->v);
            free(slot->k);
        }
    }

    free(hashmap);
}

   
void chashmap_realloc(chashmap_t* hashmap)
{
    if (NULL == hashmap)
        PANIC("chashmap_realloc: hashmap is NULL");

    if (hashmap->size == 0)
        return;

    chashmap_slot_t* cur_slot = hashmap->slots;
    chashmap_slot_t* last_slot = &hashmap->slots[hashmap->cap - 1];
    while (cur_slot <= last_slot)
    {
        chashmap_slot_t* other_slot = NULL;
        chashmap_slot_t switch_slot = {
            .v = NULL
        };

        if (NULL == cur_slot->v)
        {
            ++cur_slot;
            continue;
        }

        uint32_t hash = murmur3_32_hash(cur_slot->k);
        uint32_t index = hash % hashmap->cap;

        other_slot = &hashmap->slots[index];
        
        if (NULL != other_slot->v)
        {
            switch_slot = *other_slot;
            *other_slot = *cur_slot;
            *cur_slot = switch_slot;
        }
        else 
        {
            *other_slot = *cur_slot;
        }

        if (NULL == switch_slot.v)
            ++cur_slot;
    }
}


void chashmap_insert(chashmap_t* hashmap, void* k, void* v)
{
    if (NULL == hashmap)
        PANIC("chashmap_insert: hashmap is NULL");

    if (NULL == k)
        PANIC("chashmap_insert: key is NULL");
   
    if (NULL == v)
        PANIC("chashmap_insert: value is NULL");


    if (hashmap->size == hashmap->cap)
    {
        LOG_DBG("chashmap_insert: realloc hashmap");

        hashmap->cap *= 2;
        hashmap->slots = realloc(hashmap->slots, hashmap->cap * sizeof(chashmap_slot_t));
        chashmap_realloc(hashmap);
    }

    uint32_t hash = murmur3_32_hash(k);
    uint32_t index = hash % hashmap->cap;
    size_t k_bytes = hashmap->k_size(k);
    size_t v_bytes = hashmap->v_size(v);

    chashmap_slot_t* slot = chashmap_get_insert_slot(hashmap, index);
    slot->k = malloc(k_bytes);
    slot->v = malloc(v_bytes);

    memcpy(slot->k, k, k_bytes);
    memcpy(slot->v, v, v_bytes);

    ++hashmap->size;

    LOG_DBG("chashmap_insert: inserted %p (deref=%p) with key=%s into hashmap=%p", v, slot->v, (char*)k, (void*)hashmap);
}

chashmap_slot_t* chashmap_get_insert_slot(chashmap_t* hashmap, size_t index)
{
    chashmap_slot_t* slot = &hashmap->slots[index];

    if (NULL != slot->v)
    {
        LOG_DBG("chashmap_get_insert_slot: NULL != slot->v");

        chashmap_slot_t* last_slot_memb = slot;
        while (NULL != last_slot_memb->next && NULL != last_slot_memb->v)
        {
            last_slot_memb = last_slot_memb->next;
        }

        if (NULL != last_slot_memb->v)
        {
            last_slot_memb->next = malloc(sizeof(chashmap_t));
            last_slot_memb = last_slot_memb->next;
        }

        slot = last_slot_memb;
    }
    
    return slot;
}

void* chashmap_get(chashmap_t* hashmap, void* k)
{
    if (NULL == hashmap)
        PANIC("chashmap_get: hashmap is NULL");
   
    if (NULL == k)
        PANIC("chashmap_get: key is NULL");
   
    uint32_t hash = murmur3_32_hash(k);
    uint32_t index = hash % hashmap->cap;
    chashmap_slot_t* slot = &hashmap->slots[index];

    // TODO: разобраться, почему постоянно вылетает эта ошибка
    if (NULL == slot->v)
        PANIC("chashmap_get: value by key \"%s\" doesn't exists", (char*)k);

    size_t slot_k_size = hashmap->k_size(slot->k);
    size_t k_size = hashmap->k_size(k);

    while (NULL != slot && NULL != slot->v && slot_k_size == k_size && memcmp(slot->k, k, k_size))
    {
        slot_k_size = hashmap->k_size(slot->k);
        k_size = hashmap->k_size(k);
        slot = slot->next;
    }

    if (NULL == slot || NULL == slot->v)
        PANIC("chashmap_get: ! value by key \"%s\" doesn't exists", (char*)k);

    return slot->v;
}
