/* world.c
 *
 *
 * Copyright (c) 2015, Jeffrey Gill, Barry Rountree, Kendrick Shaw, 
 *    Catherine Kehl, Jocelyn Eckert, and Dr. Hillel J. Chiel
 *
 * This file is part of Nernst Potential Simulator.
 * 
 * Nernst Potential Simulator is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * Nernst Potential Simulator is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nernst Potential Simulator.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
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
double e, k, R, F, t, d, a, eps0, eps, c, cBoltz;


void
initConstants()
{
   e = 1.60218e-19;     // Elementary charge (C)
   k = 1.38056e-23;     // Boltzmann's constant (J K^-1)
   R = 8.31447;         // Molar gas constant (J K^-1 mol^-1)
   F = 96485.3;         // Faraday's constant (C mol^-1)
   t = 298;             // Temperature (K)

   d = 3.5e-10;         // Length of a lattice square (m)
   a = d * d;           // Membrane area per lattice quare (m^2)
   eps0 = 8.85419e-12;  // Vacuum permittivity (F m^-1)
   // eps = 2.59764;       // Membrane dielectric constant
   // eps = 50;            // Membrane dielectric constant
   eps = 250;           // Membrane dielectric constant
   c = eps * eps0 / d;  // Membrane capacitance (F m^-2)

   cBoltz = e * e / ( 2 * k * t * c * a ); // Constant used in Boltzmann calculation
}


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

