/* sim.cpp
 *
 * The engine of the simulation.
 */


#include <QApplication>
#include <iostream>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#ifdef BLR_USELINUX
#include <sys/time.h>
#endif

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
      NernstGUI *gui = new NernstGUI( o );
      gui->show();

      // Signals
      connect( gui, SIGNAL( itersChanged( int ) ), this, SLOT( changeIters( int ) ) );
      connect( gui, SIGNAL( poresChanged( int ) ), this, SLOT( changePores( int ) ) );
      connect( gui, SIGNAL( lspacingChanged( int ) ), this, SLOT( changeLspacing( int ) ) );
      connect( gui, SIGNAL( rspacingChanged( int ) ), this, SLOT( changeRspacing( int ) ) );
      connect( gui, SIGNAL( selectivityChanged( bool ) ), this, SLOT( changeSelectivity( bool ) ) );
      connect( gui, SIGNAL( electrostaticsChanged( bool ) ), this, SLOT( changeElectrostatics( bool ) ) );
      connect( gui, SIGNAL( seedChanged( QString ) ), this, SLOT( changeSeed( QString ) ) );

      connect( gui, SIGNAL( startBtnClicked() ), this, SLOT( runSim() ) );
      connect( gui, SIGNAL( pauseBtnClicked() ), this, SLOT( pauseSim() ) );
      connect( gui, SIGNAL( continueBtnClicked() ), this, SLOT( runSim() ) );
      connect( gui, SIGNAL( resetBtnClicked() ), this, SLOT( resetSim() ) );

      connect( this, SIGNAL( moveCompleted() ), gui, SIGNAL( repaintWorld() ) );
      connect( this, SIGNAL( updateStatus( QString ) ), gui, SLOT( setStatusMsg( QString ) ) );
      connect( this, SIGNAL( finished() ), gui, SIGNAL( finished() ) );
   } else {
      connect( this, SIGNAL( finished() ), qApp, SLOT( quit() ) );
   }
}


void
NernstSim::changeIters( int iters )
{
   o->iters = iters;
}


void
NernstSim::changePores( int pores )
{
   o->pores = pores;
}


void
NernstSim::changeLspacing( int lspacing )
{
   o->lspacing = lspacing;
}


void
NernstSim::changeRspacing( int rspacing )
{
   o->rspacing = rspacing;
}


void
NernstSim::changeSelectivity( bool selectivity )
{
   if( selectivity )
   {
      o->selectivity = 1;
   } else {
      o->selectivity = 0;
   }
}


void
NernstSim::changeElectrostatics( bool electrostatics )
{
   if( electrostatics )
   {
      o->electrostatics = 1;
   } else {
      o->electrostatics = 0;
   }
}


void
NernstSim::changeSeed( QString seed )
{
   o->randseed = seed.toInt();
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

#ifdef BLR_USELINUX
   gettimeofday( &tv_start, NULL );
#else
   time( &start );
#endif

   for( ; currentIter <= o->iters; currentIter++ )
   {
      QCoreApplication::processEvents();

      if( o->use_gui && paused )
      {
#ifdef BLR_USELINUX
         gettimeofday( &tv_stop, NULL );
         elapsed += ( tv_stop.tv_sec - tv_start.tv_sec ) + ( tv_stop.tv_usec - tv_start.tv_usec ) / 1000000.0;
#else
         time( &stop );
         elapsed += (double)( stop - start );
#endif
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
         emit moveCompleted();
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

#ifdef BLR_USELINUX
   gettimeofday( &tv_stop, NULL );
   elapsed += ( tv_stop.tv_sec - tv_start.tv_sec ) + ( tv_stop.tv_usec - tv_start.tv_usec ) / 1000000.0;
#else
   time( &stop );
   elapsed += (double)( stop - start );
#endif

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

