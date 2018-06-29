#include "mem.h"
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#define MAGIC_T 12345678
#define max(a, b) ((a)>(b)?(a):(b))

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

void listfree()
{
  node_t *p = freelist;
  int flag = 0;
  while (p != NULL)
  {
    if (flag) printf(" -> ");
    flag = 1;
    printf("(%p){size: %d, next: %p}", p, p -> size, p -> next);
    p = p -> next;
  }
  printf("\n");
}

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
		return -1;
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
	int _actsize = size + max(sizeof(header_t), sizeof(node_t));
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
		node_t *prevn = NULL, *nextn = NULL;
		for (; it != NULL; it = it->next)
		{
			if ((void *)it < (void *)hptr)
	  			prevn = (node_t *)it;
			else if ((void *)it >= (void *)hptr + max(sizeof(header_t), sizeof(node_t)) + lsize)
			{
	  			nextn = (node_t *)it;
				break;
			}
		}
	 	node_t *cur = (node_t *)hptr;
	 	int nextn_size = nextn -> size;
	 	void *nextn_next = nextn -> next;
		cur -> next = NULL;
		int prevmerge = 0;
		if (prevn != NULL)
		{
			void *edge = (void *)prevn + sizeof(node_t) + prevn->size;
			if (edge < (void *)hptr)
			{
				prevn -> next = cur;	
	 			cur -> size = lsize;
			}
	 		else if (edge == hptr)
 	 		{
				prevn -> size += max(sizeof(header_t),sizeof(node_t)) +lsize;
				prevmerge = 1;
 	  		}
		}
		else
		{
	 		cur -> size = lsize;
		    freelist = cur;
		}
		if (nextn != NULL)
		{
			void *edge = (void *)cur + max(sizeof(header_t),sizeof(node_t)) + lsize;
			if (edge < (void *)nextn)
			{
				if (!prevmerge)
				{
					cur -> next = nextn;
				}
				else
				{
					prevn -> next = nextn;
				}
			}
			else if (edge == (void *)nextn)
			{
				if (!prevmerge)
				{
					cur -> next = nextn -> next;
					cur -> size += sizeof(node_t) + nextn_size;
				}
				else
				{
					prevn -> next = nextn -> next;
					prevn -> size += sizeof(node_t) + nextn_size;
				}
			}
		}
		return 0;
	}
	return -1;
}

void myWork()
{
  int *a = mem_alloc(sizeof(int) * 3, M_BESTFIT);
  int i;
  for (i = 0; i< 3; ++i)
    scanf("%d", a+i);
  listfree();
  char *b = mem_alloc(sizeof(char) * 200, M_BESTFIT);
  strcpy(b, "Welcome to use Microsoft Windows (Linux kernel)");
  for (i = 0; i<3; ++i)
    printf("%d\n", a[i]);
  printf("%s\n", b);
  listfree();
  double *c = mem_alloc(sizeof(double), M_BESTFIT);
  *c = 0.5/12;
  printf("%f\n", *c);
  listfree();
  mem_free(b);
  puts("After release char *b...");
  listfree();
  short *d = mem_alloc(sizeof(short) * 10, M_BESTFIT);
  puts("After alloc short *d...");
  listfree();
  mem_free(a);
  puts("After release int *a...");
  listfree();
  mem_free(c);
  puts("After release double *c...");
  listfree();
  mem_free(d);
  puts("After release short *d...");
  listfree();
}


int main()
{
	int page = getpagesize();
	mem_init(page);
	//myWork();
	mem_destroy();
}
