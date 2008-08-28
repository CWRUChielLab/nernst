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
#include <stdio.h>
#include "options.h"
#include "atom.h"
#include "world.h"
#include "util.h"


struct atom *world;
unsigned long int direction_sz64;
unsigned char *claimed;
unsigned char *direction;

void
initWorld( struct options *o )
{
   int rc = 0;

   // Test that the size of the world is a power of 2.
   if( ( o->x * o->y )  &  ( ( o->x * o->y ) - 1 ) )
   {
      ASSERT( !( ( o->x * o->y )  &  ( ( o->x * o->y ) - 1 ) ) );
   }

   world   = calloc( sizeof( struct atom   ) * o->x * o->y, 1 );
   claimed = calloc( sizeof( unsigned char ) * o->x * o->y, 1 );

   // Lay out the memory for the direction array.
   for( direction_sz64 = get_min_array_size64() * 8; direction_sz64 < (unsigned int)( o->x * o->y ); direction_sz64 *= 2 );

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

