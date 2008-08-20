/* xsim.cpp
 *
 * The engine of the simulation with GUI support.
 */


#include <QApplication>
#include <iostream>
#include <unistd.h>

#include "xsim.h"
#include "atom.h"
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


int
XNernstSim::getCurrentIter()
{
   return currentIter;
}


void
XNernstSim::loadWorld( int iter )
{
   initialized = 1;
   currentIter = iter;
}


void
XNernstSim::initNernstSim()
{
   paused = 0;
   resetting = 0;
   NernstSim::initNernstSim();
   emit calcEquilibrium();
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
      emit updateStatus( "Iteration: " + QString::number( currentIter ) + " of " + QString::number( o->iters )
            + " | " + QString::number( (int)( 100 * (double)currentIter / (double)o->iters ) ) + "\% complete" );
      return 1;
   } else {
      if( resetting || quitting )
      {
         return 1;
      } else {
         return 0;
      }
   }
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
            + " | " + QString::number( (int)( 100 * (double)currentIter / (double)o->iters ) ) + "\% complete" );
   }

   if( currentIter % 128 == 0 )
   {
      emit updateVoltsStatus( currentIter, 1 );
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

   qtime->start();

   for( ; currentIter <= o->iters; currentIter++ )
   {
      if( preIter() )
      {
         break;
      }
      Iter();
      postIter();
   }

   elapsed += qtime->elapsed() / 1000.0;

   emit updateVoltsStatus( currentIter - 1, 0 );

   if( currentIter > o->iters )
   {
      emit updateStatus( "Iteration: " + QString::number( currentIter - 1 ) + " of " + QString::number( o->iters )
            + " | " + QString::number( (int)( 100 * (double)( currentIter - 1 ) / (double)o->iters ) ) + "\% complete" );
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
   shufflePositions( o );
   emit updateVoltsStatus( 0, 0 );
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
