/* sim.cpp
 *
 * The engine of the simulation.
 */


#include <QApplication>
#include <iostream>
#include <unistd.h>
#include <time.h>
#include <assert.h>

#include "sim.h"
#include "options.h"
#include "gui.h"
#include "atom.h"
#include "world.h"
#include "util.h"


int initialized;
int quitting;


NernstSim::NernstSim( struct options *options, QWidget *parent )
   : QWidget( parent )
{
   initWorld();
   o = options;
   maxatomsDefault = o->max_atoms;
   initialized = 0;
   quitting = 0;

   if( o->use_gui )
   {
      gui = new NernstGUI( o, this );
      gui->show();

      // Signals
      connect( gui, SIGNAL( startBtnClicked() ), this, SLOT( runSim() ) );
      connect( gui, SIGNAL( pauseBtnClicked() ), this, SLOT( pauseSim() ) );
      connect( gui, SIGNAL( continueBtnClicked() ), this, SLOT( runSim() ) );
      connect( gui, SIGNAL( resetBtnClicked() ), this, SLOT( resetSim() ) );

      connect( this, SIGNAL( moveCompleted( int ) ), gui, SIGNAL( repaintWorld() ) );
      connect( this, SIGNAL( moveCompleted( int ) ), gui, SLOT( updatePlots( int ) ) );
      connect( this, SIGNAL( updateStatus( QString ) ), gui, SLOT( setStatusMsg( QString ) ) );
      connect( this, SIGNAL( finished() ), gui, SIGNAL( finished() ) );
   } else {
      connect( this, SIGNAL( finished() ), qApp, SLOT( quit() ) );
   }
   qtime = new QTime();
}


void
NernstSim::runSim()
{
   if( !initialized )
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
      initialized = 1;
   }

   paused = 0;
   resetting = 0;
   qtime->start();

   for( ; currentIter <= o->iters; currentIter++ )
   {
      QCoreApplication::processEvents();

      if( o->use_gui && paused )
      {
	 elapsed += qtime->elapsed()/1000.0;
         emit updateStatus( "Iteration: " + QString::number( currentIter ) + " of " + QString::number( o->iters )
               +  " | " + QString::number( (int)( 100 * (double)currentIter / (double)o->iters ) ) + "\% complete" );
         return;
      }

      if( o->use_gui && ( resetting || quitting ) )
      {
         return;
      }

      moveAtoms( o );

      if( o->use_gui )
      {
         emit moveCompleted( currentIter );
      }

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

      if( o->use_gui && currentIter % 64 == 0 )
      {
         emit updateStatus( "Iteration: " + QString::number( currentIter ) + " of " + QString::number( o->iters ) 
               +  " | " + QString::number( (int)( 100 * (double)currentIter / (double)o->iters ) ) + "\% complete" );
      }

      if( o->use_gui )
      {
         sleep( o->sleep );
      }
   }

   if( o->use_gui )
   {
      currentIter--;
      emit updateStatus( "Iteration: " + QString::number( currentIter ) + " of " + QString::number( o->iters ) 
            +  " | " + QString::number( (int)( 100 * (double)currentIter / (double)o->iters ) ) + "\% complete" );
   }
   elapsed += qtime->elapsed()/1000.0;

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

   emit finished();
   initialized = 0;
   return;
}


void
NernstSim::pauseSim()
{
   paused = 1;
}


void
NernstSim::resetSim()
{
   resetting = 1;
   if( initialized )
   {
      finalizeAtoms();
      initialized = 0;
   }
   o->max_atoms = maxatomsDefault;
   emit updateStatus( "Ready" );
}

