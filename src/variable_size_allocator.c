#include <stddef.h>	/* size_t */
#include <stdlib.h>	/* malloc */
#include <assert.h>	/* assert */

#include "variable_size_allocator.h"		/* VSA functions, typedefs */

/* macro for word size - 8 bytes on my architecture */
#define WORD_SIZE sizeof(void *)
/* macro for the header size of each memory block - i'm using 8 bytes (long) */
#define BLOCK_HEADER_SIZE sizeof(void *)


/* memory pool managerial struct typedef */
struct variable_size_allocator
{
	size_t pool_size;
};


/* static functions forward declarations */
static void *AlignMemoryPoolStart(void *memory_pool);
static size_t AlignPoolSize(size_t pool_size);
static size_t AlignBlockSize(size_t block_size);
static void *MergeFreeBlocks(vsa_t *allocator, long *current_block, size_t requested_block_size);
static long *AdvanceToNextBlockHeader(long *block);
static size_t CalculateCurrentOffset(vsa_t *allocator, long *current_block);
static void *FSASetNewBlock(long *dest, long current_block_size, size_t requested_block_size);
static long GetAbsValue(long num);




vsa_t *VSAInitialize(void *memory_pool, size_t pool_size)
{
	vsa_t *allocator = NULL;	
	long *first_block = NULL;
	size_t pool_aligned_penalty = 0;

	assert(memory_pool);
	
	/* align pool starting address and calculate remaining size */
	allocator = (vsa_t *)(AlignMemoryPoolStart(memory_pool));
	pool_aligned_penalty = (char *)allocator - (char *)memory_pool;
	allocator->pool_size = AlignPoolSize(pool_size - pool_aligned_penalty);
	
	/* set first block header */
	first_block = (long *)((char *)memory_pool + sizeof(vsa_t));
	*first_block = pool_size - sizeof(vsa_t) - BLOCK_HEADER_SIZE;
	
	return allocator;
}



void *VSAAlloc(vsa_t *allocator, size_t block_size)
{
	long *current_block = (long *)((char *)allocator + sizeof(vsa_t));
	long current_block_size = 0;
	size_t current_offset = (char *)current_block - (char *)allocator;
	size_t valid_pool_size = allocator->pool_size - BLOCK_HEADER_SIZE;

	block_size = AlignBlockSize(block_size);
	
	while (current_offset < valid_pool_size)
	{
		current_block_size =  *current_block;
		
		/* if current block size is negative => the current block is already alloc'd. move pointer to next block */
		if (current_block_size < 0)
		{
			current_block = AdvanceToNextBlockHeader(current_block);
		}
		/* else, current block size is positive => check if there's enough room to fit user request */
		else
		{
			if (current_block_size < (block_size + BLOCK_HEADER_SIZE)) /* current block is avaliable but too small for requested block size */
			{
				/* call Merge function to try to de-fragment and allocate after defragmentation */
				void *allocated_merged_block = MergeFreeBlocks(allocator, current_block, block_size);
				if (NULL != allocated_merged_block)
				{
					return allocated_merged_block;
				}
			}
			else	/* current block is avaliable and is sufficient in size */
			{
				return FSASetNewBlock(current_block, current_block_size, block_size);
			}
			
			current_block = (long *)((char *)current_block + current_block_size + BLOCK_HEADER_SIZE);
		}
		
		current_offset = CalculateCurrentOffset(allocator, current_block);
	}
	
	/* if left while loop and didn't return new block => no fitting block available, return NULL */
	return NULL;	
}



void VSAFree(void *mem_to_free_ptr)
{
	*(long *)((long *)mem_to_free_ptr - 1) *= -1;
}



size_t VSALargestChunkAvailable(vsa_t *allocator)
{
	long *start_block = (long *)((char *)allocator + sizeof(vsa_t));
	long current_block_size = 0;
	long consecutive_block_sum = 0;
	long *block_runner = start_block;
	size_t current_offset = (char *)start_block - (char *)allocator;
	size_t max_chunk = 0;
	size_t valid_pool_size = allocator->pool_size - BLOCK_HEADER_SIZE;
	
	while (current_offset < valid_pool_size)
	{
		current_block_size = *block_runner;
		
		if (current_block_size > 0)
		{
			consecutive_block_sum += (*block_runner + BLOCK_HEADER_SIZE);
			block_runner = AdvanceToNextBlockHeader(block_runner);
			current_offset = CalculateCurrentOffset(allocator, block_runner);
			
			if (current_offset < valid_pool_size)	/* to prevent dereferening outside of pool */
			{
				current_block_size = *block_runner;	
			}
		}
		else
		{
			if (consecutive_block_sum > max_chunk)
			{
				max_chunk = consecutive_block_sum;
			}
			
			start_block = AdvanceToNextBlockHeader(start_block);
			block_runner = start_block;
			current_offset = CalculateCurrentOffset(allocator, start_block);
		}
		if (consecutive_block_sum > max_chunk)
		{
			max_chunk = consecutive_block_sum;
		}
		consecutive_block_sum = 0;
	}
	
	return max_chunk;
}



/***************** static helper functions *****************/


static void *MergeFreeBlocks(vsa_t *allocator, long *current_block, size_t requested_block_size)
{
	long *block_runner = current_block;
	long current_block_size = *block_runner;
	size_t valid_pool_size = allocator->pool_size - BLOCK_HEADER_SIZE;
	size_t consecutive_block_sum = 0;
	size_t current_offset = CalculateCurrentOffset(allocator, block_runner);
	
	while (current_block_size > 0 &&
		current_offset < valid_pool_size &&
		consecutive_block_sum < (requested_block_size + BLOCK_HEADER_SIZE))
	{
		consecutive_block_sum += (*block_runner + BLOCK_HEADER_SIZE);
		block_runner = AdvanceToNextBlockHeader(block_runner);
		current_offset = CalculateCurrentOffset(allocator, block_runner);
		if (current_offset < valid_pool_size)	/* to prevent dereferening outside of pool */
		{
			current_block_size = *block_runner;	
		}
	}
	
	// if requested block size is reached - return new "merged" memory block
	if (consecutive_block_sum >= (requested_block_size + BLOCK_HEADER_SIZE))
	{
		return FSASetNewBlock(current_block, consecutive_block_sum, requested_block_size);
	}
	else if (current_offset >= valid_pool_size) /* reached end of pool, no fitting block found (current_offset >= allocator->pool_size) */
	{
		return NULL;
	}
	else if (current_block_size < 0) 	/* current consecutive series of blocks is not sufficient, continue trying and merging blocks */
	{
		current_block = block_runner;
		current_block = AdvanceToNextBlockHeader(current_block);
	}
	
	return NULL;
}



static void *FSASetNewBlock(long *dest, long current_block_size, size_t requested_block_size)
{
	if (current_block_size == (requested_block_size + BLOCK_HEADER_SIZE))	/* exact match in size, don't create header after new block */
	{
		*dest = GetAbsValue(current_block_size) * -1;
		return (void *)((char *)dest + BLOCK_HEADER_SIZE);
	}
	else	/* current block is bigger than requestrd size, create new header after new block */
	{
		long *new_header = NULL;
		size_t space_remainder = current_block_size - (requested_block_size + BLOCK_HEADER_SIZE);
					
		*dest = GetAbsValue(requested_block_size) * -1;
		new_header = (long *)((char *)dest + requested_block_size + BLOCK_HEADER_SIZE);
		*new_header = space_remainder;
					
		return ((void *)((char *)dest + BLOCK_HEADER_SIZE));
	}
}


static long *AdvanceToNextBlockHeader(long *block)
{
	return (long *)((char *)block + GetAbsValue(*block) + BLOCK_HEADER_SIZE);
}


static void *AlignMemoryPoolStart(void *pool)
{
	char *runner = NULL;
	assert(pool);
	
	runner = (char *)pool;
	while ((size_t)runner % WORD_SIZE != 0)
	{
		++runner;
	}
	
	return (void *)runner;
}


static size_t AlignPoolSize(size_t pool_size)
{
	while (pool_size % WORD_SIZE != 0)
	{
		--pool_size;
	}
	
	return pool_size;
}

static size_t AlignBlockSize(size_t block_size)
{
	if (block_size < sizeof(void *))
	{
			block_size = sizeof(void *);
			return block_size;
	}
	
	if (block_size % WORD_SIZE == 0)
	{
		return block_size;
	}
	
	block_size = (((block_size / WORD_SIZE) + 1) * WORD_SIZE);
	
	return block_size;
}

static size_t CalculateCurrentOffset(vsa_t *allocator, long *current_block)
{
	return (char *)current_block - (char *)allocator;
}

static long GetAbsValue(long num)
{
	return num < 0 ? (num * -1) : num;
}
