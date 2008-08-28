/* xsim.cpp
 *
 * The engine of the simulation with GUI support.
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
#include <QTimer>
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
         currentIter = 0;
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

   //sleep( o->sleep );
   if( o->sleep > 0 )
   {
      QTimer *snooze = new QTimer( this );
      snooze->setSingleShot( 1 );
      snooze->start( 100 );
      while( snooze->isActive() )
      {
         QCoreApplication::processEvents();
      }
   }
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

   if( !resetting )
   {
      emit updateVoltsStatus( currentIter - 1, 0 );
   }

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
   emit updateStatus( "Ready" );
   emit updateVoltsStatus( 0, 0 );
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
