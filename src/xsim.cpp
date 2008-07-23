/* xsim.cpp
 *
 * The engine of the simulation with GUI support.
 */


#include <QApplication>
#include <iostream>
#include <unistd.h>

#include "xsim.h"
#include "options.h"
#include "gui.h"
#include "util.h"


int initialized;
int quitting;


XNernstSim::XNernstSim( struct options *options, QWidget *parent )
   : QWidget( parent ), NernstSim( options )
{
   initialized = 0;
   paused      = 0;
   resetting   = 0;
   quitting    = 0;

   gui = new NernstGUI( options, this );
   gui->show();

   // Signals
   connect( gui, SIGNAL( startBtnClicked() ), this, SLOT( runSim() ) );
   connect( gui, SIGNAL( pauseBtnClicked() ), this, SLOT( pauseSim() ) );
   connect( gui, SIGNAL( continueBtnClicked() ), this, SLOT( unpauseSim() ) );
   connect( gui, SIGNAL( resetBtnClicked() ), this, SLOT( resetSim() ) );

   connect( this, SIGNAL( moveCompleted( int ) ), gui, SIGNAL( repaintWorld() ) );
   connect( this, SIGNAL( moveCompleted( int ) ), gui, SLOT( updatePlots( int ) ) );
   connect( this, SIGNAL( updateStatus( QString ) ), gui, SLOT( setStatusMsg( QString ) ) );
   connect( this, SIGNAL( finished() ), gui, SIGNAL( finished() ) );
}


void
XNernstSim::initNernstSim()
{
   paused = 0;
   resetting = 0;
   NernstSim::initNernstSim();
   initialized = 1;
}


int
XNernstSim::preIter()
{
   NernstSim::preIter();
   QCoreApplication::processEvents();

   if( paused )
   {
      elapsed += qtime->elapsed() / 1000.0;
      emit updateStatus( "Iteration: " + QString::number( currentIter ) + " of " + QString::number( o->iters )
            +  " | " + QString::number( (int)( 100 * (double)currentIter / (double)o->iters ) ) + "\% complete" );
      return 1;
   }

   if( resetting || quitting )
   {
      return 1;
   }
   return 0;
}


void
XNernstSim::Iter()
{
   NernstSim::Iter();
}


void
XNernstSim::postIter()
{
   NernstSim::postIter();
   emit moveCompleted( currentIter );

   if( currentIter % 64 == 0 )
   {
      emit updateStatus( "Iteration: " + QString::number( currentIter ) + " of " + QString::number( o->iters ) 
            +  " | " + QString::number( (int)( 100 * (double)currentIter / (double)o->iters ) ) + "\% complete" );
   }

   sleep( o->sleep );
}


void
XNernstSim::completeNernstSim()
{
   NernstSim::completeNernstSim();
   if( !resetting && !quitting )
   {
      emit updateStatus( "Iteration: " + QString::number( currentIter ) + " of " + QString::number( o->iters ) 
            +  " | " + QString::number( (int)( 100 * (double)currentIter / (double)o->iters ) ) + "\% complete" );
      emit finished();
      initialized = 0;
   }
}


void
XNernstSim::runSim()
{
   if( !initialized )
   {
      initNernstSim();
   }
   for( ; currentIter <= o->iters; currentIter++ )
   {
      if( preIter() )
      {
         break;
      }
      Iter();
      postIter();
   }

   if( !paused )
   {
      currentIter--;
      completeNernstSim();
   }
}


void
XNernstSim::pauseSim()
{
   paused = 1;
}


void
XNernstSim::unpauseSim()
{
   paused = 0;
   qtime->restart();
   runSim();
}


void
XNernstSim::resetSim()
{
   resetting = 1;
   if( paused )
   {
      completeNernstSim();
   }
   if( initialized )
   {
      initialized = 0;
   }
   o->max_atoms = maxatomsDefault;
   emit updateStatus( "Ready" );
}

