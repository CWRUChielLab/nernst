/* gui.cpp
 *
 * The main application GUI.
 */


#include <QtGui>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <math.h>

#include "gui.h"
#include "sim.h"
#include "options.h"
#include "ctrl.h"
#include "paint.h"
#include "atom.h"
#include "world.h"


extern int initialized;
extern int quitting;
double x_iters[ 500000 ], y_volts[ 500000 ], y_nernst[ 500000 ];


NernstGUI::NernstGUI( struct options *options, QWidget *parent, Qt::WindowFlags flags )
   : QMainWindow( parent, flags )
{
   o = options;

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

   // Potential plot
   voltsPlot = new QwtPlot();
   voltsPlot->setTitle( "Electric Potential" );
   voltsPlot->setAxisTitle( 0, "Potential (mV)" );
   voltsPlot->setAxisTitle( 2, "Time (iters)" );

   voltsCurve = new QwtPlotCurve( "Membrane Potential" );
   voltsCurve->setData( x_iters, y_volts, 0 );
   voltsCurve->attach( voltsPlot );

   nernstCurve = new QwtPlotCurve( "Nernst Potential" );
   nernstCurve->setPen( QColor( Qt::red ) );
   nernstCurve->setData( x_iters, y_nernst, 0 );
   nernstCurve->attach( voltsPlot );

   // Main window
   mainLayout = new QGridLayout();
   mainLayout->addWidget( ctrlFrame, 0, 0 );
   mainLayout->addWidget( canvasFrame, 0, 1 );
   mainLayout->addWidget( voltsPlot, 0, 2 );
   mainLayout->setColumnMinimumWidth( 2, 350 );
   mainLayout->setColumnStretch( 2, 1 );
   mainWidget = new QWidget();
   mainWidget->setLayout( mainLayout );
   setCentralWidget( mainWidget );
   setWindowTitle( "Nernst Potential Simulator | v0.7.7" );
   setWindowIcon( QIcon( ":/img/darwin.png" ) );
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
   connect( ctrl, SIGNAL( resetBtnClicked() ), this, SLOT( resetPlots() ) );
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
   QMessageBox::about( this, "Nernst Potential Simulator",
      "(C) 2008  Barry Rountree, Jeff Gill, Kendrick Shaw, Catherine Kehl,\n"
      "                  Jocelyn Eckert, and Hillel Chiel\n"
      "\n"
      "Version 0.7.7\n"
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
   // Potential plot
   x_iters[ currentIter ] = currentIter;

   y_volts[ currentIter ] = LRcharge * e / ( c * a * o->y ) * 1000;                                // Membrane potential (mV)
   voltsCurve->setData( x_iters, y_volts, currentIter );

   if( o->pores == 0 )
   {
      y_nernst[ currentIter ] = 0;
   } else {
      y_nernst[ currentIter ] = R * t / F * log( (double)initRHS_K / (double)initLHS_K ) * 1000;   // Equilibrium potential (mV)
   }
   nernstCurve->setData( x_iters, y_nernst, currentIter );

   voltsPlot->replot();
}


void
NernstGUI::resetPlots()
{
   voltsCurve->setData( x_iters, y_volts, 0 );
   nernstCurve->setData( x_iters, y_nernst, 0 );

   voltsPlot->replot();
}


void
NernstGUI::closeEvent( QCloseEvent *event )
{
   quitting = 1;
   QWidget::closeEvent( event );
}

