#include <stdio.h>	/* printf */
#include <stdlib.h>	/* malloc */

#include "variable_size_allocator.h"

static void TestVSA1(void);
static void TestVSA2(void);

int main()
{
	TestVSA1();
	TestVSA2();
	
	return 0;
}


static void TestVSA1(void)
{
	long *b1 = NULL;
	long *b2 = NULL;
	long *b3 = NULL;
	
	long *ptr = (long *)calloc(250, 1);
	vsa_t *vsa = VSAInitialize(ptr, 250);
	
	b1 = (long *)VSAAlloc(vsa, 40);
	b2 = (long *)VSAAlloc(vsa, 80);
	b3 = (long *)VSAAlloc(vsa, 48);
	
	/* accessing the allocated block minus 8 bytes will reach the header of the block where its size is stored */
	printf(" %ld\n", *(b1 - 1));
	printf(" %ld\n", *(b2 - 1));
	printf(" %ld\n", *(b3 - 1));
	
	VSAFree(b1);
	VSAFree(b2);
	printf("lca %ld\n", VSALargestChunkAvailable(vsa));
	printf(" %ld\n", *(b1 - 1));
	printf(" %ld\n", *(b2 - 1));
	printf(" %ld\n\n", *(b3 - 1));
	
	
	printf(" %p\n", (b1 - 1));
	printf(" %p\n", (long *)vsa + 1);
	
	b2 = (long *)VSAAlloc(vsa, 104);
	printf(" %ld\n", *(b2 - 1));
	printf(" %p\n", (b2 - 1));
	

	free(ptr);
}


static void TestVSA2(void)
{

	long *b1 = NULL;
	long *b2 = NULL;
	long *b3 = NULL;
	long *b4 = NULL;
	long *b5 = NULL;
	long *b6 = NULL;
	
	long *ptr = (long *)malloc(1000);
	vsa_t *vsa = VSAInitialize(ptr, 1000);
	
	printf("lca %ld\n", VSALargestChunkAvailable(vsa));
	
	b1 = (long *)VSAAlloc(vsa, 30);
	b2 = (long *)VSAAlloc(vsa, 40);
	b3 = (long *)VSAAlloc(vsa, 50);
	b4 = (long *)VSAAlloc(vsa, 60);
	b5 = (long *)VSAAlloc(vsa, 70);
	b6 = (long *)VSAAlloc(vsa, 80);

	printf("lca %ld\n", VSALargestChunkAvailable(vsa));
	
	VSAFree(b1);
	VSAFree(b2);
	
	VSAFree(b4);
	VSAFree(b5);
	
	printf(" %ld\n", *(b1 - 1));
	printf(" %ld\n", *(b2 - 1));
	printf(" %ld\n", *(b3 - 1));
	printf(" %ld\n", *(b4 - 1));
	printf(" %ld\n\n", *(b5 - 1));
	
	
	printf(" %p\n", (b4 - 1));
	b2 = (long *)VSAAlloc(vsa, 100);
	printf(" %ld\n", *(b2 - 1));
	printf(" %p\n", (b2 - 1));
	
	if(b2 != b4)
	{
		printf("test 2 failed\n");
		return;
	}
	
	b5 = (long *)VSAAlloc(vsa, 400);
	
	printf("%ld\n", VSALargestChunkAvailable(vsa));
	
	free(ptr);
	
}

