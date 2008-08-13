/* xsim.cpp
 *
 * The engine of the simulation with GUI support.
 */


#include <QApplication>
#include <iostream>
#include <unistd.h>

#include "xsim.h"
#include "options.h"
#include "util.h"


XNernstSim::XNernstSim( struct options *options, QWidget *parent )
   : QWidget( parent ), NernstSim( options )
{
   initialized = 0;
   paused      = 0;
   resetting   = 0;
   quitting    = 0;
}


void
XNernstSim::initNernstSim()
{
   paused = 0;
   resetting = 0;
   NernstSim::initNernstSim();
   emit moveCompleted( 0 );
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

   if( currentIter % 8 == 0 )
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

   if( currentIter > o->iters )
   {
      currentIter = o->iters;
      emit updateStatus( "Iteration: " + QString::number( currentIter ) + " of " + QString::number( o->iters ) 
            +  " | " + QString::number( (int)( 100 * (double)currentIter / (double)o->iters ) ) + "\% complete" );
      emit finished();
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
   if( initialized )
   {
      completeNernstSim();
      initialized = 0;
   }
   o->max_atoms = maxatomsDefault;
   emit updateStatus( "Ready" );
}


void
XNernstSim::quitSim()
{
   quitting = 1;
   if( initialized )
   {
      completeNernstSim();
      initialized = 0;
   }
}
