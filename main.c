#include "mem.h"
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#define MAGIC_T 12345678

typedef struct __header_t {
	int size;
	int magic;
} header_t;

typedef struct __node_t {
	int size;
	struct __node_t *next;
} node_t;

//the initialize function
int mem_init(int size_of_region);
//dump the memory
void mem_dump();

//
void *mem_alloc(int size, int style);
//free the allocated memory
int mem_free(void *ptr);

int page_size;
void *allocated_addr = 0;
int allocated_memory_size = 0;
int m_error;

node_t *freelist;

int mem_init(int size_of_region)
{
	node_t *head = mmap(NULL, size_of_region, PROT_READ | PROT_WRITE, 
		MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (head != MAP_FAILED) 
	{
		allocated_addr = (void *)head;
		allocated_memory_size = size_of_region;
		head->size = size_of_region - sizeof(node_t);
		head->next = NULL;
		freelist = head;
		return 0;
	}
	else
	{
		m_error = errno;
		return (void *)-1;
	}
}

int mem_destroy()
{
	int rtvalue;
	if (allocated_addr != 0)
	{
		rtvalue = munmap(allocated_addr, allocated_memory_size);
		if (rtvalue != 0)
		{
			m_error = errno;
			return -1;
		}
	}
	return 0;
}

void *mem_alloc(int size, int style)
{
	node_t *head = freelist;
	node_t *parent = NULL;
	node_t *fit = NULL;
	node_t *fit_parent = NULL;
	int _actsize = size + sizeof(header_t);
	int lSize = 0;
	int flag = 0;
	while (head != NULL)
	{
		if (head -> size >= _actsize)
		{
			if (!flag || style == M_FIRSTFIT || (style == M_WORSTFIT && head->size > lSize) ||
				(style == M_BESTFIT && head->size < lSize))
			{
				flag = 1;
				fit = head;
				fit_parent = parent;
				lSize = head->size;
				if (style == M_FIRSTFIT)
					break;
			}
		}
		parent = head;
		head = head -> next;
	}

	if (fit == NULL)
		return NULL;
    /* now we have the pointer to the fit structure */
	if (fit_parent != NULL)
	{
		if (fit->size == _actsize)
		{
			fit_parent->next = fit->next;
		}
		else
		{
			//we should seperare the mem_block
			node_t *current = (node_t *)((void *)fit + _actsize);
			current->next = fit->next;
			fit_parent->next = current;
		}
	}
	else
	{
		if (fit->size == _actsize)
		{
			freelist = fit->next;
		}
		else
		{
			node_t *current = (node_t *)((void *)fit + _actsize);
			current->next = fit->next;
			current->size = freelist -> size - _actsize;
			freelist = current;
		}
	}
    header_t *hptr = (header_t *)fit;
    hptr->size = size;
    hptr->magic = MAGIC_T;
    return (void *)((void *)fit + sizeof(header_t));
}

int mem_free(void *ptr)
{
	header_t *hptr = (header_t *)((void *)ptr - sizeof(header_t));
	if (hptr -> magic == MAGIC_T)
	{
		int lsize = hptr -> size;
		int ishead = 0;
		node_t *it = freelist;
		node_t *prev = NULL, *next = NULL;
		for (; it != NULL; it = it->next)
		{
			if ((void *)it < (void *)hptr)
	  			prev = (node_t *)it;
			else if ((void *)it >= (void *)hptr + sizeof(header_t) + lsize)
			{
	  			next = (node_t *)it;
				break;
			}
		}
	 	node_t *cur = (node_t *)hptr;
		cur -> next = NULL;
		int prevmerge = 0;
		if (prev != NULL)
		{
			void *edge = (void *)prev + sizeof(node_t) + prev->size;
			if (edge < (void *)hptr)
			{
				prev -> next = cur;	
	 			cur -> size = sizeof(header_t) + lsize;
			}
	 		else if (edge == hptr)
 	 		{
				prev -> size += sizeof(header_t) + lsize;
				prevmerge = 1;
 	  		}
		}
		else
		{
	 		cur -> size = sizeof(header_t) + lsize;
		    freelist = cur;
		}
		if (next != NULL)
		{
			void *edge = (void *)cur + sizeof(header_t) + lsize;
			if (edge < (void *)next)
			{
				if (!prevmerge)
				{
					cur -> next = next;
				}
				else
				{
					prev -> next = next;
				}
			}
			else if (edge == next)
			{
				if (!prevmerge)
				{
					cur -> next = next -> next;
					cur -> size += (sizeof(node_t)+next->size);
				}
				else
				{
					prev -> next = next -> next;
					prev -> size += (sizeof(node_t) + next->size);
				}
			}
		}
		return 0;
	}
	return -1;
}

void myWork()
{
	int *a = mem_alloc(sizeof(int), M_BESTFIT);
    *a = 5;
    printf("the value is %d\n", *a);
    mem_free(a);
}


int main()
{
	int page = getpagesize();
	mem_init(page);
	myWork();
	mem_destroy();
}
