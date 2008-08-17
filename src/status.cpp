/* status.cpp
 *
 * The GUI's status bar.
 */


#include <QtGui>

#include "status.h"
#include "gui.h"
#include "options.h"
#include "atom.h"
#include "world.h"


#include <iostream>


NernstStatusBar::NernstStatusBar( struct options *options, QWidget *parent )
   : QStatusBar( parent )
{
   o = options;
   mode = 1;

   statusLbl = new QLabel( "Ready" );
   statusLbl->setMinimumWidth( 300 );

   progressBar = new QProgressBar( this );
   progressBar->setRange( 1, o->iters );
   progressBar->setValue( 1 );
   progressBar->setMinimumWidth( 350 );
   progressBar->setMaximumWidth( 350 );

   voltsLbl = new QLabel( "0 mV" );

   addWidget( statusLbl );
   addWidget( progressBar );
   addWidget( new QWidget(), 1 );
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

