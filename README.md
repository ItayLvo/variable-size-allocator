# Variable-Size Allocator
The Variable-size Allocator is a custom memory allocator written in C, designed to manage variable-sized memory blocks within a fixed memory pool. This implementation efficiently handles memory allocation, deallocation, and defragmentation, making it a useful component in systems that require fine handling of memory management.



## Overview
The Variable-size Allocator project provides a mechanism to handle allocation requests within a memory block provided by the user.
The VSA implementation is focused on efficiency for both memory usage and time complexity of operations, and does not use any additional memory to handle the memory pool.



## API

```c
typedef struct variable_size_allocator vsa_t;
```
Typedef for the allocator.

```c
vsa_t *VSAInitialize(void *memory_pool, size_t pool_size);
```
Description: Initializes the VSA.
Parameters: Pointer to allocated memory block, pool size.
Returns: Pointer to the allocated block. If failed, returns NULL.

```c
void *VSAAlloc(vsa_t *allocator, size_t block_size);
```
Description: Allocates a block of memory of size 'block_size'. Will allocate the first block that matches the size requirement ("first fit").
Parameters: Pointer to allocator, requested block size in bytes.
Returns: Pointer to the allocated memory block. If failed, returns NULL.


```c
void VSAFree(void *mem_to_free_ptr);
```
Description: Frees a specific block of memory from the allocator.
Parameters: Pointer to a previously allocated memory block. If NULL does nothing.
Returns: void.


```c
size_t VSALargestChunkAvailable(vsa_t *vsa);
```
Description: Returns the size of largest available-for-allocation continuous block in bytes.
Parameters: Pointer to an allocator.
Returns: Size of largest continuous block possible for allocation in bytes.



## How it works

The Variable-size Allocator manages memory within a user-provided memory pool. Each allocation is tracked by a hidden "header" attached to each block, which stores the size of the allocated block (positive for free blocks, negative for allocated ones). This design allows the allocator to navigate through memory blocks, locate free space, and handle both allocation and deallocation efficiently.

**Initialization:**

The allocator is initialized using a fixed-size memory pool, which is aligned for performance. The header stores the overall pool size.

**Allocation and Deallocation:**

Allocation: When a user requests a block, the allocator searches for the first block that meets the size requirement (a "first fit" approach).

Block Header: Every memory block includes a hidden header, storing the block size:
- A positive value indicates the block is free.
- A negative value indicates the block is allocated.

Defragmentation: During an allocation attempt, if the allocator finds a free block that isn’t large enough, it attempts to merge contiguous free blocks to create sufficient space. The defragmentation process only happens if required and will not continue if sufficient space was created. This ensures no time is wasted on unnecessary defragmentation.

Freeing Memory: When `VSAfree()` is called, the allocator reads the block size from the header to correctly release the exact amount of memory.


**Memory Alignment**
All allocations (and the memory pool itself) are aligned to the word size, ensuring that data within the memory blocks adheres to alignment requirements and optimizing performance.



## Usage example:
```c
int main() {
    char memory_pool[1024];  // Define a memory pool

    // Initialize allocator
    vsa_t *allocator = VSAInitialize(memory_pool, sizeof(memory_pool));

    // Allocate memory blocks
    void *block1 = VSAAlloc(allocator, 128);
    void *block2 = VSAAlloc(allocator, 64);

    // Free a memory block
    VSAFree(block1);

    // Check the largest available contiguous block
    size_t largest_chunk = VSALargestChunkAvailable(allocator);

    return 0;
}
```
