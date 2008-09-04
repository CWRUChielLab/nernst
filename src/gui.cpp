/* gui.cpp
 *
 * The main application GUI.
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
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <SFMT.h>
#include <math.h>
#include <limits>
#include <fstream>

#include "gui.h"
#include "status.h"
#include "xsim.h"
#include "ctrl.h"
#include "paint.h"
#include "options.h"
#include "safecalls.h"
using namespace SafeCalls;

double x_iters[ MAX_ITERS ];
double y_volts[ MAX_ITERS ];
double y_ghk[ MAX_ITERS ];
// double y_gibbs[ MAX_ITERS ];
// double y_boltzmann[ MAX_ITERS ];


NernstGUI::NernstGUI( struct options *options, QWidget *parent, Qt::WindowFlags flags )
   : QMainWindow( parent, flags )
{
   o = options;

   // Simulation
   sim = safeNew( XNernstSim( o, this ) );
   s = o->s = sim;	//easy way to pass around access to the simulation.
   

   // Control panel
   ctrl = safeNew( NernstCtrl( o, this ) );

   ctrlFrame = safeNew( QFrame() );
   ctrlFrame->setFrameStyle( QFrame::Box | QFrame::Sunken );

   ctrlLayout = safeNew( QVBoxLayout() );
   ctrlLayout->addWidget( ctrl );
   ctrlFrame->setLayout( ctrlLayout );

   // World visualization
   canvas = safeNew( NernstPainter( o, 0, this ) );

   topCanvasLbl = safeNew( QLabel( "<b>Full World View</b>" ) );
   topCanvasLbl->setAlignment( Qt::AlignCenter );

   canvasScroll = safeNew( QScrollArea() );
   canvasScroll->setWidget( canvas );
   canvasScroll->setAlignment( Qt::AlignCenter );
   canvasScroll->setMinimumWidth( 300 );

   canvasFrame = safeNew( QFrame() );
   canvasFrame->setFrameStyle( QFrame::Box | QFrame::Sunken );
   
   canvasLayout = safeNew( QGridLayout() );
   canvasLayout->addWidget( topCanvasLbl, 0, 0 );
   canvasLayout->addWidget( canvasScroll, 1, 0 );
   canvasFrame->setLayout( canvasLayout );

   zoom = safeNew( NernstPainter( o, 1, this ) );
   topZoomLbl = safeNew( QLabel( "<b>Zoomed View</b>" ) );
   topZoomLbl->setAlignment( Qt::AlignCenter );
   inZoomLbl = safeNew( QLabel( "Intracellular" ) );
   inZoomLbl->setAlignment( Qt::AlignCenter );
   outZoomLbl = safeNew( QLabel( "Extracellular" ) );
   outZoomLbl->setAlignment( Qt::AlignCenter );

   zoomInBtn = safeNew( QPushButton( "+" ) );
   zoomInBtn->setMaximumWidth( 30 );
   zoomOutBtn = safeNew( QPushButton( "-" ) );
   zoomOutBtn->setMaximumWidth( 30 );

   zoomFrame = safeNew( QFrame() );
   zoomFrame->setFrameStyle( QFrame::Box | QFrame::Sunken );

   zoomLayout = safeNew( QGridLayout() );
   zoomLayout->addWidget( topZoomLbl, 0, 0, 1, 4 );
   zoomLayout->addWidget( zoom, 1, 0, 1, 4 );
   zoomLayout->addWidget( inZoomLbl, 2, 0 );
   zoomLayout->addWidget( outZoomLbl, 2, 3 );
   zoomLayout->addWidget( zoomOutBtn, 3, 1 );
   zoomLayout->addWidget( zoomInBtn, 3, 2 );
   zoomLayout->setAlignment( Qt::AlignCenter );
   zoomFrame->setLayout( zoomLayout );

   // Potential plot   
   voltsPlot = safeNew( QwtPlot() );
   voltsPlot->setTitle( "Membrane Potential" );
   voltsPlot->setAxisTitle( 0, "Potential (mV)" );
   voltsPlot->setAxisTitle( 2, "Time (iters)" );

   numCurves = 4;
   curves = safeNew( QwtPlotCurve *[ numCurves ] );
   currentNernstCurve = 3;

   curves[ 0 ] = safeNew( QwtPlotCurve( "Membrane Potential" ) );
   curves[ 0 ]->setData( x_iters, y_volts, 0 );
   curves[ 0 ]->attach( voltsPlot );

   /*
   curves[ 1 ] = safeNew( QwtPlotCurve( "Equilibrium Potential (Gibbs)" ) );
   curves[ 1 ]->setPen( QColor( Qt::green ) );
   curves[ 1 ]->setData( x_iters, y_gibbs, 0 );
   curves[ 1 ]->attach( voltsPlot );
   voltsGibbs = 0;

   curves[ 2 ] = safeNew( QwtPlotCurve( "Equilibrium Potential (Boltzmann)" ) );
   curves[ 2 ]->setPen( QColor( Qt::blue ) );
   curves[ 2 ]->setData( x_iters, y_boltzmann, 0 );
   curves[ 2 ]->attach( voltsPlot );
   voltsBoltzmann = 0;
   */

   curves[ currentNernstCurve ] = safeNew( QwtPlotCurve( "GHK Potential" ) );
   curves[ currentNernstCurve ]->setPen( QColor( Qt::red ) );
   curves[ currentNernstCurve ]->setData( x_iters, y_ghk, 0 );
   curves[ currentNernstCurve ]->attach( voltsPlot );
   voltsGHK = 0;

   // Concentration table
   measuredLbl = safeNew( QLabel( "<b>Measured Concentrations</b>" ) );
   measuredLbl->setAlignment( Qt::AlignHCenter );
   inLbl = safeNew( QLabel( "<b>Intracellular</b>" ) );
   outLbl = safeNew( QLabel( "<b>Extracellular</b>" ) );
   KLbl = safeNew( QLabel( "<font color=#ff2600><b>K<sup>+</sup></b></font>" ) );
   NaLbl = safeNew( QLabel( "<font color=#0000ff><b>Na<sup>+</sup></b></font>" ) );
   ClLbl = safeNew( QLabel( "<font color=#00b259><b>Cl<sup>&ndash;</sup></b></font>" ) );
   //ImpChargeLbl = new QLabel( "<b>Initial<br>Impermeable<br>Charges for<br>Electroneutrality</b>" );
   //ImpPartLbl = new QLabel( "<b>Initial<br>Impermeable<br>Particles for<br>Osmotic Balance</b>" );
   ImpChargeLbl = safeNew( QLabel( "<b>IICE</b>" ) );
   ImpChargeLbl->setToolTip( "Initial impermeable charges for electroneutrality" );
   ImpPartLbl = safeNew( QLabel( "<b>IINPOB</b>" ) );
   ImpPartLbl->setToolTip( "Initial impermable neutral particles for osmotic balance" );

   KInLbl = safeNew( QLabel() );
   KOutLbl = safeNew( QLabel() );
   NaInLbl = safeNew( QLabel() );
   NaOutLbl = safeNew( QLabel() );
   ClInLbl = safeNew( QLabel() );
   ClOutLbl = safeNew( QLabel() );
   ImpChargeInLbl = safeNew( QLabel() );
   ImpChargeOutLbl = safeNew( QLabel() );
   ImpPartInLbl = safeNew( QLabel() );
   ImpPartOutLbl = safeNew( QLabel() );
   adjustTable();

   // Main window layout
   mainLayout = safeNew( QGridLayout() );
   mainLayout->addWidget( ctrlFrame, 0, 0 );

   worldLayout = safeNew( QVBoxLayout() );
   worldLayout->addWidget( canvasFrame );
   worldLayout->addWidget( zoomFrame );
   mainLayout->addLayout( worldLayout, 0, 1 );

   resultsLayout = safeNew( QVBoxLayout() );
   plotFrame = safeNew( QFrame() );
   plotFrame->setFrameStyle( QFrame::Box | QFrame::Sunken );
   plotFrame->setMaximumWidth( 350 );
   plotLayout = safeNew( QVBoxLayout() );
   plotFrame->setLayout( plotLayout );
   plotLayout->addWidget( voltsPlot );
   curveLbl = safeNew( QLabel() );
   plotLayout->addWidget( curveLbl );
   resultsLayout->addWidget( plotFrame );

   concFrame = safeNew( QFrame() );
   concFrame->setFrameStyle( QFrame::Box | QFrame::Sunken );
   concLayout = safeNew( QGridLayout() );
   concFrame->setLayout( concLayout );

   concLayout->addWidget( measuredLbl, 0, 0, 1, 3 );

   concLayout->addWidget( inLbl, 1, 1 );
   concLayout->addWidget( outLbl, 1, 2 );

   concLayout->addWidget( KLbl, 2, 0 );
   concLayout->addWidget( KInLbl, 2, 1 );
   concLayout->addWidget( KOutLbl, 2, 2 );

   concLayout->addWidget( NaLbl, 3, 0 );
   concLayout->addWidget( NaInLbl, 3, 1 );
   concLayout->addWidget( NaOutLbl, 3, 2 );

   concLayout->addWidget( ClLbl, 4, 0 );
   concLayout->addWidget( ClInLbl, 4, 1 );
   concLayout->addWidget( ClOutLbl, 4, 2 );

   concLayout->addWidget( ImpChargeLbl, 5, 0 );
   concLayout->addWidget( ImpChargeInLbl, 5, 1 );
   concLayout->addWidget( ImpChargeOutLbl, 5, 2 );

   concLayout->addWidget( ImpPartLbl, 6, 0 );
   concLayout->addWidget( ImpPartInLbl, 6, 1 );
   concLayout->addWidget( ImpPartOutLbl, 6, 2 );

   resultsLayout->addWidget( concFrame );
   mainLayout->addLayout( resultsLayout, 0, 2 );
   mainLayout->setColumnMinimumWidth( 2, 350 );
   mainLayout->setColumnStretch( 1, 1 );

   mainWidget = safeNew( QWidget() );
   mainWidget->setLayout( mainLayout );
   setCentralWidget( mainWidget );
   setWindowTitle( "Nernst Potential Simulator" );
   setWindowIcon( QIcon( ":/img/nernst.png" ) );
   statusBar = safeNew( NernstStatusBar( o, this ) );
   setStatusBar( statusBar );

   // Actions
   saveInitAct = safeNew( QAction( "&Save Current Conditions", this ) );
   saveInitAct->setStatusTip( "Save the control panel settings for later use" );
   connect( saveInitAct, SIGNAL( triggered() ), this, SLOT( saveInit() ) );

   loadInitAct = safeNew( QAction( "&Load Initial Conditions", this ) );
   loadInitAct->setStatusTip( "Load control panel settings from a file" );
   connect( loadInitAct, SIGNAL( triggered() ), this, SLOT( loadInit() ) );

   saveWorldAct = safeNew( QAction( "Save &World", this ) );
   saveWorldAct->setStatusTip( "Save the state of the world for later simulations" );
   saveWorldAct->setEnabled( 0 );
   connect( saveWorldAct, SIGNAL( triggered() ), this, SLOT( saveWorld() ) );

   loadWorldAct = safeNew( QAction( "Load W&orld", this ) );
   loadWorldAct->setStatusTip( "Load a world state to continue a simulation" );
   connect( loadWorldAct, SIGNAL( triggered() ), this, SLOT( loadWorld() ) );

   zoomInAct = safeNew( QAction( "Zoom &In", this ) );
   zoomInAct->setStatusTip( "Zoom in for a closer look" );
   connect( zoomInAct, SIGNAL( triggered() ), zoom, SLOT( zoomIn() ) );

   zoomOutAct= safeNew( QAction( "Zoom &Out", this ) );
   zoomOutAct->setStatusTip( "Zoom out to see the more of the world" );
   connect( zoomOutAct, SIGNAL( triggered() ), zoom, SLOT( zoomOut() ) );

   fullScreenAct = safeNew( QAction( "&Full Screen", this ) );
   fullScreenAct->setStatusTip( "Toggle between full screen mode and normal mode" );
   fullScreenAct->setCheckable( 1 );
   fullScreenAct->setChecked( 0 );
   connect( fullScreenAct, SIGNAL( triggered( bool ) ), this, SLOT( toggleFullScreen( bool ) ) );

   slowMotionAct = safeNew( QAction( "&Slow Motion", this ) );
   slowMotionAct->setStatusTip( "Toggle between slow motion mode and normal speed" );
   slowMotionAct->setCheckable( 1 );
   slowMotionAct->setChecked( 0 );
   connect( slowMotionAct, SIGNAL( triggered( bool ) ), this, SLOT( toggleSlowMotion( bool ) ) );

   clearTrackedAct = safeNew( QAction( "&Clear Tracked Ions", this ) );
   clearTrackedAct->setStatusTip( "Remove tracking from every ion" );
   connect( clearTrackedAct, SIGNAL( triggered() ), this, SLOT( clearTrackedIons() ) );

   quitAct = safeNew( QAction( "&Quit", this ) );
   quitAct->setStatusTip( "Quit the simulator" );
   connect( quitAct, SIGNAL( triggered() ), this, SLOT( close() ) );

   howDoesItWorkAct = safeNew( QAction( "&How does it work?", this ) );
   howDoesItWorkAct->setStatusTip( "View a short introduction to the simulator" );
   connect( howDoesItWorkAct, SIGNAL( triggered() ), this, SLOT( howDoesItWork() ) );

   howDoIUseAct = safeNew( QAction( "How do I &use the simulator?", this ) );
   howDoIUseAct->setStatusTip( "View a brief tutorial to the simulator" );
   connect( howDoIUseAct, SIGNAL( triggered() ), this, SLOT( howDoIUse() ) );

   aboutAct = safeNew( QAction( "&About", this ) );
   aboutAct->setStatusTip( "View information about the simulator" );
   connect( aboutAct, SIGNAL( triggered() ), this, SLOT( about() ) );

   aboutQtAct = safeNew( QAction( "About &Qt", this ) );
   aboutQtAct->setStatusTip( "View information about Qt" );
   connect( aboutQtAct, SIGNAL( triggered() ), qApp, SLOT( aboutQt() ) );

   // Menus
   fileMenu = menuBar()->addMenu( "&File" );
   fileMenu->addAction( loadInitAct );
   fileMenu->addAction( saveInitAct );
   //fileMenu->addSeparator();
   //fileMenu->addAction( loadWorldAct );
   //fileMenu->addAction( saveWorldAct );
   fileMenu->addSeparator();
   fileMenu->addAction( quitAct );

   viewMenu = menuBar()->addMenu( "&View" );
   viewMenu->addAction( zoomInAct );
   viewMenu->addAction( zoomOutAct );
   viewMenu->addAction( fullScreenAct );
   viewMenu->addSeparator();
   viewMenu->addAction( slowMotionAct );
   viewMenu->addAction( clearTrackedAct );

   helpMenu = menuBar()->addMenu( "&Help" );
   helpMenu->addAction( howDoesItWorkAct );
   helpMenu->addAction( howDoIUseAct );
   helpMenu->addSeparator();
   helpMenu->addAction( aboutAct );
   helpMenu->addAction( aboutQtAct );

   // Signals
   connect( zoomInBtn, SIGNAL( clicked() ), zoom, SLOT( zoomIn() ) );
   connect( zoomOutBtn, SIGNAL( clicked() ), zoom, SLOT( zoomOut() ) );

   connect( this, SIGNAL( resetSim() ), sim, SLOT( resetSim() ) );
   connect( this, SIGNAL( resetSim() ), canvas, SLOT( resetPaint() ) );
   connect( this, SIGNAL( resetSim() ), zoom, SLOT( resetPaint() ) );
   connect( this, SIGNAL( resetSim() ), this, SLOT( disableSaveWorld() ) );
   connect( this, SIGNAL( resetSim() ), this, SLOT( resetPlots() ) );
   connect( this, SIGNAL( resetSim() ), statusBar, SLOT( resetProgress() ) );
   connect( this, SIGNAL( resetSim() ), statusBar, SLOT( recalcProgress() ) );

   connect( this, SIGNAL( settingsLoaded() ), ctrl, SLOT( setNewLoadedSettings() ) );
   connect( this, SIGNAL( settingsLoaded() ), ctrl, SLOT( reloadSettings() ) );
   connect( this, SIGNAL( worldLoaded( int ) ), ctrl, SLOT( reloadSettings() ) );
   connect( this, SIGNAL( worldLoaded( int ) ), ctrl, SLOT( disableCtrl() ) );
   connect( this, SIGNAL( worldLoaded( int ) ), ctrl, SLOT( reenableCtrl() ) );
   connect( this, SIGNAL( worldLoaded( int ) ), canvas, SLOT( startPaint() ) );
   connect( this, SIGNAL( worldLoaded( int ) ), zoom, SLOT( startPaint() ) );
   connect( this, SIGNAL( worldLoaded( int ) ), statusBar, SLOT( recalcProgress() ) );
   connect( this, SIGNAL( worldLoaded( int ) ), sim, SLOT( loadWorld( int ) ) );

   connect( sim, SIGNAL( updateVoltsStatus( int, int ) ), statusBar, SLOT( setVoltsLbl( int, int ) ) );
   connect( sim, SIGNAL( updateStatus( QString ) ), statusBar, SLOT( setStatusLbl( QString ) ) ); 
   connect( sim, SIGNAL( calcEquilibrium() ), this, SLOT( calcEquilibrium() ) );
   connect( sim, SIGNAL( moveCompleted( int ) ), ctrl, SLOT( updateIter( int ) ) );
   connect( sim, SIGNAL( moveCompleted( int ) ), canvas, SLOT( update() ) );
   connect( sim, SIGNAL( moveCompleted( int ) ), zoom, SLOT( update() ) );
   connect( sim, SIGNAL( moveCompleted( int ) ), this, SLOT( updatePlots( int ) ) );
   connect( sim, SIGNAL( moveCompleted( int ) ), this, SLOT( updateTable() ) );
   connect( sim, SIGNAL( moveCompleted( int ) ), statusBar, SLOT( updateProgressBar( int ) ) );
   connect( sim, SIGNAL( finished() ), ctrl, SLOT( reenableCtrl() ) );

   connect( ctrl, SIGNAL( startBtnClicked() ), canvas, SLOT( startPaint() ) );
   connect( ctrl, SIGNAL( startBtnClicked() ), zoom, SLOT( startPaint() ) );
   connect( ctrl, SIGNAL( startBtnClicked() ), statusBar, SLOT( recalcProgress() ) );
   connect( ctrl, SIGNAL( startBtnClicked() ), sim, SLOT( runSim() ) );

   connect( ctrl, SIGNAL( pauseBtnClicked() ), sim, SLOT( pauseSim() ) );
   connect( ctrl, SIGNAL( pauseBtnClicked() ), this, SLOT( enableSaveWorld() ) );

   connect( ctrl, SIGNAL( continueBtnClicked() ), statusBar, SLOT( recalcProgress() ) );
   connect( ctrl, SIGNAL( continueBtnClicked() ), this, SLOT( calcEquilibrium() ) );
   connect( ctrl, SIGNAL( continueBtnClicked() ), this, SLOT( disableSaveWorld() ) );
   connect( ctrl, SIGNAL( continueBtnClicked() ), sim, SLOT( unpauseSim() ) );

   connect( ctrl, SIGNAL( resetCurrentBtnClicked() ), sim, SLOT( resetSim() ) );
   connect( ctrl, SIGNAL( resetCurrentBtnClicked() ), canvas, SLOT( resetPaint() ) );
   connect( ctrl, SIGNAL( resetCurrentBtnClicked() ), zoom, SLOT( resetPaint() ) );
   connect( ctrl, SIGNAL( resetCurrentBtnClicked() ), this, SLOT( disableSaveWorld() ) );
   connect( ctrl, SIGNAL( resetCurrentBtnClicked() ), this, SLOT( resetPlots() ) );
   connect( ctrl, SIGNAL( resetCurrentBtnClicked() ), statusBar, SLOT( resetProgress() ) );
   connect( ctrl, SIGNAL( resetCurrentBtnClicked() ), statusBar, SLOT( recalcProgress() ) );

   connect( ctrl, SIGNAL( resetLoadedBtnClicked() ), sim, SLOT( resetSim() ) );
   connect( ctrl, SIGNAL( resetLoadedBtnClicked() ), canvas, SLOT( resetPaint() ) );
   connect( ctrl, SIGNAL( resetLoadedBtnClicked() ), zoom, SLOT( resetPaint() ) );
   connect( ctrl, SIGNAL( resetLoadedBtnClicked() ), this, SLOT( disableSaveWorld() ) );
   connect( ctrl, SIGNAL( resetLoadedBtnClicked() ), this, SLOT( resetPlots() ) );
   connect( ctrl, SIGNAL( resetLoadedBtnClicked() ), statusBar, SLOT( resetProgress() ) );
   connect( ctrl, SIGNAL( resetLoadedBtnClicked() ), statusBar, SLOT( recalcProgress() ) );

   connect( ctrl, SIGNAL( resetDefaultBtnClicked() ), sim, SLOT( resetSim() ) );
   connect( ctrl, SIGNAL( resetDefaultBtnClicked() ), canvas, SLOT( resetPaint() ) );
   connect( ctrl, SIGNAL( resetDefaultBtnClicked() ), zoom, SLOT( resetPaint() ) );
   connect( ctrl, SIGNAL( resetDefaultBtnClicked() ), this, SLOT( disableSaveWorld() ) );
   connect( ctrl, SIGNAL( resetDefaultBtnClicked() ), this, SLOT( resetPlots() ) );
   connect( ctrl, SIGNAL( resetDefaultBtnClicked() ), statusBar, SLOT( resetProgress() ) );
   connect( ctrl, SIGNAL( resetDefaultBtnClicked() ), statusBar, SLOT( recalcProgress() ) );

   connect( ctrl, SIGNAL( quitBtnClicked() ), this, SLOT( close() ) );
   connect( ctrl, SIGNAL( adjustTable() ), this, SLOT( adjustTable() ) );
   connect( ctrl, SIGNAL( worldSizeChange() ), canvas, SLOT( adjustPaintRegion() ) );
   connect( ctrl, SIGNAL( worldSizeChange() ), zoom, SLOT( adjustPaintRegion() ) );
   connect( ctrl, SIGNAL( updatePreview() ), canvas, SLOT( update() ) );
   connect( ctrl, SIGNAL( updatePreview() ), zoom, SLOT( update() ) );

   connect( canvas, SIGNAL( ionMarked() ), canvas, SLOT( update() ) );
   connect( canvas, SIGNAL( ionMarked() ), zoom, SLOT( update() ) );
   connect( zoom, SIGNAL( ionMarked() ), canvas, SLOT( update() ) );
   connect( zoom, SIGNAL( ionMarked() ), zoom, SLOT( update() ) );
}


void
NernstGUI::about()
{
   QMessageBox::about( this, "About Nernst Potential Simulator",
      "<h3>About Nernst Potential Simulator</h3><br>"
      "<br>"
      "Version 1.0.2<br>"
      "Copyright &copy; 2008  "
      "Jeffrey Gill, Barry Rountree, Kendrick Shaw, "
      "Catherine Kehl, Jocelyn Eckert, "
      "and Dr. Hillel J. Chiel.<br>"
      "<br>"
      "Released under the GPL version 3 or any later version. "
      "This is free software; see the source for copying "
      "conditions. There is NO warranty; not even for "
      "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.<br>"
      "<br>"
      "Have a suggestion? Find a bug? Send us your comments "
      "at <a href='mailto:autopoiesis@case.edu'>"
      "autopoiesis@case.edu</a>." );
}


void
NernstGUI::howDoesItWork()
{
   QString *part1 = new QString(
      "<h3>A Brief Introduction</h3><br>"
      "<br>"
      "The Nernst Potential Simulator simulates the random motion of "
      "dissolved ions in a virtual cell.  Three colors of dots "
      "represent K<sup>+</sup> (red), Na<sup>+</sup> (blue), and "
      "Cl<sup>&ndash;</sup> (green). Other solutes, macromolecules, "
      "organelles, and water are ignored in this simulation.<br>"
      "<br>"
      "A single step through time in the world is called an "
      "\"iteration\". During each iteration, every ion moves to a "
      "randomly selected point adjacent to its current position. "
      "Then, each ion channel, or pore, embedded in the central "
      "membrane has a chance of transporting one ion next to it to "
      "the opposite side. The probability of this transport occurring "
      "is dependent on the current membrane potential and is calculated "
      "using the Boltzmann equation." );
   
   QString *part2 = new QString(
      "<h3>A Brief Introduction</h3><br>"
      "<br>"
      "The membrane potential is calculated by finding the charge "
      "difference between the intracellular space (left compartment) "
      "and the extracelular space (right compartment) divided by the "
      "capacitance of the membrane. When there is initially a "
      "difference in concentrations of an ion between the two "
      "compartments, these ions will tend to flow down their "
      "concentration gradient until the build-up of charges causes "
      "an electrostatic imbalance strong enough to reduce their net "
      "flow across the membrane to zero.  This results in a non-zero "
      "electric potential across the membrane.<br>"
      "<br>"
      "The values labeled IICE and IINPOB are charged particles and "
      "noncharged particles, respectively, that reside in either "
      "compartment (not shown).  The charged particles are placed "
      "so that, initially, each compartment is electrically neutral "
      "regardless of the user's ion concentration settings.  The "
      "noncharged particles are present to ensure osmotic balance "
      "between the compartments (at least initially)." );

   QMessageBox *msg1 = new QMessageBox( QMessageBox::Information, "How does it work?", *part1, NULL, this );
   QMessageBox *msg2 = new QMessageBox( QMessageBox::Information, "How does it work?", *part2, NULL, this );
   QPushButton *moreBtn = msg1->addButton( "More", QMessageBox::ActionRole );
   
   msg1->exec();
   
   if( msg1->clickedButton() == moreBtn )
   {
      msg2->exec();
   }
}


void
NernstGUI::howDoIUse()
{
   QString *part1 = new QString(
      "<h3>A Short Tutorial</h3><br>"
      "<br>"
      "<b>Iterations:</b> Move this slider to increase or decrease "
      "the length of time the simulation will run. Additional iterations "
      "can be added while the simulation is paused or after it has "
      "finished.<br>"
      "<br>"
      "<b>Width and height:</b> These sliders control the size of the "
      "world.<br>"
      "<br>"
      "<b>Dielectric constant:</b> This is a property of the membrane. "
      "Changing its value affects the dynamics of the system.<br>"
      "<br>"
      "<b>Random start:</b> The integer input here, called the seed, "
      "determines the set of random numbers used in the simulation. "
      "Running the simulation twice with the same seed and parameters "
      "will result in identical runs." );

   QString *part2 = new QString(
      "<h3>A Short Tutorial</h3><br>"
      "<br>"
      "<b>Concentration sliders:</b> Move these sliders to change the "
      "initial values of the concentrations of each ion type in each "
      "compartment. You can also enter a value in the text box.  Units "
      "are in millimolar.<br>"
      "<br>"
      "<b>Permeability sliders:</b> The number of pores in the membrane "
      "is proportional to these values. Each ion type has its own "
      "selective ion channel that only allows that type to pass through. "
      "Increasing or decreasing the permeability of an ion will increase "
      "or decrease the number of pores for that ion.<br>"
      "<br>"
      "<b>Selective Permeability:</b> Unchecking this box allows ions of "
      "any type to pass through any channel.<br>"
      "<br>"
      "<b>Electrostatics:</b> Unchecking this box causes the ions to "
      "be treated like neutral particles. The current membrane potential "
      "is ignored for determining the probability of ion transport, and "
      "each ion has a 50/50 chance of moving across the membrane when "
      "it approaches a pore." );

   QString *part3 = new QString(
      "<h3>A Short Tutorial</h3><br>"
      "<br>"
      "<b>Tracking ions:</b> Clicking on ions in either the full world "
      "view or the zoomed view after a simulation has begun will cause "
      "them to become highlighted. This makes following the movements of "
      "an individual ion much easier. Tracking can be removed from all "
      "ions through the \"View\" menu.<br>"
      "<br>"
      "<b>Zoomed view:</b> This window provides a closer look at the "
      "central membrane and ion transport activity. The level of "
      "magnification can be manipulated with the buttons marked "
      "\"+\" and \"-\".<br>"
      "<br>"
      "<b>Membrane potential plot:</b> This plots the potential across "
      "the membrane in millivolts every iteration. A red line indicates "
      "the steady state value predicted by either the Nernst equation or "
      "the Goldman-Hodgkin-Katz voltage equation.<br>"
      "<br>"
      "<b>Measured concentrations table:</b> This table displays the "
      "current concentrations of ions in each compartment." );

   QMessageBox *msg1 = new QMessageBox( QMessageBox::Information, "How do I use the simulator?", *part1, NULL, this );
   QMessageBox *msg2 = new QMessageBox( QMessageBox::Information, "How do I use the simulator?", *part2, NULL, this );
   QMessageBox *msg3 = new QMessageBox( QMessageBox::Information, "How do I use the simulator?", *part3, NULL, this );
   QPushButton *moreBtn1 = msg1->addButton( "More", QMessageBox::ActionRole );
   QPushButton *moreBtn2 = msg2->addButton( "More", QMessageBox::ActionRole );
   
   msg1->exec();
   
   if( msg1->clickedButton() == moreBtn1 )
   {
      msg2->exec();
      if( msg2->clickedButton() == moreBtn2 )
      {
         msg3->exec();
      }
   }
}


void
NernstGUI::saveInit()
{
   QString fileName;
   fileName = QFileDialog::getSaveFileName( this, "Save Current Conditions", "settings.init", "Initial Conditions (*.init)" );

   if( fileName == "" )
   {
      return;
   }

   QFile file( fileName );
   if( !file.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, "Current Conditions", QString("Cannot write file %1:\n%2.")
            .arg( file.fileName() )
            .arg( file.errorString() ) );
      return;
   }

   QTextStream out( &file );
   QApplication::setOverrideCursor( Qt::WaitCursor );
   
   // Begin writing to file
   out << o->iters;           endl( out );
   out << o->x;               endl( out );
   out << o->y;               endl( out );
   out << o->randseed;        endl( out );
   out << o->lK;              endl( out );
   out << o->lNa;             endl( out );
   out << o->lCl;             endl( out );
   out << o->rK;              endl( out );
   out << o->rNa;             endl( out );
   out << o->rCl;             endl( out );
   out << o->pK;              endl( out );
   out << o->pNa;             endl( out );
   out << o->pCl;             endl( out );
   out << o->selectivity;     endl( out );
   out << o->electrostatics;  endl( out );
   out << o->eps;             endl( out );

   QApplication::restoreOverrideCursor();
   return;
}


void
NernstGUI::loadInit()
{
   if( sim->getCurrentIter() == 0 ||
         QMessageBox::warning( this, "Nernst Potential Simulator",
                                     "Loading new conditions will discard your current simulation. Do you want to continue?",
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::Yes )
   {
      emit resetSim();

      QString fileName;
      fileName = QFileDialog::getOpenFileName( this, "Load Initial Conditions", "", "Initial Conditions (*.init)" );

      if( fileName == "" )
      {
         return;
      }

      QFile file( fileName );
      if( !file.open( QIODevice::ReadOnly ) )
      {
         QMessageBox::warning( this, "Initial Conditions", QString("Cannot read file %1:\n%2.")
               .arg( file.fileName() )
               .arg( file.errorString() ) );
         return;
      }

      QTextStream in( &file );
      QApplication::setOverrideCursor( Qt::WaitCursor );

      // Begin reading file
      in >> o->iters;
      in >> o->x;
      in >> o->y;
      in >> o->randseed;
      in >> o->lK;
      in >> o->lNa;
      in >> o->lCl;
      in >> o->rK;
      in >> o->rNa;
      in >> o->rCl;
      in >> o->pK;
      in >> o->pNa;
      in >> o->pCl;
      in >> o->selectivity;
      in >> o->electrostatics;
      in >> o->eps;

      QApplication::restoreOverrideCursor();
      emit settingsLoaded();
      return;
   }
}


void
NernstGUI::saveWorld()
{
   // Needs work.
   /*
   QString fileName;
   fileName = QFileDialog::getSaveFileName( this, "Save World", "nernst.world", "World State (*.world)" );

   if( fileName == "" )
   {
      return;
   }

   QFile file( fileName );
   if( !file.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, "World State", QString("Cannot write file %1:\n%2.")
            .arg( file.fileName() )
            .arg( file.errorString() ) );
      return;
   }

   QTextStream out( &file );
   QApplication::setOverrideCursor( Qt::WaitCursor );

   std::cout << "get_sfmt_idx() = " << get_sfmt_idx() << "\n";

   // Begin writing to file
   out << sim->getCurrentIter(); endl( out );
   out << o->iters;              endl( out );
   out << o->x;                  endl( out );
   out << o->y;                  endl( out );
   out << o->pores;              endl( out );
   out << o->randseed;           endl( out );
   out << o->lconc;              endl( out );
   out << o->rconc;              endl( out );
   out << o->selectivity;        endl( out );
   out << o->electrostatics;     endl( out );

   for( int x = 0; x < o->x; x++ )
   {
      for( int y = 0; y < o->y; y++ )
      {
         out << world[ idx( x, y ) ].delta_x << " ";
         out << world[ idx( x, y ) ].delta_y << " ";
         out << world[ idx( x, y ) ].color << " ";
      }
   }
   endl( out );

   out << get_sfmt_idx();
   endl( out );

   for( int i = 0; i < sizeofSFMT(); i++ )
   {
      out << *(get_sfmt_state32() + i ) << " ";
   }
   endl( out );

   QApplication::restoreOverrideCursor();
   return;
   */
}


void
NernstGUI::loadWorld()
{
   // Not working yet
   /*
   QString fileName;
   fileName = QFileDialog::getOpenFileName( this, "Load World", "", "World State (*.world)" );

   if( fileName == "" )
   {
      return;
   }

   QFile file( fileName );
   if( !file.open( QIODevice::ReadOnly ) )
   {
      QMessageBox::warning( this, "World State", QString("Cannot read file %1:\n%2.")
            .arg( file.fileName() )
            .arg( file.errorString() ) );
      return;
   }

   QTextStream in( &file );
   QApplication::setOverrideCursor( Qt::WaitCursor );

   // Begin reading file
   int currentIter, sfmt_idx;

   in >> currentIter;
   in >> o->iters;
   in >> o->x;
   in >> o->y;
   in >> o->pores;
   in >> o->randseed;
   in >> o->lconc;
   in >> o->rconc;
   in >> o->selectivity;
   in >> o->electrostatics;

   shufflePositions( o );
   initWorld( o );
   initAtoms( o );

   for( int x = 0; x < o->x; x++ )
   {
      for( int y = 0; y < o->y; y++ )
      {
         int delta_x, delta_y, color;

         in >> delta_x;
         in >> delta_y;
         in >> color;

         world[ idx( x, y ) ].delta_x = delta_x;
         world[ idx( x, y ) ].delta_y = delta_y;
         world[ idx( x, y ) ].color   = color;
      }
   }

   in >> sfmt_idx;
   set_sfmt_idx( sfmt_idx );
   std::cout << "get_sfmt_idx() = " << get_sfmt_idx() << "\n";

   for( int i = 0; i < sizeofSFMT(); i++ )
   {
      in >> *(get_sfmt_state32() + i );
   }

   QApplication::restoreOverrideCursor();
   emit worldLoaded( currentIter );
   return;
   */
}


void
NernstGUI::toggleFullScreen( bool checked )
{
   if( checked )
   {
      showFullScreen();
   } else {
      showNormal();
   }
}


void
NernstGUI::toggleSlowMotion( bool checked )
{
   if( checked )
   {
      o->sleep = 1;
   } else {
      o->sleep = 0;
   }
}


void
NernstGUI::enableSaveInit()
{
   saveInitAct->setEnabled( 1 );
}


void
NernstGUI::disableSaveInit()
{
   saveInitAct->setEnabled( 0 );
}


void
NernstGUI::enableLoadInit()
{
   loadInitAct->setEnabled( 1 );
}


void
NernstGUI::disableLoadInit()
{
   loadInitAct->setEnabled( 0 );
}


void
NernstGUI::enableSaveWorld()
{
   saveWorldAct->setEnabled( 1 );
}


void
NernstGUI::disableSaveWorld()
{
   saveWorldAct->setEnabled( 0 );
}


void
NernstGUI::enableLoadWorld()
{
   loadWorldAct->setEnabled( 1 );
}


void
NernstGUI::disableLoadWorld()
{
   loadWorldAct->setEnabled( 0 );
}


void
NernstGUI::clearTrackedIons()
{
   if( s->world != NULL )
   {
      for( int x = 0; x < o->x; x++ )
      {
         for( int y = 0; y < o->y; y++ )
         {
            switch( s->world[ s->idx( x, y ) ].color )
            {
               case ATOM_K_TRACK:
                  s->world[ s->idx( x, y ) ].color = ATOM_K;
                  break;
               case ATOM_Na_TRACK:
                  s->world[ s->idx( x, y ) ].color = ATOM_Na;
                  break;
               case ATOM_Cl_TRACK:
                  s->world[ s->idx( x, y ) ].color = ATOM_Cl;
                  break;
               default:
                  break;
            }
         }
      }
      canvas->update();
      zoom->update();
   }
}


double
NernstGUI::gibbsEnergy( int q )
{
   // This does not make accurate predictions.
   // Perhaps the equation is wrong.

   /*
   double energy;
   energy = ( e * ionCharge( ATOM_K ) * e * ( LRcharge - 2 * q ) / ( c * a * o->y ) )        // qQ/C = qV (C^2 F^-1 = J)
               + ( k * t * log( (double)( initRHS_K + q ) / (double)( initLHS_K - q ) ) );   //           (J K^-1 K = J)
   return energy;
   */

   return 0 * q;
}


double
NernstGUI::boltzmannProbDiff( int q )
{
   // This does make accurate predictions for KCl.
   // However, we aren't using it right now.

   /*
   double probLeft;
   double probRight;
   double diff;
   const double constant = e * e / ( 2 * k * t * c * a );

   probLeft  = 16.0 / 256.0 * exp( constant * ( LRcharge - 2 * q ) * -ionCharge( ATOM_K ) / o->y );
   probRight = 16.0 / 256.0 * exp( constant * ( LRcharge - 2 * q ) *  ionCharge( ATOM_K ) / o->y );

   diff = ( probRight * ( initLHS_K - q ) ) - ( probLeft * ( initRHS_K + q ) );

   return diff;
   */

   return 0 * q;
}


void
NernstGUI::calcEquilibrium()
{
   // Equilibrium predicted by the Nernst equation
   //voltsNernst = R * t / F * log( (double)initRHS_K / (double)initLHS_K ) * 1000;

   // Equilibrium predicted by the Goldman-Hodgkin-Katz voltage equation
   voltsGHK = o->R * o->t / o->F * log( ( ( o->pK * s->initRHS_K ) + ( o->pNa * s->initRHS_Na ) + ( o->pCl * s->initLHS_Cl ) ) /
                               ( ( o->pK * s->initLHS_K ) + ( o->pNa * s->initLHS_Na ) + ( o->pCl * s->initRHS_Cl ) ) ) * 1000;

   /*
   int q;

   // Equilibrium predicted by Gibbs energy minimumization
   q = 1;
   double currentEnergy       = gibbsEnergy( 0 ); 
   double testEnergy          = gibbsEnergy( q ); 
   double nextTestEnergy;

   if( abs( currentEnergy ) > abs( testEnergy ) )              // If this first test moved the value
   {                                                           // closer to zero,
      if( currentEnergy * testEnergy > 0 )                     // and if this first test didn't already
      {                                                        // make the value cross zero,
         nextTestEnergy       = gibbsEnergy( q + 1 ); 
         while( testEnergy * nextTestEnergy > 0 )              // Keep incrementing q until the
         {                                                     // value crosses zero.
            q++;
            testEnergy        = nextTestEnergy; 
            nextTestEnergy    = gibbsEnergy( q + 1 );
         }
      }
   } else {                                                    // Else, try the test in the other
      q = -1;                                                  // direction.
      testEnergy              = gibbsEnergy( q );
      if( abs( currentEnergy ) > abs( testEnergy ) )           // If this first test moved the value
      {                                                        // closer to zero,
         if( currentEnergy * testEnergy > 0 )                  // and if this first test didn't already
         {                                                     // make the value cross zero,
            nextTestEnergy    = gibbsEnergy( q - 1 ); 
            while( testEnergy * nextTestEnergy > 0 )           // Keep decrementing q until the
            {                                                  // value crosses zero.
               q--;
               testEnergy     = nextTestEnergy; 
               nextTestEnergy = gibbsEnergy( q - 1 );
            }
         }
      }
   }

   voltsGibbs = ( LRcharge - 2 * q ) * e / ( c * a * o->y ) * 1000;

   // Equilibrium predicted when probability of crossing the membrane is closest to 50/50
   q = 1;
   double currentProbDiff       = boltzmannProbDiff( 0 );
   double testProbDiff          = boltzmannProbDiff( q );
   double nextTestProbDiff;

   if( abs( currentProbDiff ) > abs( testProbDiff ) )          // If this first test moved the value
   {                                                           // closer to zero,
      if( currentProbDiff * testProbDiff > 0 )                 // and if this first test didn't already
      {                                                        // make the value cross zero,
         nextTestProbDiff       = boltzmannProbDiff( q + 1 );
         while( testProbDiff * nextTestProbDiff > 0 )          // Keep incrementing q until the
         {                                                     // value crosses zero.
            q++;
            testProbDiff        = nextTestProbDiff;
            nextTestProbDiff    = boltzmannProbDiff( q + 1 );
         }
      }
   } else {                                                    // Else, try the test in the other
      q = -1;                                                  // direction.
      testProbDiff              = boltzmannProbDiff( q );
      if( abs( currentProbDiff ) > abs( testProbDiff ) )       // If this first test moved the value
      {                                                        // closer to zero,
         if( currentProbDiff * testProbDiff > 0 )              // and if this first test didn't already
         {                                                     // make the value cross zero,
            nextTestProbDiff    = boltzmannProbDiff( q - 1 );
            while( testProbDiff * nextTestProbDiff > 0 )       // Keep decrementing q until the
            {                                                  // value crosses zero.
               q--;
               testProbDiff     = nextTestProbDiff;
               nextTestProbDiff = boltzmannProbDiff( q - 1 );
            }
         }
      }
   }

   voltsBoltzmann = ( LRcharge - 2 * q ) * e / ( c * a * o->y ) * 1000;
   */
}


void
NernstGUI::updatePlots( int currentIter )
{
   static int nernstHasSomeData = 0;
   static int beganThisNernstCurve = 0;

   if( currentIter < 0 )
   {
      nernstHasSomeData = 0;
      beganThisNernstCurve = 0;
      return;
   }

   x_iters[ currentIter ] = currentIter;
   y_volts[ currentIter ] = s->LRcharge * o->e / ( o->c * o->a * o->y ) * 1000;  // Current membrane potential (mV)
   // y_gibbs[ currentIter ] = voltsGibbs;
   // y_boltzmann[ currentIter ] = voltsBoltzmann;

   curves[ 0 ]->setData( x_iters, y_volts, currentIter );
   // curves[ 1 ]->setData( x_iters, y_gibbs, currentIter );
   // curves[ 2 ]->setData( x_iters, y_boltzmann, currentIter );

   int drawThisTime = 1;
   if( o->electrostatics != 1                               ||
       o->selectivity != 1                                  ||
       voltsGHK == std::numeric_limits<double>::infinity()  || // inf
       voltsGHK == -std::numeric_limits<double>::infinity() || // -inf
       voltsGHK != voltsGHK                                 )  // NaN
   {
      drawThisTime = 0;
   }

   if( drawThisTime )
   {
      if( !nernstHasSomeData )
      {
         beganThisNernstCurve = currentIter;
         nernstHasSomeData = 1;
      }

      y_ghk[ currentIter ] = voltsGHK;
      curves[ currentNernstCurve ]->setData( x_iters + beganThisNernstCurve,
                                             y_ghk + beganThisNernstCurve,
                                             currentIter - beganThisNernstCurve + 1 );
      if( o->pK >  0 && o->pNa == 0 && o->pCl == 0 ||
          o->pK == 0 && o->pNa >  0 && o->pCl == 0 ||
          o->pK == 0 && o->pNa == 0 && o->pCl >  0 )
      {
         curveLbl->setText( "<font color=#ff0000>Nernst: " + QString::number( voltsGHK ) + " mV</font>" );
      } else {
         curveLbl->setText( "<font color=#ff0000>Goldman-Hodgkin-Katz: " + QString::number( voltsGHK ) + " mV</font>" );
      }
   } else {
      if( nernstHasSomeData )
      {
         currentNernstCurve++;
         if( currentNernstCurve >= numCurves )
         {
            // Grow the curves array.
            QwtPlotCurve *temp[ numCurves ];
            for( int i = 0; i < numCurves; i++ )
            {
               temp[ i ] = curves[ i ];
            }
            numCurves *= 2;
            curves = safeNew( QwtPlotCurve *[ numCurves ] );
            for( int i = 0; i < numCurves / 2; i++ )
            {
               curves[ i ] = temp[ i ];
            }
         }
         curves[ currentNernstCurve ] = safeNew( QwtPlotCurve( "GHK Potential" ) );
         curves[ currentNernstCurve ]->setPen( QColor( Qt::red ) );
         curves[ currentNernstCurve ]->attach( voltsPlot );
         nernstHasSomeData = 0;
      }
      curveLbl->setText( "<font color=#ff0000>Goldman-Hodgkin-Katz: N/A</font>" );
   }
   voltsPlot->replot();
}


void
NernstGUI::resetPlots()
{
   curves[ 0 ]->setData( x_iters, y_volts, 0 );
   // curves[ 1 ]->setData( x_iters, y_gibbs, 0 );
   // curves[ 2 ]->setData( x_iters, y_boltzmann, 0 );

   for( int i = 3; i <= currentNernstCurve; i++ )
   {
      curves[ i ]->setData( x_iters, y_ghk, 0 );
   }

   currentNernstCurve = 3;
   updatePlots( -1 );
   voltsPlot->replot();

   curveLbl->setText( "" );
}


void
NernstGUI::adjustTable()
{
   // Adjust the concentration value in the table to agree with changing settings.
   KInLbl->setText( QString::number( o->lK ) + " mM" );
   KOutLbl->setText( QString::number( o->rK ) + " mM" );
   NaInLbl->setText( QString::number( o->lNa ) + " mM" );
   NaOutLbl->setText( QString::number( o->rNa ) + " mM" );
   ClInLbl->setText( QString::number( o->lCl ) + " mM" );
   ClOutLbl->setText( QString::number( o->rCl ) + " mM" );

   int chargeLeft  = o->lK + o->lNa - o->lCl;
   int chargeRight = o->rK + o->rNa - o->rCl;

   if( chargeLeft > 0 )
   {
      ImpChargeInLbl->setText( "(<html>&ndash;</html>) " + QString::number( abs( chargeLeft ) ) + " mM" );
   } else {
      ImpChargeInLbl->setText( "(+) " + QString::number( abs( chargeLeft ) ) + " mM" );
   }

   if( chargeRight > 0 )
   {
      ImpChargeOutLbl->setText( "(<html>&ndash;</html>) " + QString::number( abs( chargeRight ) ) + " mM" );
   } else {
      ImpChargeOutLbl->setText( "(+) " + QString::number( abs( chargeRight ) ) + " mM" );
   }

   int partLeft  = o->lK + o->lNa + o->lCl + abs( chargeLeft );
   int partRight = o->rK + o->rNa + o->rCl + abs( chargeRight );

   if( partLeft > partRight )
   {
      ImpPartInLbl->setText( "0 mM" );
      ImpPartOutLbl->setText( QString::number( partLeft - partRight ) + " mM " );
   } else {
      ImpPartOutLbl->setText( "0 mM" );
      ImpPartInLbl->setText( QString::number( partRight - partLeft ) + " mM " );
   }
}

void
NernstGUI::updateTable()
{
   // Fill the concentration table with the current concentrations.
   int numK, numNa, numCl;

   numK  = (int)( (double)(s->initLHS_K)  / ( (double)( o->x / 2 - 1 ) * (double)( o->y ) / 3.0 ) * (double)MAX_CONC + 0.5 );
   numNa = (int)( (double)(s->initLHS_Na) / ( (double)( o->x / 2 - 1 ) * (double)( o->y ) / 3.0 ) * (double)MAX_CONC + 0.5 );
   numCl = (int)( (double)(s->initLHS_Cl) / ( (double)( o->x / 2 - 1 ) * (double)( o->y ) / 3.0 ) * (double)MAX_CONC + 0.5 );

   KInLbl->setText( QString::number( numK ) + " mM" );
   NaInLbl->setText( QString::number( numNa ) + " mM" );
   ClInLbl->setText( QString::number( numCl ) + " mM" );

   numK  = (int)( (double)(s->initRHS_K)  / ( (double)( o->x / 2 - 2 ) * (double)( o->y ) / 3.0 ) * (double)MAX_CONC + 0.5 );
   numNa = (int)( (double)(s->initRHS_Na) / ( (double)( o->x / 2 - 2 ) * (double)( o->y ) / 3.0 ) * (double)MAX_CONC + 0.5 );
   numCl = (int)( (double)(s->initRHS_Cl) / ( (double)( o->x / 2 - 2 ) * (double)( o->y ) / 3.0 ) * (double)MAX_CONC + 0.5 );

   KOutLbl->setText( QString::number( numK ) + " mM" );
   NaOutLbl->setText( QString::number( numNa ) + " mM" );
   ClOutLbl->setText( QString::number( numCl ) + " mM" );
}


void
NernstGUI::closeEvent( QCloseEvent *event )
{
   sim->quitSim();
   QWidget::closeEvent( event );
}

