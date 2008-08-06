/* gui.cpp
 *
 * The main application GUI.
 */


#include <QtGui>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <math.h>

#include "gui.h"
#include "xsim.h"
#include "ctrl.h"
#include "paint.h"
#include "options.h"
#include "atom.h"
#include "world.h"


double x_iters[ 500000 ], y_volts[ 500000 ], y_nernst[ 500000 ];


NernstGUI::NernstGUI( struct options *options, QWidget *parent, Qt::WindowFlags flags )
   : QMainWindow( parent, flags )
{
   o = options;

   // Simulation
   sim = new XNernstSim( o, this );

   // Control panel
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
   setWindowTitle( "Nernst Potential Simulator | v0.7.11" );
   setWindowIcon( QIcon( ":/img/nernst.png" ) );
   setStatusMsg( "Ready" );

   // Actions
   quitAct = new QAction( "&Quit", this );
   quitAct->setStatusTip( "Quit the simulator" );
   connect( quitAct, SIGNAL( triggered() ), this, SLOT( close() ) );

   aboutAct = new QAction( "&About", this );
   aboutAct->setStatusTip( "Version information" );
   connect( aboutAct, SIGNAL( triggered() ), this, SLOT( about() ) );

   // Menus
   fileMenu = menuBar()->addMenu( "&File" );
   fileMenu->addAction( quitAct );

   helpMenu = menuBar()->addMenu( "&Help" );
   helpMenu->addAction( aboutAct );

   // Signals
   connect( sim, SIGNAL( moveCompleted( int ) ), canvas, SLOT( update() ) );
   connect( sim, SIGNAL( moveCompleted( int ) ), this, SLOT( updatePlots( int ) ) );
   connect( sim, SIGNAL( updateStatus( QString ) ), this, SLOT( setStatusMsg( QString ) ) );
   connect( sim, SIGNAL( finished() ), ctrl, SLOT( finish() ) );

   connect( ctrl, SIGNAL( startBtnClicked() ), canvas, SLOT( startPaint() ) );
   connect( ctrl, SIGNAL( startBtnClicked() ), sim, SLOT( runSim() ) );
   connect( ctrl, SIGNAL( pauseBtnClicked() ), sim, SLOT( pauseSim() ) );
   connect( ctrl, SIGNAL( continueBtnClicked() ), sim, SLOT( unpauseSim() ) );
   connect( ctrl, SIGNAL( resetBtnClicked() ), sim, SLOT( resetSim() ) );
   connect( ctrl, SIGNAL( resetBtnClicked() ), canvas, SLOT( resetPaint() ) );
   connect( ctrl, SIGNAL( resetBtnClicked() ), this, SLOT( resetPlots() ) );
   connect( ctrl, SIGNAL( quitBtnClicked() ), this, SLOT( close() ) );
   connect( ctrl, SIGNAL( updatePreview() ), canvas, SLOT( cleanUpdate() ) );
   connect( ctrl, SIGNAL( worldShrunk() ), this, SLOT( shrinkWindow() ) );

   connect( canvas, SIGNAL( previewRedrawn() ), this, SLOT( fixRedraw() ) );
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
      "Version 0.7.11\n"
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
NernstGUI::fixRedraw()
{
   // Fixes a redraw issue whenever the world size is changed in Windows.
   canvasFrame->hide();
   canvasFrame->show();
}


void
NernstGUI::shrinkWindow()
{
   // Shrinks the window to a managable size if it was enlarged too much.
   if( !isMaximized() )
   {
      resize( sizeHint() );
   }
}


void
NernstGUI::closeEvent( QCloseEvent *event )
{
   sim->quitSim();
   QWidget::closeEvent( event );
}

