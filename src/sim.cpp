/* sim.cpp
 *
 * The engine of the simulation without GUI support.
 *
 * Copyright (c) 2008, Jeffery Gill, Barry Rountree, Kendrick Shaw, 
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
#include <unistd.h>

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
   initConstants();
   maxatomsDefault = o->max_atoms;
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
   takeCensus( 0 );

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
   if( currentIter % 4 == 0 )
   {
      takeCensus( currentIter );
   }

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
   finalizeAtoms();

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

