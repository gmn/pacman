
#define ZONEID	0xBADA55

//int pagesize = 4096;
//int numpages = 1024;
//int nextAllocNum = 0;

// mb * 1024 * 1024 / pagesize == numpages

//#define NUMPAGES (zonemegs*1024*1024)
//#define ZONEMEGS (numpages*pagesize/(1024*1024))

// page_t pages [512];

#define _CRT_SECURE_NO_DEPRECATE
#include <stdlib.h>
#include "i_system.h"
#include "pacman.h"
#include "virtmem.h"  // definition      


static int vmemused;


memblock_t __memblock_default_s =  __memblock_default_d;
memzone_t __memzone_default_s = __memzone_default_d;

/* -
 *  another thing flashed through my mind when I saw
 *   pages above, carve the memory up into numpages of pagesize
 *
 *   if the request is less than pagesize, check through the
 *    used pages until you find one with the request amount or
 *    less in size and pointer into the page.
 *
 *   when a request exceeds the remainder of all pages, start a 
 *    new page.  
 *
 *   if the request is larger than a page request the nearest
 *    multiple of pagesize greater than the request amount and
 *    return the start of the first page and mark all the pages used.
 *
 *   this wont pack the memory so tightly together which
 *   will leave a fudge-factor area for accidental overruns.  detecting
 *   this and preventing it outright would be better.  consider creating
 *   your pages into a list and then check mem requests against the
 *   bound of the current or other pages to prevent overruns.  then
 *   return the pointer.
 *
 *  This addends Z_Free then.  z_free then has to check the current
 *   page for other allocations , free the one that is asking to be
 *   freed and adjust the pages internal data
 *
 *  if the free request is for multiple pages, then all the pages
 *   must be reset.
 *
 */

memzone_t *mainzone;

/*
====================
 V_virtmem_init
====================
*/

void V_virtmem_init (void)
{
	memblock_t *block;
	int size;

	mainzone = (memzone_t *) I_ZoneBase (&size);
	*mainzone = __memzone_default_s;
	mainzone->size = size;

	// set the entire zone to one free block
	mainzone->blocklist.next =
	mainzone->blocklist.prev =
	block = (memblock_t *) ( (byte *)mainzone + sizeof(memzone_t) );
	*block = __memblock_default_s;

/* mainzone is the start address of all the memory.  so mainzone + memzone_t
 * is (memblock_t *)(mainzone + 34) .  holy fucking magic act.  so casting
 * the address to a memzone pointer now gives the memory the attributes of
 * being dereferenced as a struct of that type!  did not know that.
 *
 * so, block is the first block.  mainzone->blocklist.next/prev both point to
 * it.  got it
 * */ 

	mainzone->blocklist.user = (void **) &mainzone;
	//mainzone->blocklist.user = (void *) mainzone;
	//mainzone->blocklist.tag = 
	mainzone->rover = block;

/* the mainzone->blocklist.user points to the mainzone.  kind of like my
 * owner.  and mainzone->rover starts at the first block.
 */

	block->prev = block->next = &mainzone->blocklist;

/*
 * block->prev/next both point to mainzone->blocklist.  and mainzone->blocklist
 * next/prev both point to block.
 */

	// NULL indicates a free block
	block->user = NULL;

    /* the block is the size of ALL the remaining malloc'd memory */
	block->size = mainzone->size - sizeof(memzone_t);

    vmemused = 0;
}

#if 0
//
// Z_ClearZone
//
void Z_ClearZone (memzone_t *zone)
{
    memblock_t  *block;
    
    // set the entire zone to one free block
    zone->blocklist.next = 
    zone->blocklist.prev =
    block = (memblock_t *) ( (byte *)zone + sizeof(memzone_t) );

    //zone->blocklist.user = (void *) zone;
    zone->blocklist.user = (void **) &zone;
    //zone->blocklist.tag
    zone->rover = block;

//..........
}
#endif

/*
==================== 
 V_free
==================== 
*/
void V_free (void *ptr)
{
    memblock_t  *block;
    memblock_t  *other;
	
    block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t) );

	/* block literally is the block that is one memblock before the ptr */
	/* each chunk of mem has its own header memblock_t that immediately 
     *  preceeds it */

    if (block->id != ZONEID)
		sys_error_fatal ("Z_Free: freed a pointer without ZONEID");

    if (block->user > (void **)0x100)
	{
		// I dont buy it
		*block->user = 0;
	}
    
    // mark as free
    block->user = NULL;
    block->tag = 0;
    block->id = 0;

    vmemused -= block->size;

    other = block->prev;
    if (!other->user)
    {
		// merge with previous free block
		other->size += block->size;
		other->next = block->next;
		other->next->prev = other;

		if (block == mainzone->rover)
			mainzone->rover = other;

		block = other;
	}

	other = block->next;
	if (!other->user)
	{
		// merge the next free block onto the end
		block->size += other->size;
		block->next = other->next;
		block->next->prev = block;

		if (other == mainzone->rover)
			mainzone->rover = block;
	}
}

#define MINFRAGMENT  64	

//
// V_malloc
//
// usually called with (size, PU_STATIC, 0)
void *V_malloc (int size, int tag, void *user)
{
	int 			extra;
	memblock_t		*start;
	memblock_t		*rover;
	memblock_t		*newblock;
	memblock_t		*base;
	static gbool	out_of_memory = gfalse;

	if (out_of_memory)
		sys_error_warn ("out of memory w/ %d remaining\n", mainzone->rover->size);

	/* rounds up to the nearest 4 above */
	size = (size + 3) & ~3;

	// scan through the block list 
	// looking for the first free block
	// of sufficient size,
	// throwing out any purgable blocks along the way,
	

	// add in size of block header
	size += sizeof(memblock_t);
	

	// start looking at rover
	base = mainzone->rover;



	// if there is a free block behind the rover,
	//  back up over them
//	if (base->prev && !base->prev->user)
//		base = base->prev;




	rover = base;
	start = base->prev;


    do
    {
        if (!rover->user)
        {
            if (rover->size >= size)
                break;
        }

        rover = rover->next;
    } 
    while (rover != start);

    if (rover == start)
        sys_error_fatal("V_malloc: ran out of memory sectors");


/*
	do
	{
		if (rover == start)
		{
			sys_error_fatal ("V_malloc: failed on allocation of %i bytes", size);
		}


		rover = rover->next;

				//
		// NO USERS
		//

		if (rover->user)
		{
			if (rover->tag < PU_PURGELEVEL)
			{
				// hit a block that can't be purged,
				//  so move base past it
				base = rover = rover->next;
			}
			else
			{
				// free the rover block (adding the size to base)
			
				// the rover can be the base block
				base = base->prev;
				V_free ((byte *)rover+sizeof(memblock_t));
				base = base->next;
				rover = base->next;
			}
		}				
		else
			rover = rover->next;
			

	} while (base->user || base->size < size);
	*/

	if (size > base->size)
		sys_error_fatal("zone: request %d w/ only %d remaining\n", size, base->size);

	// found a block big enough
	extra = base->size - size;

	if (extra > MINFRAGMENT)
	{
		// there will be a free fragment after the allocated block
		newblock = (memblock_t *) (((byte *)base) + size);
		*newblock = __memblock_default_s;
		/* newblock comes after the one we are mallocing right now */
		newblock->size = extra;

		// NULL indicates free block
		newblock->user = NULL;
		newblock->tag = 0;
		newblock->prev = base;
		newblock->next = base->next;
		newblock->next->prev = newblock;

		base->next = newblock;
		base->size = size;
	} 
	else
	{
		out_of_memory = gtrue;	
	}

	if (user)
	{
		// mark as an in use block
		base->user = &user;

		*(void **)user = (void *) ((byte *)base + sizeof(memblock_t));
	}
	else
	{
		if (tag >= PU_PURGELEVEL)
			sys_error_fatal("V_malloc: an owner is required for purgable blocks");

		// mark as in use, but unowned
		base->user = (void **)2;
	}
	base->tag = tag;

	// next allocation will start looking here
	mainzone->rover = base->next;

	base->id = ZONEID;

    vmemused += base->size;

	/* you get the memory immediately following the memblock_t */
	return (void *) ((byte *)base + sizeof(memblock_t));
}

void V_virtmem_shutdown (void)
{
	I_FreeZoneBase ((byte *)mainzone);
}

int V_getMemUsed ( void )
{
    return vmemused;
}
