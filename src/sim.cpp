/* sim.cpp
 *
 * The engine of the simulation without GUI support.
 */


#include <QApplication>
#include <iostream>
#include <unistd.h>

#include "sim.h"
#include "options.h"
#include "atom.h"
#include "world.h"
#include "util.h"


NernstSim::NernstSim( struct options *options )
{
   initWorld();
   o = options;
   maxatomsDefault = o->max_atoms;
   qtime = new QTime();
}


void
NernstSim::initNernstSim()
{
   currentIter = 1;
   elapsed = 0;
   initAtoms( o );
   takeCensus( 0 );

   if( o->progress )
	{
      std::cout << "Iteration: 0 of " << o->iters << " | ";
      std::cout << "0\% complete\r" << std::flush;
   }
   qtime->start();
}


int 
NernstSim::preIter()
{
   return 0;
}


void 
NernstSim::Iter()
{
   moveAtoms( o );
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
   elapsed += qtime->elapsed() / 1000.0;

   finalizeAtoms();

   if( o->progress )
   {
      std::cout << "Iteration: " << o->iters << " of " << o->iters << " | ";
      std::cout << "100\% complete" << std::endl;
   }

   if( o->profiling )
   {
      std::cout << "iters = "            << o->iters
                << "  seconds = "        << elapsed
                << "  iters/sec = "      << o->iters / elapsed
                << "  pores = "          << o->pores
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
   for( ; currentIter <= o->iters; currentIter++ )
   {
      preIter();
      Iter();
      postIter();
   }
   completeNernstSim();
}

