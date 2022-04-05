
// Derived from cache.c code, but adapted to take into account our
// inability to physically shift textures about in TMU memory.
// Essentially, instead of being physically shifted, items have to
// be discarded and re-uploaded at the new location.
// Depending on how other APIs work, this code could, possibly,
// be generalised
// A fair bit of simplification happens as we only have one, all-in,
// cache, and because we can't defragment in the same way as a main
// memory cache because stuff can't be moved about - we simply
// throw things out until there's room for a new item.
// ATM the gap made for new items is at the beginning, with the
// access number of items being ignored. This could be improved.
// In an ideal world this code would also handle software texture
// caching.

// WARNING: This code has NOT been tested. Major problems may occur
// when we actually hit the texture memory limit and have to throw
// stuff out.

#define _CRT_SECURE_NO_DEPRECATE 1
#pragma warning(disable : 4996)


#include <stdio.h>
#include <string.h>

#include <glide.h>

#include <xlib.h>

#include "xglide.h"

RCSID( "$Id: gcache.cpp,v 1.1.1.1 2003/08/19 17:44:50 bernie Exp $" )


// Internal routines
static int glide_find_free_lump( char *, int );
static void remove_from_list( gcachelump_t * );
static void throw_out( gcachelump_t * );

// Internal vars
static int access_num;
static int bottom_tmu_address, top_tmu_address;
static gcache_t glide_cache;




//
// Initialises GLIDE cache
//
void glide_cache_init( void ) {

	gcachelump_t *first_lump, *this_lump;
	int size, lp;

	// This will be their first access
	access_num = 0;


	// Get the extents of TMU memory
	bottom_tmu_address = pGrTexMinAddress( GR_TMU0 );
	top_tmu_address = pGrTexMaxAddress( GR_TMU0 );
	size = top_tmu_address - bottom_tmu_address;

	// Initialise cache fields
	glide_cache.free_space = size;
	glide_cache.total_space = size;
	glide_cache.num_lumps = 1;


	// Clear out all cache lumps
	for( lp=0; lp<MAX_GCACHE_LUMPS; lp++ ) {

		this_lump = &glide_cache.gcache_lumps[lp];
		this_lump->assigned = FALSE;
		this_lump->previous = NULL;
		this_lump->next = NULL;
		this_lump->free = TRUE;
		this_lump->key[0] = 0;
		this_lump->size = 0;
		this_lump->access_num = 0;
	}

	// Start it off with a single blank lump
	first_lump = &glide_cache.gcache_lumps[0];
	first_lump->size = size;
	first_lump->free = TRUE;
	first_lump->next = NULL;	// Terminator
	first_lump->previous = NULL;
	first_lump->assigned = TRUE;
	glide_cache.first_lump = first_lump;

	// Say what's happened
	// dprintf("glide_cache_init - grabbed %uKb texture memory\n",size/1024);

	dprintf("init: %uKb glide cache.\n", size/1024 );

	return;
}





//
// Null routine principally for debugging
//
void glide_cache_uninit( void ) {

	// xprintf("glide_cache_uninit - uninited cache\n");

	return;
}





//
// Returns a pointer to a requested item if it's in the cache, or -1
// if it ain't there
//
int glide_cache_item_check( char *key ) {

	gcachelump_t *this_lump;

	// Increase our global access number
	access_num++;

	// See if it's in the cache
	this_lump = glide_cache.first_lump;

	while( this_lump != NULL ) {

		// If this is filled and has the right key, it's the one they asked for
		if( (this_lump->free == FALSE) && !stricmp( this_lump->key, key ) ) {

			this_lump->access_num = access_num;
			
			return this_lump->start + bottom_tmu_address;
		}

		this_lump = this_lump->next;
	}

	return -1;
}




//
// Returns a pointer to free space in the cache for placing a new item
//
int glide_cache_item_space( char *key, int size ) {

	int earliest_start, start, cnt;
	gcachelump_t *this_lump, *earliest_lump;

	// Check they haven't got too many lumps
	if( glide_cache.num_lumps >= MAX_GCACHE_LUMPS ) {
		dprintf("glide_cache_item_space: Too many lumps in cache");
		return -1;
	}

	// Check this item could physically fit
	if( size > glide_cache.total_space ) {
		dprintf("glide_cache_item_space: Asked to cache %d byte item in %d byte cache\n", size, glide_cache.total_space );
		return -1;
	}

	// Since we don't defragment, we have to throw out contiguous
	// items until we make a large enough gap. N.B. This takes no
	// account of how old the item to be discarded is, so it could,
	// in certain instances, be exceedingly inefficient.
	// TODO - Check this doesn't suck too much...

	// Go around growing the first cache lump until it's large
	// enough for the item we wish to store
	for( cnt=0; cnt<MAX_GCACHE_LUMPS; cnt++ ) {

		// If there's already a lump, use it
		if( (start = glide_find_free_lump( key, size )) != (-1) )
			return start + bottom_tmu_address;

		// Throw out the bottom item

		earliest_lump = glide_cache.first_lump;
		earliest_start = (glide_cache.first_lump)->start;

		this_lump = (glide_cache.first_lump)->next;

		while( this_lump != NULL ) {

			// Check if it's the earliest so far

			if( !this_lump->free && (this_lump->start < earliest_start) ) {
				earliest_start = this_lump->start;
				earliest_lump = this_lump;
			}

			// Move on
			this_lump = this_lump->next;
		}

		// Get rid of the first lump in memory
		throw_out( earliest_lump );
	}

	// We shouldn't get here. If something screws up, an infinite
	// loop will happen.
//	dprintf("glide_cache_item_space: fragmentation error.\n");

	return (-1);
}








//
// Throws an item out of the cache, effectively turning
// it into a free lump
//
void throw_out( gcachelump_t *to_go ) {

	gcachelump_t *previous_lump, *this_lump, *next_lump;
	int free_amount = to_go->size;

	// Get those invaluable pointers

	previous_lump = to_go->previous;
	next_lump = to_go->next;

	// If there's a free lump before this one, simply increase its
	// size and remove this lump altogether

	to_go->free = TRUE;

	if( (previous_lump != NULL) && (previous_lump->free == TRUE) ) {

		previous_lump->free += to_go->size;
		remove_from_list( to_go );

		// Get ready for possible linking with the
		// following lump -
		// In best case we amalgamte three free lumps into one

		this_lump = previous_lump;
	}
	else
		// Any forward linking will be from this lump
		this_lump=to_go;


	// If there's a free lump after this one, resize
	// this one and discard the next

	this_lump = to_go;
	if( (next_lump != NULL) && next_lump->free ) {

		this_lump->size += next_lump->size;
		remove_from_list( next_lump );
	}

	// Update the overall free space
	glide_cache.free_space += free_amount;

	return;
}






//
// Looks for a free cache lump and returns a ptr to it, 
// or NULL if failed. Does lump splitting required to 
// leave free space as another item
//
int glide_find_free_lump( char *key, int size ) {

	int lp, new_free_size;
	gcachelump_t *this_lump, *free_lump;

	// See if we can find a large enough contiguous lump

	this_lump = glide_cache.first_lump;

	while( this_lump != NULL ) {

		// If this lump is free, and big enough, use it

		if( (this_lump->free == TRUE) && (this_lump->size >= size) ) {

			// Split this free lump into two if the item doesn't fit it
			// exactly

			new_free_size = this_lump->size - size;

			if( new_free_size ) {

				// Check there's room in the list for another lump

				if( glide_cache.num_lumps >= MAX_GCACHE_LUMPS ) {
					dprintf("glide_cache_item_space: Too many lumps in cache for splitting");
					return (-1);
				}

				// Find an unassigned lump entry
				lp = 0;

				while( (glide_cache.gcache_lumps[lp].assigned == TRUE) && (lp < MAX_GCACHE_LUMPS) )
					lp++;

				free_lump = &glide_cache.gcache_lumps[lp];

				if( free_lump->assigned == TRUE ) {
					dprintf("glide_cache_item_space: Couldn't find unassigned lump slot in cache %d");
					return (-1);
				}

				// Assign the remaining space to the newly found lump
				// entry

				free_lump->assigned = TRUE;
				free_lump->free = TRUE;
				free_lump->start = this_lump->start + size;
				free_lump->size = new_free_size;
				glide_cache.num_lumps++;

				// Update linking information

				free_lump->next = this_lump->next;
				free_lump->previous = this_lump;

				if( this_lump->next != NULL )
					(this_lump->next)->previous = free_lump;

				this_lump->next = free_lump;
			}

			// Set up this item

			this_lump->access_num = access_num;
			this_lump->free = FALSE;
			this_lump->size = size;
			strncpy( this_lump->key, key, PATH_MAX );

			// Remember that space's been taken up

			glide_cache.free_space -= size;

			// Tell them where it starts
			return this_lump->start;
		}

		// Go forward (always a good idea)
		this_lump = this_lump->next;
	}

	// We didn't find an appropriate lump
	return (-1);
}








//
// Removes an item from the doubly linked list
//
void remove_from_list( gcachelump_t *this_lump ) {

	// We have one fewer lumps in the cache now

	glide_cache.num_lumps--;

	// Link past this item

	if( this_lump->previous != NULL )
		(this_lump->previous)->next = this_lump->next;
	else
		glide_cache.first_lump = this_lump->next;

	if( this_lump->next != NULL )
		(this_lump->next)->previous = this_lump->previous;

	// Mark this entry as free in the array of list entries
	this_lump->assigned = FALSE;
	this_lump->free = TRUE;
	this_lump->key[0] = 0;

	return;
}

