/* sim.cpp
 *
 * The engine of the simulation.
 */


#include <QApplication>
#include <QVBoxLayout>
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


NernstSim::NernstSim( struct options *options, QWidget *parent )
   : QWidget( parent )
{
   initWorld();
   o = options;
   maxatomsDefault = o->max_atoms;
   paused = 0;
   resetting = 0;
   quitting = 0;

   if( o->use_gui )
   {
      NernstGUI *gui = new NernstGUI( o );

      QVBoxLayout *mainLayout = new QVBoxLayout();
      mainLayout->addWidget( gui );

      setLayout( mainLayout );
      setWindowTitle( "Nernst Potential Simulator | v 0.6.6" );

      // Signals
      connect( gui, SIGNAL( itersChanged( QString ) ), this, SLOT( changeIters( QString ) ) );
      connect( gui, SIGNAL( poresChanged( QString ) ), this, SLOT( changePores( QString ) ) );
      connect( gui, SIGNAL( lspacingChanged( int ) ), this, SLOT( changeLspacing( int ) ) );
      connect( gui, SIGNAL( rspacingChanged( int ) ), this, SLOT( changeRspacing( int ) ) );
      connect( gui, SIGNAL( selectivityChanged( bool ) ), this, SLOT( changeSelectivity( bool ) ) );
      connect( gui, SIGNAL( electrostaticsChanged( bool ) ), this, SLOT( changeElectrostatics( bool ) ) );
      connect( gui, SIGNAL( seedChanged( QString ) ), this, SLOT( changeSeed( QString ) ) );

      connect( gui, SIGNAL( startBtnClicked() ), this, SLOT( startSim() ) );
      connect( gui, SIGNAL( pauseBtnClicked() ), this, SLOT( pauseSim() ) );
      connect( gui, SIGNAL( continueBtnClicked() ), this, SLOT( unpauseSim() ) );
      connect( gui, SIGNAL( resetBtnClicked() ), this, SLOT( resetSim() ) );
      connect( gui, SIGNAL( quitBtnClicked() ), this, SLOT( close() ) );

      connect( this, SIGNAL( moveCompleted() ), gui, SIGNAL( repaint() ) );
      connect( this, SIGNAL( finished() ), gui, SIGNAL( finished() ) );
   } else {
      connect( this, SIGNAL( finished() ), qApp, SLOT( quit() ) );
   }
}


void
NernstSim::changeIters( QString iters )
{
   o->iters = iters.toInt();
}


void
NernstSim::changePores( QString pores )
{
   o->pores = pores.toInt();
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
NernstSim::startSim()
{
   resetting = 0;
   initAtoms( o );
   takeCensus( 0 );

   if( o->progress )
   {
      std::cout << "Iteration: 0 of " << o->iters << " | ";
      std::cout << "0\% complete\r" << std::flush;
   }

#ifdef BLR_USELINUX
   gettimeofday( &tv_start, NULL );
#else
   time( &start );
#endif

   for( currentIter = 1; currentIter <= o->iters; currentIter++ )
   {
      while( o->use_gui && paused )
      {
         QCoreApplication::processEvents();
         if( resetting || quitting )
         {
            paused = 0;
            finalizeAtoms();
            o->max_atoms = maxatomsDefault;
            return;
         }
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

      if( o->use_gui )
      {
         sleep( o->sleep );
      }

      QCoreApplication::processEvents();
      if( o->use_gui && ( resetting || quitting ) )
      {
         paused = 0;
         finalizeAtoms();
         o->max_atoms = maxatomsDefault;
         return;
      }
   }

#ifdef BLR_USELINUX
   gettimeofday( &tv_stop, NULL );
#else
   time( &stop );
#endif

   finalizeAtoms();

   if( o->progress )
   {
      std::cout << "Iteration: " << o->iters << " of " << o->iters << " | ";
      std::cout << "100\% complete" << std::endl;
   }

   if( o->profiling )
   {
#ifdef BLR_USELINUX
      elapsed = ( tv_stop.tv_sec - tv_start.tv_sec ) + ( tv_stop.tv_usec - tv_start.tv_usec ) / 1000000.0;
#else
      elapsed = (double)( stop - start );
#endif
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
   return;
}


void
NernstSim::pauseSim()
{
   paused = 1;
}


void
NernstSim::unpauseSim()
{
   paused = 0;
}


void
NernstSim::resetSim()
{
   resetting = 1;
   o->max_atoms = maxatomsDefault;
}


void
NernstSim::closeEvent( QCloseEvent *event )
{
   quitting = 1;
   QWidget::closeEvent( event );
}

