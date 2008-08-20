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


struct atom *world;
unsigned long int direction_sz64;
unsigned char *claimed;
unsigned char *direction;
double e, k, R, F, t, d, a, eps0, eps, c;


void
initWorld( struct options *o )
{
   int rc = 0;

   // Test that the size of the world is a power of 2.
   if( ( o->x * o->y )  &  ( ( o->x * o->y ) - 1 ) )
   {
      fprintf( stderr, "ERROR: World size must be a power of 2.\n" );
      assert( !( ( o->x * o->y )  &  ( ( o->x * o->y ) - 1 ) ) );
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

   // Constants
   e = 1.60218e-19;     // Elementary charge (C)
   k = 1.38056e-23;     // Boltzmann's constant (J K^-1)
   R = 8.31447;         // Molar gas constant (J K^-1 mol^-1)
   F = 96485.3;         // Faraday's constant (C mol^-1)
   t = 298;             // Temperature (K)

   d = 3.5e-10;         // Length of a lattice square (m)
   a = d * d;           // Membrane area per lattice quare (m^2)
   eps0 = 8.85419e-12;  // Vacuum permittivity (F m^-1)
   //eps = 2.59764;       // Membrane dielectric constant
   //eps = 50;            // Membrane dielectric constant
   eps = 250;            // Membrane dielectric constant
   c = eps * eps0 / d;  // Membrane capacitance (F m^-2)
}

