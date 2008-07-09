/* world.c
 *
 * It's the thing that holds the atoms.
 */


#ifdef BLR_USEMAC
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#include <string.h>        //memset()
#include <assert.h>
#include <SFMT.h>
#include <unistd.h>
#define _XOPEN_SOURCE 600	
#ifdef USING_LOUDER
#define __USE_XOPEN2K      //Needed for posix_memalign on louder -- why?
#endif
#include <stdlib.h>
#include "options.h"
#include "atom.h"
#include "world.h"


struct atom *world;
unsigned long int direction_sz64;
unsigned char *claimed;
unsigned char *direction;


void
initWorld()
{
   int rc = 0;

   // Test that the size of the world is a power of 2.
   assert( !( (WORLD_X*WORLD_Y)  &  ( (WORLD_X*WORLD_Y) - 1 ) ) );

   world   = calloc( sizeof( struct atom   ) * WORLD_X * WORLD_Y, 1 );
   claimed = calloc( sizeof( unsigned char ) * WORLD_X * WORLD_Y, 1 );

   //Lay out the memory for the direction array.
   for( direction_sz64 = get_min_array_size64() * 8; direction_sz64 < WORLD_X * WORLD_Y; direction_sz64 *= 2 );

#ifdef BLR_USELINUX
   rc = posix_memalign( (void**)&direction, getpagesize(), direction_sz64 );
#else
#ifdef BLR_USEMAC
   direction = malloc( direction_sz64 );
#else
#ifdef BLR_USEWIN
   direction = malloc( direction_sz64 + 16 );
   direction += 16 - (long int)direction % 16;
#endif
#endif
#endif

   assert( rc == 0 );
   assert( world && claimed && direction );
}

