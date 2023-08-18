#ifndef CUTE_MURMUR3_32_HASH_H
#define CUTE_MURMUR3_32_HASH_H

#include <stddef.h>
#include <string.h>
#include <stdint.h>

#include "clogging/clogging.h"

#define MURMUR3_32_SEED 0x12345678


uint32_t murmur32_scramble(uint32_t k);
uint32_t murmur3_32_hash(const char* str);


#endif // CUTE_MURMUR3_32_HASH_H
