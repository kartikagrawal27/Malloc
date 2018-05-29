/**
 * Machine Problem 4
 * CS 241 - Spring 2016
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

typedef struct metadata_memhandler 
{
	struct metadata_memhandler *next;
	struct metadata_memhandler *prev;
	size_t size;
	char free;
} metadata;

metadata* start = NULL;
metadata* end = NULL;
metadata* largest_available=NULL;
metadata* another_large = NULL;


void coalesce(metadata * node)
{
	metadata * mynode = node->next;

	metadata * pr = node->prev;
	if (mynode!= NULL)
	{
		if(mynode->free==1)
		{
			node->size += mynode->size + sizeof(metadata);	
			node->next = mynode->next;	
			if (mynode->next != NULL)
				mynode->next->prev = node;
		 	if (mynode==end)
				end=node;
		}
		
	}	
	if (pr!=NULL)
	{
		if(pr->free==1)
		{
			pr->size+=node->size + sizeof(metadata);
			pr->next = node->next;	
			if (node->next!=NULL)
				node->next->prev = pr;
			if (node == end)
				end=pr;
		}
		
	}
	if (largest_available == NULL || largest_available->size < node->size)
	{
		largest_available=node;
		another_large=largest_available;
	}	
}



void splitblock (metadata * chose, int size)
{
	if (chose !=NULL)
	{
		metadata * workingnode  = (metadata*) ((char*)chose+(size+sizeof(metadata))); 
		workingnode->size=chose->size - (size+sizeof(metadata));
		chose->size = size;
		workingnode->next = chose->next;
		workingnode->free=1;		

		if (chose->next != NULL)
			(chose->next)->prev = workingnode;

		workingnode->prev = chose;
		
		chose->next = workingnode;	
		if (chose == end)
			end = workingnode;

		if (largest_available == NULL || (largest_available->size < workingnode->size))
		{
			another_large=largest_available;
			largest_available=workingnode;
		}		
	}
}

/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) 
{
	size_t finalsize = num*size;
	void * newptr = malloc(finalsize);
	if (newptr != NULL)
	{
		memset(newptr, 0, finalsize);
		return newptr;
	}
	return NULL;
}

/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size) 
{
	if (size ==0)
		return NULL;

	metadata * mynode = NULL;
	metadata * currnode = end;
	
	if(largest_available)
	{
		if(size <= largest_available->size)
		{
			while (currnode != NULL) 
			{
				if (currnode->free==1)
				{	
					if(currnode->size>=size)
					{
						mynode = currnode;
						break;
					}
				}
				currnode = currnode->prev;
			}
		}
		
	}

	if (mynode == largest_available)
		largest_available = another_large;

	if (mynode == NULL) 
	{
		void * pteer;
		if ((pteer=sbrk(size+3*sizeof(metadata))) == (void*)-1)	
			return NULL;

		mynode = pteer;
		mynode->size = size+2*sizeof(metadata);
		mynode->prev = end;
		if (end!=NULL)
			end->next = mynode;
		if (start==NULL)
			start = mynode;
		end = mynode;
		end->next=NULL;
	}		
	mynode->free=0;
	if ((mynode ->size > (size+sizeof(metadata)) && (mynode->size -size - sizeof(metadata))>150))
		splitblock(mynode, size);
	return mynode+1;
}


/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) 
{
 	if (!ptr)
    		return;
	metadata* currnode = (metadata*)ptr-1;
	currnode->free = 1;
	coalesce(currnode);
}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) 
{  
 	 if (!ptr)
   		return malloc(size);

  	if (size==0) 
  	{
		free(ptr);
		return NULL;
 	}
	
 	metadata * curr =(metadata*)ptr-1;

	if (curr->size >= size && curr->size <= (size+sizeof(metadata)))
		return curr+1;

	void * newp = malloc(size);
	
	if (!newp)
		return NULL;
	if (curr->size < size)
		memcpy(newp,curr+1,curr->size);
	else
		memcpy(newp,curr+1,size);

	free(curr+1);
	return newp;
}