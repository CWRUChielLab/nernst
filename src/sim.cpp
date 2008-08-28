/* sim.cpp
 *
 * The engine of the simulation without GUI support.
 *
 * Copyright (c) 2008, Jeffrey Gill, Barry Rountree, Kendrick Shaw, 
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
 */


#include <QApplication>
#include <iostream>
#include <unistd>
#include <cstdlib>
#include <stdio>
#include <string.h>        //memset()
#include <assert.h>
#include <SFMT.h>

#ifdef BLR_USEMAC
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#define _XOPEN_SOURCE 600
#ifdef USING_LOUDER
#define __USE_XOPEN2K      //Needed for posix_memalign on louder -- why?
#endif

#include "sim.h"
#include "options.h"
#include "atom.h"
#include "world.h"
#include "util.h"
#include "safecalls.h"

using namespace SafeCalls;

NernstSim::NernstSim( struct options *options )
{
   o = options;
   maxatomsDefault = o->max_atoms;
   currentIter = 0;
   qtime = safeNew( QTime() );
}


void
NernstSim::initNernstSim()
{
   currentIter = 1;
   elapsed = 0;
   shufflePositions( o );
   initWorld( o );
   initAtoms( o );
   //takeCensus( 0 );

   if( o->progress )
	{
      std::cout << "Iteration: 0 of " << o->iters << " | ";
      std::cout << "0\% complete\r" << std::flush;
   }
}


int 
NernstSim::preIter()
{
   return 0;
}


void 
NernstSim::Iter()
{
   moveAtoms();
}


void 
NernstSim::postIter()
{
   /*
   if( currentIter % 4 == 0 )
   {
      takeCensus( currentIter );
   }
   */

   if( o->progress && currentIter % 256 == 0 )
   {
      std::cout << "                                                                    \r" << std::flush;
      std::cout << "Iteration: " << currentIter << " of " << o->iters << " | ";
      std::cout << (int)( 100 * (double)currentIter / (double)o->iters ) << "\% complete\r" << std::flush;
   }
}


void 
NernstSim::completeNernstSim()
{
   //finalizeAtoms();

   if( o->progress )
   {
      std::cout << "Iteration: " << o->iters << " of " << o->iters << " | ";
      std::cout << "100\% complete" << std::endl;
   }

   if( o->profiling )
   {
      std::cout << "iters = "            << currentIter - 1
                << "  seconds = "        << elapsed
                << "  iters/sec = "      << ( currentIter - 1 ) / elapsed
                << "  ions = "           << o->max_atoms
                << "  area = "           << (long)(o->x) * (long)(o->y)
                << "  density = "        << (double)o->max_atoms / ( (long)(o->x) * (long)(o->y) )
                << "  seed = "           << o->randseed
                << std::endl;
   }
}


void
NernstSim::runSim()
{
   initNernstSim();

   qtime->start();

   for( ; currentIter <= o->iters; currentIter++ )
   {
      preIter();
      Iter();
      postIter();
   }

   elapsed += qtime->elapsed() / 1000.0;

   completeNernstSim();
}


void
NernstSim::initWorld( struct options *o )
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

