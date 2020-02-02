/* Z_zone.c */

#include "doomdef.h"

/*
==============================================================================

						ZONE MEMORY ALLOCATION

There is never any space between memblocks, and there will never be two
contiguous free memblocks.

The rover can be left pointing at a non-empty block

It is of no value to free a cachable block, because it will get overwritten
automatically if needed

==============================================================================
*/

#define DEBUG 0

memzone_t	*mainzone;

/*
========================
=
= Z_Init
=
========================
*/


extern unsigned int _bbsstart;      // bss start free memory ram
extern unsigned long _ramsize;      // megabytes of RAM
extern unsigned long _stacksize;    // kilobytes of stack

unsigned int _bbsstart2 = 0x800A9CA8;//tmp addres
void Z_Init (void)//L80032014()
{
	byte	*mem;
	int		size;

	mem = (byte *)((_bbsstart2 + 3) & ~3);
	size = (((_ramsize - _stacksize) - (((_bbsstart2 + 3) & ~3) & 0x1FFFFFFF)) + 3) & ~3;
	//printf("Mem __bss %X\n", _bbsstart);
	//printf("Mem size %d\n", size);

	/* mars doesn't have a refzone */
	mainzone = Z_InitZone (mem, size);
}

/*
========================
=
= Z_InitZone
=
========================
*/

memzone_t *Z_InitZone(byte *base, int size)//L8003206C()
{
	memzone_t *zone;

	zone = (memzone_t *)base;

	zone->size = size;
	zone->rover = &zone->blocklist;
	zone->blocklist.size = size - (int)((byte *)&zone->blocklist - (byte *)zone);
	zone->blocklist.user = NULL;
	zone->blocklist.tag = 0;
	zone->blocklist.id = ZONEID;
	zone->blocklist.next = NULL;
	zone->blocklist.prev = NULL;
	//zone->blocklist.lockframe = -1;

	return zone;
}

/*
========================
=
= Z_Malloc2
=
= You can pass a NULL user if the tag is < PU_PURGELEVEL
========================
*/

#define MINFRAGMENT	64

void *Z_Malloc2 (memzone_t *mainzone, int size, int tag, void *user)//L800320A0()
{
	int		extra;
	memblock_t	*start, *rover, *newblock, *base;

    #if DEBUG
    Z_CheckHeap (mainzone);	/* DEBUG */
    #endif

	/* */
	/* scan through the block list looking for the first free block */
	/* of sufficient size, throwing out any purgable blocks along the way */
	/* */

	size += sizeof(memblock_t);	/* account for size of block header */
	size = (size+3) & ~3;		/* phrase align everything */

	start = base = mainzone->rover;

	while (base->user || base->size < size)
	{
		if (base->user)
			rover = base;
		else
			rover = base->next;

		if (!rover)
			goto backtostart;

		if (rover->user)
		{
			if (rover->tag < PU_PURGELEVEL)
			{
				/* hit an in use block, so move base past it */
				base = rover->next;
				if (!base)
				{
				backtostart:
					base = &mainzone->blocklist;
				}

				if (base == start)	/* scaned all the way around the list */
                {
                    Z_DumpHeap(mainzone);
					I_Error("Z_Malloc: failed allocation on %i", size);
                }
				continue;
			}

            /* */
            /* free the rover block (adding the size to base) */
            /* */
            Z_Free((byte *)rover + sizeof(memblock_t)); /* mark as free */
		}

		if (base != rover)
		{	/* merge with base */
			base->size += rover->size;
			base->next = rover->next;
			if (rover->next)
				rover->next->prev = base;
		}
	}

	/* */
	/* found a block big enough */
	/* */
	extra = base->size - size;
	if (extra >  MINFRAGMENT)
	{	/* there will be a free fragment after the allocated block */
		newblock = (memblock_t *) ((byte *)base + size );
		newblock->prev = base;
		newblock->next = base->next;
		if (newblock->next)
			newblock->next->prev = newblock;

        base->next = newblock;
		base->size = size;

		newblock->size = extra;
		newblock->user = NULL;		/* free block */
		newblock->tag = 0;
		//printf("size %d\n",size);
	}

	if (user)
	{
		base->user = user;			/* mark as an in use block */
		*(void **)user = (void *)((byte *)base + sizeof(memblock_t));
		//printf("base->user_M %d\n",base->user);
	}
	else
	{
		if (tag >= PU_PURGELEVEL)
			I_Error ("Z_Malloc: an owner is required for purgable blocks");
		base->user = (void *)1;		/* mark as in use, but unowned	 */
	}

	base->tag = tag;
	base->id = ZONEID;

	mainzone->rover = base->next;	/* next allocation will start looking here */
	if (!mainzone->rover)
		mainzone->rover = &mainzone->blocklist;

    #if DEBUG
    Z_CheckHeap (mainzone);	/* DEBUG */
    #endif

	return (void *) ((byte *)base + sizeof(memblock_t));
}


/*
========================
=
= Z_Alloc2
=
= You can pass a NULL user if the tag is < PU_PURGELEVEL
= Exclusive Psx Doom
========================
*/

void *Z_Alloc2(memzone_t *mainzone, int size, int tag, void *user)//L80032298()
{
	int		extra;
	memblock_t	*rover, *newblock, *base, *block;

	/* */
	/* scan through the block list looking for the first free block */
	/* of sufficient size, throwing out any purgable blocks along the way */
	/* */

	#if DEBUG
    Z_CheckHeap (mainzone);	/* DEBUG */
    #endif

    base = &mainzone->blocklist;

    while (base->next)
    {
        base = base->next;
    }
	/*
	for (block = mainzone->blocklist.next; block; block = base->next)
	{
		base = base->next;
	}*/

	size += sizeof(memblock_t);	/* account for size of block header */
	size = (size + 3)&~3;			/* phrase align everything */

	while (base->user || base->size < size)
	{
		if (base->user)
			rover = base;
		else
		{
			/* hit an in use block, so move base past it */
			rover = base->prev;
			if (!rover)
				I_Error("Z_Alloc: failed allocation on %i", size);//FIXME Change Z_Malloc To Z_Alloc
		}

		if (rover->user)
		{
			if (rover->tag < PU_PURGELEVEL)
			{
				/* hit an in use block, so move base past it */
				base = rover->prev;
				if (!base)
					I_Error("Z_Alloc: failed allocation on %i", size);//FIXME Change Z_Malloc To Z_Alloc
				continue;
			}

			/* */
            /* free the rover block (adding the size to base) */
            /* */
            Z_Free((byte *)rover + sizeof(memblock_t)); /* mark as free */
		}

		if (base != rover)
		{	/* merge with base */
			rover->size += base->size;
			rover->next = base->next;

			if (base->next)
				base->next->prev = rover;

            base = rover;
		}
	}

	/* */
	/* found a block big enough */
	/* */
	extra = base->size - size;
	newblock = base;

	if (extra >  MINFRAGMENT)
	{	/* there will be a free fragment after the allocated block */
		base = (memblock_t *)((byte *)base + extra);
		base->size = size;
		base->prev = newblock;
		base->next = newblock->next;

		if (newblock->next)
			newblock->next->prev = base;

		newblock->next = base;
		newblock->size = extra;
		newblock->user = NULL; // free block
		newblock->tag = 0;
	}

	if (user)
	{
		base->user = user;			/* mark as an in use block */
		*(void **)user = (void *)((byte *)base + sizeof(memblock_t));
	}
	else
	{
		if (tag >= PU_PURGELEVEL)
			I_Error("Z_Alloc: an owner is required for purgable blocks");//FIXME Change Z_Malloc To Z_Alloc
		base->user = (void *)1;		/* mark as in use, but unowned	 */
	}

	base->id = ZONEID;
	base->tag = tag;
	mainzone->rover = &mainzone->blocklist;

	#if DEBUG
    Z_CheckHeap (mainzone);	/* DEBUG */
    #endif

	return (void *)((byte *)base + sizeof(memblock_t));
}


/*
========================
=
= Z_Free2
=
========================
*/

inline void Z_Free2(memzone_t *mainzone, void *ptr)//L800324A8()
{
	memblock_t	*block;

	block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
	if (block->id != ZONEID)
		I_Error("Z_Free: freed a pointer without ZONEID");

	if (block->user > (void **)0x100)	/* smaller values are not pointers */
		*block->user = 0;		/* clear the user's mark */
	block->user = NULL;	/* mark as free */
	block->tag = 0;
	block->id = 0;
}

/*
========================
=
= Z_FreeTags
=
========================
*/

void Z_FreeTags (memzone_t *mainzone, int tag)//L80032510()
{
	memblock_t	*block, *next;

	for (block = &mainzone->blocklist ; block ; block = next)
	{
		next = block->next;		/* get link before freeing */
		if (!block->user)
			continue;			/* free block */
		if (block->tag & tag)
		{
		    Z_Free2(mainzone, (byte *)block + sizeof(memblock_t));
		}
	}

	for (block = &mainzone->blocklist; block; block = next)
	{
		next = block->next; // get link before freeing
		if (!block->user && next && !next->user)
		{
			block->size += next->size;
			block->next = next->next;
			if (next->next) // <- Final Doom, Doom GH, Eup Jap
                next->next->prev = block;
            next = block;
		}
	}
    mainzone->rover = &mainzone->blocklist;
}


/*
========================
=
= Z_CheckHeap
=
========================
*/

void Z_CheckHeap (memzone_t *mainzone)//L80032640()
{
	memblock_t *checkblock;

	for (checkblock = &mainzone->blocklist ; checkblock; checkblock = checkblock->next)
	{
		if (!checkblock->next)
		{
			if ((byte *)checkblock + checkblock->size - (byte *)mainzone != mainzone->size)
				I_Error ("Z_CheckHeap: zone size changed\n");
			continue;
		}

		if ( (byte *)checkblock + checkblock->size != (byte *)checkblock->next)
			I_Error ("Z_CheckHeap: block size does not touch the next block\n");
		if ( checkblock->next->prev != checkblock)
			I_Error ("Z_CheckHeap: next block doesn't have proper back link\n");
	}

	#if DEBUG
	Z_DumpHeap(mainzone);
	#endif
}


/*
========================
=
= Z_ChangeTag
=
========================
*/

void Z_ChangeTag (void *ptr, int tag)//L80032708()
{
	memblock_t	*block;

	block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
	if (block->id != ZONEID)
		I_Error ("Z_ChangeTag: freed a pointer without ZONEID");
	if (tag >= PU_PURGELEVEL && (int)block->user < 0x100)
		I_Error ("Z_ChangeTag: an owner is required for purgable blocks");
	block->tag = tag;
}


/*
========================
=
= Z_FreeMemory
=
========================
*/

int Z_FreeMemory (memzone_t *mainzone)//L80032794()
{
	memblock_t	*block;
	int			free;

	free = 0;
	for (block = &mainzone->blocklist ; block ; block = block->next)
    {
		if (!block->user)
			free += block->size;
    }

	return free;
}

/*
========================
=
= Z_DumpHeap
=
========================
*/

void Z_DumpHeap(memzone_t *mainzone)//L800327D4()
{
#if DEBUG
	memblock_t	*block;

	printf("zone size: %i  location: %p\n", mainzone->size, mainzone);

	for (block = &mainzone->blocklist; block; block = block->next)
	{
		printf("block:%p    size:%7i    user:%p    tag:%3i    frame:%i\n",
			block, block->size, block->user, block->tag, block->lockframe);

		if (!block->next)
			continue;

		if ((byte *)block + block->size != (byte *)block->next)
			printf("ERROR: block size does not touch the next block\n");
		if (block->next->prev != block)
			printf("ERROR: next block doesn't have proper back link\n");
	}
#endif
}

