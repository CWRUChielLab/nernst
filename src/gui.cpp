/* gui.cpp
 *
 * The main application GUI.
 */


#include <QtGui>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>

#include "gui.h"
#include "sim.h"
#include "options.h"
#include "ctrl.h"
#include "paint.h"
#include "atom.h"


extern int LRcharge;
extern int initialized;
extern int quitting;
double t[ 50000 ], v[ 50000 ];


NernstGUI::NernstGUI( struct options *o, QWidget *parent, Qt::WindowFlags flags )
   : QMainWindow( parent, flags )
{
   // Initialization controls
   ctrl = new NernstCtrl( o, this );

   ctrlFrame = new QFrame();
   ctrlFrame->setFrameStyle( QFrame::Box | QFrame::Sunken );

   ctrlLayout = new QVBoxLayout();
   ctrlLayout->addWidget( ctrl );
   ctrlFrame->setLayout( ctrlLayout );

   // World visualization
   canvas = new NernstPainter( o, this );

   canvasFrame = new QFrame();
   canvasFrame->setFrameStyle( QFrame::Box | QFrame::Sunken );

   canvasLayout = new QVBoxLayout();
   canvasLayout->addWidget( canvas );
   canvasFrame->setLayout( canvasLayout );

   // Graphs
   voltsPlot = new QwtPlot();
   voltsPlot->setTitle( "Test Plot" );
   voltsCurve = new QwtPlotCurve( "LRcharge" );

   t[ 0 ] = 0;
   v[ 0 ] = 0;

   voltsCurve->setData( t, v, 0 );
   voltsCurve->attach( voltsPlot );

   // Main window
   mainLayout = new QGridLayout();
   mainLayout->addWidget( ctrlFrame, 0, 0 );
   mainLayout->addWidget( canvasFrame, 0, 1 );
   mainLayout->addWidget( voltsPlot, 0, 2 );
   mainLayout->setColumnStretch( 2, 1 );
   mainWidget = new QWidget();
   mainWidget->setLayout( mainLayout );
   setCentralWidget( mainWidget );
   setWindowTitle( "Nernst Potential Simulator | v 0.7.2" );
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
   connect( ctrl, SIGNAL( updatePreview() ), canvas, SLOT( update() ) );

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
      "Version 0.7.2\n"
      "Released under the GPL version 3 or any later version.\n"
      "This is free software; see the source for copying conditions. There is NO\n"
      "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
      "\n"
      "(Note -- SFMT or dSFMT might also be included -- need to work out the\n"
      "appropriate copyright notice for that.)" );
}


void
NernstGUI::updatePlots( int currentIter )
{
   t[ currentIter ] = currentIter;
   v[ currentIter ] = LRcharge;
   voltsCurve->setData( t, v, currentIter );
   voltsPlot->replot();
}


void
NernstGUI::closeEvent( QCloseEvent *event )
{
   quitting = 1;
   QWidget::closeEvent( event );
}

