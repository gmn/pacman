
#ifndef __Z_ZONE_H__
#define __Z_ZONE_H__    1

typedef struct memblock_s 
{
	int 		size;
	void **		user;
	int			tag;
	int			id;
	struct memblock_s	*next;
	struct memblock_s	*prev;
} memblock_t;

#define __memblock_default_d    { 0, NULL, 0, 0, NULL, NULL }


typedef struct
{
	int			size;

	// start-end cap for doubly linked list
	memblock_t	blocklist;

	memblock_t 	*rover;
} memzone_t;

#define __memzone_default_d { 0, __memblock_default_d, NULL }


typedef enum {
	PU_STATIC 		= 1,
	PU_LEVEL  		= 50,
	PU_PURGELEVEL 	= 100
} purgetag_t;

#endif


void V_virtmem_init (void);
void V_free (void *p);
void * V_malloc (int, int, void *);
void V_virtmem_shutdown (void);
int V_getMemUsed (void);
