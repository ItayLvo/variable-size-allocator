#ifndef __VSA_H__
#define __VSA_H__

#include <stddef.h> /* size_t */

typedef struct variable_size_allocator vsa_t;

/*
description: Initializes an allocator.
input: pointer to memory pool, memory pool size.
return: a pointer to an allocator. if failed, returns NULL.
time complexity: O(1)
space complexity: O(1)
*/
vsa_t *VSAInitialize(void *memory_pool, size_t pool_size);

/*
description: Allocates a block of memory of size 'block_size'. will allocate the first block that matches the size requirement ("first fit").
input: Pointer to allocator, size_t block_size the size in bytes of the block requested.
return: pointer to memory allocated, aligned. if failed returns NULL
time complexity: O(n) (n = number of allocated chunks, maximum being memory pool size)
space complexity: O(1)
*/
void *VSAAlloc(vsa_t *allocator, size_t block_size);

/*
description: Frees a specific block of memory from the allocator.
input: pointer to memory block. If NULL does nothing.
return: void.
time complexity: O(1)
space complexity: O(1)
*/
void VSAFree(void *mem_to_free_ptr);

/*
description: Returns the size of largest available-for-allocation continuous block in bytes.
input: pointer to an allocator.
return: size of largest block posible for allocation.
time complexity: O(n)
space complexity: O(1)
*/
size_t VSALargestChunkAvailable(vsa_t *vsa);

#endif /* __VSA_H__ */
