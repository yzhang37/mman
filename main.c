#include "mem.h"
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>

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
	int lSize = 0;
	while (head != NULL)
	{
		if (head -> size >= size)
		{
			if (style == M_FIRSTFIT || (style == M_WORSTFIT && head->size > lSize) ||
				(style == M_BESTFIT && head->size < lSize))
			{
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
	
	if (fit_parent != NULL)
	{
		if (fit->size == size)
		{
			fit_parent->next = fit->next;
		}
		else
		{
			//we should seperare the mem_block
			node_t *current = fit + 
		}
	}
	else
	{
		if (fit->size == size)
		{
			freelist = fit->next;
		}
		else
		{

		}
	}
}

int mem_free(void *ptr)
{
	header_t *hptr = (header_t *)((void *)ptr - sizeof(header_t));

}

void myWork()
{
	int *a = mem_alloc(sizeof(int), M_BESTFIT);

}


int main()
{
	int page = getpagesize();
	mem_init(page);
	myWork();
	mem_destroy();
}