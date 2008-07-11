/* gui.cpp
 *
 * The main application GUI.
 */

/*
#include <QFrame>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
*/
#include <QtGui>

#include "gui.h"
#include "sim.h"
#include "options.h"
#include "ctrl.h"
#include "paint.h"
#include "atom.h"


extern int initialized;
extern int quitting;


NernstGUI::NernstGUI( struct options *o, QWidget *parent, Qt::WindowFlags flags )
   : QMainWindow( parent, flags )
{
   // Initialization controls
   NernstCtrl *ctrl = new NernstCtrl( o );

   QFrame *ctrlFrame = new QFrame();
   ctrlFrame->setFrameStyle( QFrame::Box | QFrame::Sunken );

   QVBoxLayout *ctrlLayout = new QVBoxLayout();
   ctrlLayout->addWidget( ctrl );
   ctrlFrame->setLayout( ctrlLayout );

   // World visualization
   NernstPainter *canvas = new NernstPainter( o );

   QFrame *canvasFrame = new QFrame();
   canvasFrame->setFrameStyle( QFrame::Box | QFrame::Sunken );

   QVBoxLayout *canvasLayout = new QVBoxLayout();
   canvasLayout->addWidget( canvas );
   canvasFrame->setLayout( canvasLayout );

   // Main window
   QGridLayout *mainLayout = new QGridLayout();
   mainLayout->addWidget( ctrlFrame, 0, 0 );
   mainLayout->addWidget( canvasFrame, 0, 1 );
   mainLayout->setColumnStretch( 1, 1 );
   QWidget *mainWidget = new QWidget();
   mainWidget->setLayout( mainLayout );
   setCentralWidget( mainWidget );
   setWindowTitle( "Nernst Potential Simulator" );
   setStatusMsg( "Ready" );

   // Menus
   quitAct = new QAction( "&Quit", this );
   quitAct->setStatusTip( "Quit the simulator" );

   aboutAct = new QAction( "&About", this );
   aboutAct->setStatusTip( "Version information" );

   fileMenu = menuBar()->addMenu( "&File" );
   fileMenu->addAction( quitAct );

   helpMenu = menuBar()->addMenu( "&Help" );
   helpMenu->addAction( aboutAct );

   // Signals
   connect( ctrl, SIGNAL( itersChanged( int ) ), this, SIGNAL( itersChanged( int ) ) );

   connect( ctrl, SIGNAL( poresChanged( int ) ), canvas, SLOT( changePores( int ) ) );
   connect( ctrl, SIGNAL( poresChanged( int ) ), this, SIGNAL( poresChanged( int ) ) );

   connect( ctrl, SIGNAL( lspacingChanged( int ) ), canvas, SLOT( changeLspacing( int ) ) );
   connect( ctrl, SIGNAL( lspacingChanged( int ) ), this, SIGNAL( lspacingChanged( int ) ) );
   
   connect( ctrl, SIGNAL( rspacingChanged( int ) ), canvas, SLOT( changeRspacing( int ) ) );
   connect( ctrl, SIGNAL( rspacingChanged( int ) ), this, SIGNAL( rspacingChanged( int ) ) );

   connect( ctrl, SIGNAL( selectivityChanged( bool ) ), this, SIGNAL( selectivityChanged( bool ) ) );
   connect( ctrl, SIGNAL( electrostaticsChanged( bool ) ), this, SIGNAL( electrostaticsChanged( bool ) ) );

   connect( ctrl, SIGNAL( seedChanged( QString ) ), this, SIGNAL( seedChanged( QString ) ) );

   connect( ctrl, SIGNAL( startBtnClicked() ), canvas, SLOT( startPaint() ) );
   connect( ctrl, SIGNAL( startBtnClicked() ), this, SIGNAL( startBtnClicked() ) );

   connect( ctrl, SIGNAL( pauseBtnClicked() ), this, SIGNAL( pauseBtnClicked() ) );

   connect( ctrl, SIGNAL( continueBtnClicked() ), this, SIGNAL( continueBtnClicked() ) );

   connect( ctrl, SIGNAL( resetBtnClicked() ), canvas, SLOT( resetPaint() ) );
   connect( ctrl, SIGNAL( resetBtnClicked() ), this, SIGNAL( resetBtnClicked() ) );

   connect( ctrl, SIGNAL( quitBtnClicked() ), this, SLOT( close() ) );

   connect( this, SIGNAL( repaintWorld() ), canvas, SLOT( update() ) );
   connect( this, SIGNAL( finished() ), ctrl, SLOT( finish() ) );

   connect( quitAct, SIGNAL( triggered() ), this, SLOT( close() ) );
   connect( aboutAct, SIGNAL( triggered() ), this, SLOT( about() ) );
}


void
NernstGUI::setStatusMsg( QString msg )
{
   statusBar()->showMessage( msg );
}


void
NernstGUI::about()
{
   QMessageBox::about( this, "About | Nernst Potential Simulator",
      "(C) 2008  Barry Rountree, Jeff Gill, Kendrick Shaw, Catherine Kehl,\n"
      "                  Jocelyn Eckert, and Hillel Chiel\n"
      "\n"
      "Version 0.6.9\n"
      "Released under the GPL version 3 or any later version.\n"
      "This is free software; see the source for copying conditions. There is NO\n"
      "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
      "\n"
      "(Note -- SFMT or dSFMT might also be included -- need to work out the\n"
      "appropriate copyright notice for that.)" );
}


void
NernstGUI::closeEvent( QCloseEvent *event )
{
   quitting = 1;
   if( initialized )
   {
      finalizeAtoms();
   }
   QWidget::closeEvent( event );
}

