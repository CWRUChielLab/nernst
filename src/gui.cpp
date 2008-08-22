/* gui.cpp
 *
 * The main application GUI.
 */


#include <QtGui>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <SFMT.h>
#include <math.h>
#include <fstream>

#include "gui.h"
#include "status.h"
#include "xsim.h"
#include "ctrl.h"
#include "paint.h"
#include "options.h"
#include "atom.h"
#include "world.h"


#include <iostream>


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
   voltsPlot->setTitle( "Membrane Potential" );
   voltsPlot->setAxisTitle( 0, "Potential (mV)" );
   voltsPlot->setAxisTitle( 2, "Time (iters)" );

   numCurves = 4;
   curves = new QwtPlotCurve *[ numCurves ];
   currentNernstCurve = 3;

   curves[ 0 ] = new QwtPlotCurve( "Membrane Potential" );
   curves[ 0 ]->setData( x_iters, y_volts, 0 );
   curves[ 0 ]->attach( voltsPlot );

   /*
   curves[ 1 ] = new QwtPlotCurve( "Equilibrium Potential (Gibbs)" );
   curves[ 1 ]->setPen( QColor( Qt::green ) );
   curves[ 1 ]->setData( x_iters, y_gibbs, 0 );
   curves[ 1 ]->attach( voltsPlot );
   voltsGibbs = 0;

   curves[ 2 ] = new QwtPlotCurve( "Equilibrium Potential (Boltzmann)" );
   curves[ 2 ]->setPen( QColor( Qt::blue ) );
   curves[ 2 ]->setData( x_iters, y_boltzmann, 0 );
   curves[ 2 ]->attach( voltsPlot );
   voltsBoltzmann = 0;
   */

   curves[ currentNernstCurve ] = new QwtPlotCurve( "GHK Potential" );
   curves[ currentNernstCurve ]->setPen( QColor( Qt::red ) );
   curves[ currentNernstCurve ]->setData( x_iters, y_ghk, 0 );
   curves[ currentNernstCurve ]->attach( voltsPlot );
   voltsGHK = 0;

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
   setWindowTitle( "Nernst Potential Simulator | v0.9.0" );
   setWindowIcon( QIcon( ":/img/nernst.png" ) );
   statusBar = new NernstStatusBar( o, this );
   setStatusBar( statusBar );

   // Actions
   saveInitAct = new QAction( "&Save Initial Conditions", this );
   saveInitAct->setStatusTip( "Save the control panel settings for later use" );
   connect( saveInitAct, SIGNAL( triggered() ), this, SLOT( saveInit() ) );

   loadInitAct = new QAction( "&Load Initial Conditions", this );
   loadInitAct->setStatusTip( "Load control panel settings from a file" );
   connect( loadInitAct, SIGNAL( triggered() ), this, SLOT( loadInit() ) );

   saveWorldAct = new QAction( "Save &World", this );
   saveWorldAct->setStatusTip( "Save the state of the world for later simulations" );
   saveWorldAct->setEnabled( 0 );
   connect( saveWorldAct, SIGNAL( triggered() ), this, SLOT( saveWorld() ) );

   loadWorldAct = new QAction( "Load W&orld", this );
   loadWorldAct->setStatusTip( "Load a world state to continue a simulation" );
   connect( loadWorldAct, SIGNAL( triggered() ), this, SLOT( loadWorld() ) );

   quitAct = new QAction( "&Quit", this );
   quitAct->setStatusTip( "Quit the simulator" );
   connect( quitAct, SIGNAL( triggered() ), this, SLOT( close() ) );

   aboutAct = new QAction( "&About", this );
   aboutAct->setStatusTip( "" );
   connect( aboutAct, SIGNAL( triggered() ), this, SLOT( about() ) );

   aboutQtAct = new QAction( "About &Qt", this );
   aboutQtAct->setStatusTip( "" );
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

   helpMenu = menuBar()->addMenu( "&Help" );
   helpMenu->addAction( aboutAct );
   helpMenu->addAction( aboutQtAct );

   // Signals
   connect( this, SIGNAL( settingsLoaded() ), ctrl, SLOT( reloadSettings() ) );
   connect( this, SIGNAL( worldLoaded( int ) ), ctrl, SLOT( reloadSettings() ) );
   connect( this, SIGNAL( worldLoaded( int ) ), ctrl, SLOT( disableCtrl() ) );
   connect( this, SIGNAL( worldLoaded( int ) ), ctrl, SLOT( reenableCtrl() ) );
   connect( this, SIGNAL( worldLoaded( int ) ), canvas, SLOT( startPaint() ) );
   connect( this, SIGNAL( worldLoaded( int ) ), statusBar, SLOT( recalcProgress() ) );
   connect( this, SIGNAL( worldLoaded( int ) ), sim, SLOT( loadWorld( int ) ) );

   connect( sim, SIGNAL( calcEquilibrium() ), this, SLOT( calcEquilibrium() ) );
   connect( sim, SIGNAL( moveCompleted( int ) ), ctrl, SLOT( updateIter( int ) ) );
   connect( sim, SIGNAL( moveCompleted( int ) ), canvas, SLOT( update() ) );
   connect( sim, SIGNAL( moveCompleted( int ) ), this, SLOT( updatePlots( int ) ) );
   connect( sim, SIGNAL( moveCompleted( int ) ), statusBar, SLOT( updateProgressBar( int ) ) );
   connect( sim, SIGNAL( updateStatus( QString ) ), statusBar, SLOT( setStatusLbl( QString ) ) ); 
   connect( sim, SIGNAL( updateVoltsStatus( int, int ) ), statusBar, SLOT( setVoltsLbl( int, int ) ) );
   connect( sim, SIGNAL( finished() ), ctrl, SLOT( reenableCtrl() ) );

   connect( ctrl, SIGNAL( startBtnClicked() ), canvas, SLOT( startPaint() ) );
   connect( ctrl, SIGNAL( startBtnClicked() ), statusBar, SLOT( recalcProgress() ) );
   connect( ctrl, SIGNAL( startBtnClicked() ), this, SLOT( disableLoadInit() ) );
   connect( ctrl, SIGNAL( startBtnClicked() ), this, SLOT( disableLoadWorld() ) );
   connect( ctrl, SIGNAL( startBtnClicked() ), sim, SLOT( runSim() ) );
   connect( ctrl, SIGNAL( pauseBtnClicked() ), sim, SLOT( pauseSim() ) );
   connect( ctrl, SIGNAL( pauseBtnClicked() ), this, SLOT( enableSaveWorld() ) );
   connect( ctrl, SIGNAL( continueBtnClicked() ), statusBar, SLOT( recalcProgress() ) );
   connect( ctrl, SIGNAL( continueBtnClicked() ), this, SLOT( recountIons() ) );
   connect( ctrl, SIGNAL( continueBtnClicked() ), this, SLOT( disableSaveWorld() ) );
   connect( ctrl, SIGNAL( continueBtnClicked() ), sim, SLOT( unpauseSim() ) );
   connect( ctrl, SIGNAL( resetBtnClicked() ), sim, SLOT( resetSim() ) );
   connect( ctrl, SIGNAL( resetBtnClicked() ), canvas, SLOT( resetPaint() ) );
   connect( ctrl, SIGNAL( resetBtnClicked() ), this, SLOT( enableLoadInit() ) );
   connect( ctrl, SIGNAL( resetBtnClicked() ), this, SLOT( enableLoadWorld() ) );
   connect( ctrl, SIGNAL( resetBtnClicked() ), this, SLOT( disableSaveWorld() ) );
   connect( ctrl, SIGNAL( resetBtnClicked() ), this, SLOT( resetPlots() ) );
   connect( ctrl, SIGNAL( resetBtnClicked() ), statusBar, SLOT( resetProgress() ) );
   connect( ctrl, SIGNAL( resetBtnClicked() ), statusBar, SLOT( recalcProgress() ) );
   connect( ctrl, SIGNAL( quitBtnClicked() ), this, SLOT( close() ) );
   connect( ctrl, SIGNAL( worldShrunk() ), this, SLOT( shrinkWindow() ) );
#ifdef BLR_USELINUX
   connect( ctrl, SIGNAL( updatePreview() ), canvas, SLOT( update() ) );
#else
   connect( ctrl, SIGNAL( updatePreview() ), canvas, SLOT( cleanUpdate() ) );
#endif

   connect( canvas, SIGNAL( previewRedrawn() ), this, SLOT( fixRedraw() ) );
}


void
NernstGUI::about()
{
   QMessageBox::about( this, "About Nernst Potential Simulator",
      "<h3>About Nernst Potential Simulator</h3><br>"
      "<br>"
      "Version 0.9.0<br>"
      "Copyright " + QString( 0x00A9 ) + " 2008  "
      "Jeff Gill, Barry Rountree, Kendrick Shaw, "
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
NernstGUI::saveInit()
{
   // Needs fixed with new options.
   /*
   QString fileName;
   fileName = QFileDialog::getSaveFileName( this, "Save Initial Conditions", "settings.init", "Initial Conditions (*.init)" );

   if( fileName == "" )
   {
      return;
   }

   QFile file( fileName );
   if( !file.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, "Initial Conditions", QString("Cannot write file %1:\n%2.")
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
   out << o->pores;           endl( out );
   out << o->randseed;        endl( out );
   out << o->lconc;           endl( out );
   out << o->rconc;           endl( out );
   out << o->selectivity;     endl( out );
   out << o->electrostatics;  endl( out );

   QApplication::restoreOverrideCursor();
   return;
   */
}


void
NernstGUI::loadInit()
{
   // Needs fixed with new options.
   /*
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
   in >> o->pores;
   in >> o->randseed;
   in >> o->lconc;
   in >> o->rconc;
   in >> o->selectivity;
   in >> o->electrostatics;

   QApplication::restoreOverrideCursor();
   emit settingsLoaded();
   return;
   */
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
NernstGUI::recountIons()
{
   int x, y;
   initLHS_K  = 0;
   initLHS_Na = 0;
   initLHS_Cl = 0;
   initRHS_K  = 0;
   initRHS_Na = 0;
   initRHS_Cl = 0;

   // Count up the ions on the LHS
   for( x = 0; x < o->x / 2; x++ )
   {
      for( y = 0; y < o->y; y++ )
      {
         switch( world[ idx( x, y ) ].color )
         {
            case ATOM_K:
            case ATOM_K_TRACK:
               initLHS_K++;
               break;
            case ATOM_Na:
            case ATOM_Na_TRACK:
               initLHS_Na++;
               break;
            case ATOM_Cl:
            case ATOM_Cl_TRACK:
               initLHS_Cl++;
               break;
            default:
               break;
         }
      }
   }

   // Count up the ions on the RHS
   for( x = o->x / 2 + 1; x < o->x; x++ )
   {
      for( y = 0; y < o->y; y++ )
      {
         switch( world[ idx( x, y ) ].color )
         {
            case ATOM_K:
            case ATOM_K_TRACK:
               initRHS_K++;
               break;
            case ATOM_Na:
            case ATOM_Na_TRACK:
               initRHS_Na++;
               break;
            case ATOM_Cl:
            case ATOM_Cl_TRACK:
               initRHS_Cl++;
               break;
            default:
               break;
         }
      }
   }

   calcEquilibrium();
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
   voltsGHK = R * t / F * log( ( ( o->pK * initRHS_K ) + ( o->pNa * initRHS_Na ) + ( o->pCl * initLHS_Cl ) ) /
                               ( ( o->pK * initLHS_K ) + ( o->pNa * initLHS_Na ) + ( o->pCl * initRHS_Cl ) ) ) * 1000;

   /*
   int q;

   // Equilibrium predicted by Gibbs energy minimumization
   q = 1;
   double currentEnergy    = gibbsEnergy( 0 ); 
   double testEnergy       = gibbsEnergy( q ); 
   double nextTestEnergy;

   if( abs( currentEnergy ) > abs( testEnergy ) )              // If this first test moved the value
   {                                                           // closer to zero,
      if( currentEnergy * testEnergy > 0 )                     // and if this first test didn't already
      {                                                        // make the value cross zero,
         nextTestEnergy    = gibbsEnergy( q + 1 ); 
         while( testEnergy * nextTestEnergy > 0 )              // Keep incrementing q until the
         {                                                     // value crosses zero.
            q++;
            testEnergy     = nextTestEnergy; 
            nextTestEnergy = gibbsEnergy( q + 1 );
         }
      }
   } else {                                                    // Else, try the test in the other
      q = -1;                                                  // direction.
      testEnergy           = gibbsEnergy( q );
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
   double currentProbDiff    = boltzmannProbDiff( 0 );
   double testProbDiff       = boltzmannProbDiff( q );
   double nextTestProbDiff;

   if( abs( currentProbDiff ) > abs( testProbDiff ) )          // If this first test moved the value
   {                                                           // closer to zero,
      if( currentProbDiff * testProbDiff > 0 )                 // and if this first test didn't already
      {                                                        // make the value cross zero,
         nextTestProbDiff    = boltzmannProbDiff( q + 1 );
         while( testProbDiff * nextTestProbDiff > 0 )          // Keep incrementing q until the
         {                                                     // value crosses zero.
            q++;
            testProbDiff     = nextTestProbDiff;
            nextTestProbDiff = boltzmannProbDiff( q + 1 );
         }
      }
   } else {                                                    // Else, try the test in the other
      q = -1;                                                  // direction.
      testProbDiff           = boltzmannProbDiff( q );
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

   x_iters[ currentIter ] = currentIter;
   y_volts[ currentIter ] = LRcharge * e / ( c * a * o->y ) * 1000;  // Current membrane potential (mV)
   // y_gibbs[ currentIter ] = voltsGibbs;
   // y_boltzmann[ currentIter ] = voltsBoltzmann;

   curves[ 0 ]->setData( x_iters, y_volts, currentIter );
   // curves[ 1 ]->setData( x_iters, y_gibbs, currentIter );
   // curves[ 2 ]->setData( x_iters, y_boltzmann, currentIter );

   if( o->electrostatics == 1 )
   {
      if( !nernstHasSomeData )
      {
         beganThisNernstCurve = currentIter;
         nernstHasSomeData = 1;
      }
      if( o->pK == 0.0 && o->pNa == 0.0 && o->pCl == 0.0 )
      {
         y_ghk[ currentIter ] = y_volts[ currentIter ];
      } else {
         if( o->selectivity == 0 )
         {
            y_ghk[ currentIter ] = 0;
         } else {
            y_ghk[ currentIter ] = voltsGHK;
         }
      }
      curves[ currentNernstCurve ]->setData( x_iters + beganThisNernstCurve,
                                             y_ghk + beganThisNernstCurve,
                                             currentIter - beganThisNernstCurve + 1 );

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
            curves = new QwtPlotCurve *[ numCurves ];
            for( int i = 0; i < numCurves / 2; i++ )
            {
               curves[ i ] = temp[ i ];
            }
         }
         curves[ currentNernstCurve ] = new QwtPlotCurve( "GHK Potential" );
         curves[ currentNernstCurve ]->setPen( QColor( Qt::red ) );
         curves[ currentNernstCurve ]->attach( voltsPlot );
         nernstHasSomeData = 0;
      }

   }

   voltsPlot->replot();
}


void
NernstGUI::resetPlots()
{
   curves[ 0 ]->setData( x_iters, y_volts, 0 );
   // curves[ 1 ]->setData( x_iters, y_gibbs, 0 );
   // curves[ 2 ]->setData( x_iters, y_boltzmann, 0 );

   currentNernstCurve = 3;
   curves[ currentNernstCurve ]->setData( x_iters, y_ghk, 0 );

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

