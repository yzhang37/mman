#include "mem.h"
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>

int page_size = 4096;
int m_error;

typedef struct __header_t {
	int size;
	int magic;
} header_t;

typedef struct __node_t {
	int size;
	struct __node_t *next;
} node_t;

int mem_init(int size_of_region)
{
	page_size = getpagesize();
	node_t *head = mmap(NULL, page_size, PROT_READ | PROT_WRITE, 
		MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (head != MAP_FAILED) 
	{
		head->size = page_size - sizeof(node_t);
		head->next = NULL;
		return 0;
	}
	else
	{
		m_error = errno;
		return (void *)-1;
	}
}
