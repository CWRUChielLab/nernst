/* status.cpp
 *
 * The GUI's status bar.
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


#include <QtGui>

#include "status.h"
#include "gui.h"
#include "options.h"
#include "safecalls.h"
using namespace SafeCalls;

NernstStatusBar::NernstStatusBar( struct options *options, QWidget *parent )
   : QStatusBar( parent )
{
   o = options;
   mode = 1;

   statusLbl = safeNew( QLabel( "Ready" ) );
   statusLbl->setMinimumWidth( 300 );

   progressBar = safeNew( QProgressBar( this ) );
   progressBar->setRange( 1, o->iters );
   progressBar->setValue( 1 );
   progressBar->setMinimumWidth( 350 );
   progressBar->setMaximumWidth( 350 );

   voltsLbl = safeNew( QLabel( "0 mV" ) );

   addWidget( statusLbl );
   addWidget( progressBar );
   addWidget( safeNew( QWidget() ), 1 );
   addWidget( voltsLbl );

   hideOrShow( "" );

   connect( this, SIGNAL( messageChanged( QString ) ), this, SLOT( hideOrShow( QString ) ) );
}


void
NernstStatusBar::mousePressEvent( QMouseEvent *event )
{
   event->accept();

   if( mode == 1 )
   {
      mode = 2;
   } else {
      mode = 1;
   }

   hideOrShow( "" );
}


void
NernstStatusBar::hideOrShow( QString msg )
{
   if( msg == "" )
   {
   switch( mode )
      {
         case 1:
            statusLbl->show();
            progressBar->hide();
            break;
         case 2:
            statusLbl->hide();
            progressBar->show();
            break;
         default:
            break;
      }
   }
}


void
NernstStatusBar::setStatusLbl( QString msg )
{
   statusLbl->setText( msg );   
}


void
NernstStatusBar::updateProgressBar( int currentIter )
{
   progressBar->setValue( currentIter );
}


void
NernstStatusBar::setVoltsLbl( int currentIter, int avg )
{
   if( avg )
   {
      int i;
      double avgVolt = 0;
      for( i = currentIter - 128; i < currentIter; i++ )
      {
         avgVolt += y_volts[ i ];
      }
      avgVolt /= 128.0;
      voltsLbl->setText( QString::number( avgVolt, 'g', 4 ) + " mV" );
   } else {
      voltsLbl->setText( QString::number( y_volts[ currentIter ], 'g', 4 ) + " mV" );
   }
}


void
NernstStatusBar::recalcProgress()
{
   progressBar->setMaximum( o->iters );
}


void
NernstStatusBar::resetProgress()
{
   progressBar->reset();
}

