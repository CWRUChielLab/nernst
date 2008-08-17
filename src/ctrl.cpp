/* ctrl.cpp
 *
 * GUI control panel
 */


#include <QtGui>
#include <time.h>
#include <math.h>

#include "ctrl.h"
#include "options.h"
#include "atom.h"
#include "world.h"


NernstCtrl::NernstCtrl( struct options *options, QWidget *parent )
   : QWidget( parent )
{
   o = options;
   currentIter = 0;

   // Default values
   itersDefault = o->iters;
   xDefault = o->x;
   yDefault = o->y;
   poresDefault = o->pores;
   lconcDefault = o->lconc;
   rconcDefault = o->rconc;
   selectivityDefault = o->selectivity;
   electrostaticsDefault = o->electrostatics;

   // Header
   headerLbl = new QLabel( "Control Panel" );
   headerLbl->setAlignment( Qt::AlignHCenter );

   // Iterations control
   itersLbl = new QLabel( "&Iterations" );
   itersLbl->setToolTip( "Set the number of iterations of the simulation\nbetween 1 and 100,000." );

   itersSld = new QSlider( Qt::Horizontal );
   itersSld->setMinimumWidth( 100 );
   itersSld->setRange( 1, 100000 );
   itersSld->setPageStep( 1000 );
   itersSld->setValue( itersDefault );
   itersSld->setToolTip( "Set the number of iterations of the simulation\nbetween 1 and 100,000." );

   itersLbl->setBuddy( itersSld );

   itersVal = new QLabel( QString::number( itersDefault ) );
   itersVal->setAlignment( Qt::AlignRight );

   // World width control
   xLbl = new QLabel( "&Width" );
   xLbl->setToolTip( "Set the width of the world." );

   xSld = new QSlider( Qt::Horizontal );
   xSld->setMinimumWidth( 100 );
   xSld->setRange( (int)( log( MIN_X ) / log( 2 ) + 0.5 ), (int)( log( MAX_X ) / log( 2 ) + 0.5 ) );
   xSld->setValue( (int)( log( xDefault ) / log( 2 ) + 0.5 ) );
   xSld->setToolTip( "Set the width of the world." );

   xLbl->setBuddy( xSld );

   xVal = new QLabel( QString::number( xDefault ) );
   xVal->setAlignment( Qt::AlignRight );

   // World height control
   yLbl = new QLabel( "&Height" );
   yLbl->setToolTip( "Set the height of the world." );

   ySld = new QSlider( Qt::Horizontal );
   ySld->setMinimumWidth( 100 );
   ySld->setRange( (int)( log( MIN_Y ) / log( 2 ) + 0.5 ), (int)( log( MAX_Y ) / log( 2 ) + 0.5 ) );
   ySld->setValue( (int)( log( yDefault ) / log( 2 ) + 0.5 ) );
   ySld->setToolTip( "Set the height of the world." );

   yLbl->setBuddy( ySld );

   yVal = new QLabel( QString::number( yDefault ) );
   yVal->setAlignment( Qt::AlignRight );

   // Pores control
   poresLbl = new QLabel( "P&ores" );
   poresLbl->setToolTip( "Set the number of ion channels contained in the\ncentral membrane between 0 and " + QString::number( o->y ) + "." );

   poresSld = new QSlider( Qt::Horizontal );
   poresSld->setMinimumWidth( 100 );
   poresSld->setRange( 0, o->y );
   poresSld->setValue( poresDefault );
   poresSld->setToolTip( "Set the number of ion channels contained in the\ncentral membrane between 0 and " + QString::number( o->y ) + "." );

   poresLbl->setBuddy( poresSld );

   poresVal = new QLabel( QString::number( poresDefault ) );
   poresVal->setAlignment( Qt::AlignRight );

   // Seed control
   seedLbl = new QLabel( "See&d" );
   seedLbl->setToolTip( "Set the seed for the random number generator.\nExperiments with matching seeds and world settings\nare identical." );

   seedVal = new QLineEdit( QString::number( o->randseed ) );
   seedVal->setValidator( new QIntValidator( this ) );
   seedVal->setToolTip( "Set the seed for the random number generator.\nExperiments with matching seeds and world settings\nare identical." );

   seedLbl->setBuddy( seedVal );

   // Intracellular (left) ion concentration control
   lconcLbl = new QLabel( "I&ntracellular" );

   lconcSld = new QSlider( Qt::Horizontal );
   lconcSld->setMinimumWidth( 100 );
   lconcSld->setRange( MIN_CONC, MAX_CONC );
   lconcSld->setValue( lconcDefault );

   lconcLbl->setBuddy( lconcSld );
   
   lconcVal = new QLabel( QString::number( lconcDefault ) + " mM" );
   lconcVal->setAlignment( Qt::AlignRight );

   // Extracellular (right) ion concentration control
   rconcLbl = new QLabel( "E&xtracellular" );

   rconcSld = new QSlider( Qt::Horizontal );
   rconcSld->setMinimumWidth( 100 );
   rconcSld->setRange( MIN_CONC, MAX_CONC );
   rconcSld->setValue( rconcDefault );

   rconcLbl->setBuddy( rconcSld );
   
   rconcVal = new QLabel( QString::number( rconcDefault ) + " mM" );
   rconcVal->setAlignment( Qt::AlignRight );

   // Selectivity control
   selectivity = new QCheckBox( "Se&lective Permeability" );
   selectivity->setChecked( selectivityDefault );
   selectivity->setToolTip( "Toggle selectivity of the membrane's pores so that\nonly potassium ions are permeable to the membrane." );

   // Electrostatics control
   electrostatics = new QCheckBox( "&Electrostatics" );
   electrostatics->setChecked( electrostaticsDefault );
   electrostatics->setToolTip( "Toggle electrostatic forces between ions needed for\nbalancing a diffusion gradient." );

   // Push buttons
   startBtn = new QPushButton( "&Start" );
   pauseBtn = new QPushButton( "&Pause" );
   continueBtn = new QPushButton( "&Continue" );
   resetBtn = new QPushButton( "&Reset" );
   quitBtn = new QPushButton( "&Quit" );

   // Layout
   mainLayout = new QVBoxLayout();
   ctrlLayout = new QGridLayout();

   ctrlLayout->addWidget( headerLbl, 0, 0, 1, 3 );

   ctrlLayout->addWidget( itersLbl, 1, 0 );
   ctrlLayout->addWidget( itersSld, 1, 1 );
   ctrlLayout->addWidget( itersVal, 1, 2 );

   ctrlLayout->addWidget( xLbl, 2, 0 );
   ctrlLayout->addWidget( xSld, 2, 1 );
   ctrlLayout->addWidget( xVal, 2, 2 );

   ctrlLayout->addWidget( yLbl, 3, 0 );
   ctrlLayout->addWidget( ySld, 3, 1 );
   ctrlLayout->addWidget( yVal, 3, 2 );
  
   ctrlLayout->addWidget( poresLbl, 4, 0 );
   ctrlLayout->addWidget( poresSld, 4, 1 );
   ctrlLayout->addWidget( poresVal, 4, 2 );

   ctrlLayout->setColumnMinimumWidth( 2, 50 );

   ctrlLayout->addWidget( seedLbl, 5, 0 );
   ctrlLayout->addWidget( seedVal, 5, 1, 1, 2 );

   concBox = new QGroupBox( "Initial KCl Concentration" );
   concLayout = new QGridLayout( concBox );

   concLayout->addWidget( lconcLbl, 0, 0 );
   concLayout->addWidget( lconcSld, 0, 1 );
   concLayout->addWidget( lconcVal, 0, 2 );

   concLayout->addWidget( rconcLbl, 1, 0 );
   concLayout->addWidget( rconcSld, 1, 1 );
   concLayout->addWidget( rconcVal, 1, 2 );

   concLayout->setColumnMinimumWidth( 2, 60 );
   ctrlLayout->addWidget( concBox, 6, 0, 1, 3 );

   ctrlLayout->addWidget( selectivity, 7, 0, 1, 3 );
   ctrlLayout->addWidget( electrostatics, 8, 0, 1, 3 );

   mainLayout->addLayout( ctrlLayout );
   mainLayout->addStretch( 1 );

   stackedBtnLayout = new QStackedLayout();
   stackedBtnLayout->addWidget( startBtn );
   stackedBtnLayout->addWidget( pauseBtn );
   stackedBtnLayout->addWidget( continueBtn );
   stackedBtnLayout->setCurrentWidget( startBtn );

   mainLayout->addLayout( stackedBtnLayout );
   mainLayout->addWidget( resetBtn );
   mainLayout->addWidget( quitBtn );

   setLayout( mainLayout );

   // Signals
   connect( itersSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeIters( int ) ) );
   connect( xSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeX( int ) ) );
   connect( ySld, SIGNAL( valueChanged( int ) ), this, SLOT( changeY( int ) ) );
   connect( poresSld, SIGNAL( valueChanged( int ) ), this, SLOT( changePores( int ) ) );
   connect( seedVal, SIGNAL( textChanged( QString ) ), this, SLOT( changeSeed( QString ) ) );
   connect( lconcSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeLconc( int ) ) );
   connect( rconcSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeRconc( int ) ) );
   connect( selectivity, SIGNAL( toggled( bool ) ), this, SLOT( changeSelectivity( bool ) ) );
   connect( electrostatics, SIGNAL( toggled ( bool ) ), this, SLOT( changeElectrostatics( bool ) ) );

   connect( startBtn, SIGNAL( clicked() ), this, SIGNAL( startBtnClicked() ) );
   connect( pauseBtn, SIGNAL( clicked() ), this, SIGNAL( pauseBtnClicked() ) );
   connect( continueBtn, SIGNAL( clicked() ), this, SIGNAL( continueBtnClicked() ) );
   connect( resetBtn, SIGNAL( clicked() ), this, SIGNAL( resetBtnClicked() ) );
   connect( quitBtn, SIGNAL( clicked() ), this, SIGNAL( quitBtnClicked() ) );

   connect( this, SIGNAL( startBtnClicked() ), this, SLOT( disableCtrl() ) );
   connect( this, SIGNAL( pauseBtnClicked() ), this, SLOT( reenableCtrl() ) );
   connect( this, SIGNAL( continueBtnClicked() ), this, SLOT( disableCtrl() ) );
   connect( this, SIGNAL( resetBtnClicked() ), this, SLOT( resetCtrl() ) );
}


void
NernstCtrl::updateIter( int iter )
{
   currentIter = iter + 1;
}


void
NernstCtrl::changeIters( int iters )
{
   /*
   // Round any value set on itersSld to the nearest multiple of 1000.
   int roundedIters = 1000 * (int)( ( (double)iters + 500.0 ) / 1000.0 );
   roundedIters = ( roundedIters > 0 ? roundedIters : 1 );
   if( iters != roundedIters )
   {
      itersSld->setValue( roundedIters );
   }
   o->iters = roundedIters;
   itersVal->setNum( o->iters );
   */

   o->iters = iters;
   itersVal->setNum( o->iters );
}


void
NernstCtrl::changeX( int xpow )
{
   int oldx = o->x;

   o->x = (int)pow( 2, xpow );
   xVal->setNum( o->x );

   randomizePositions( o );
   emit updatePreview();

   if( o->x < oldx )
   {
      emit worldShrunk();
   }
}


void
NernstCtrl::changeY( int ypow )
{
   int oldy = o->y;

   o->y = (int)pow( 2, ypow );
   yVal->setNum( o->y );

   randomizePositions( o );
   emit updatePreview();

   poresLbl->setToolTip( "Set the number of ion channels contained in the\ncentral membrane between 0 and " + QString::number( o->y ) + "." );
   poresSld->setRange( 0, o->y );
   poresSld->setToolTip( "Set the number of ion channels contained in the\ncentral membrane between 0 and " + QString::number( o->y ) + "." );

   if( o->y < oldy )
   {
      emit worldShrunk();
   }
}


void
NernstCtrl::changePores( int pores )
{
   o->pores = pores;
   poresVal->setNum( o->pores );
   redistributePores();
   emit updatePreview();
}


void
NernstCtrl::changeSeed( QString seed )
{
   o->randseed = seed.toInt();
}


void
NernstCtrl::changeLconc( int lconc )
{
   o->lconc = lconc;
   lconcVal->setText( QString::number( o->lconc ) + " mM" );
   emit updatePreview();
}


void
NernstCtrl::changeRconc( int rconc )
{
   o->rconc = rconc;
   rconcVal->setText( QString::number( o->rconc ) + " mM" );
   emit updatePreview();
}


void
NernstCtrl::changeSelectivity( bool selectivity )
{
   if( selectivity )
   {
      o->selectivity = 1;
   } else {
      o->selectivity = 0;
   }
}


void
NernstCtrl::changeElectrostatics( bool electrostatics )
{
   if( electrostatics )
   {
      o->electrostatics = 1;
   } else {
      o->electrostatics = 0;
   }
}


void
NernstCtrl::reloadSettings()
{
   itersSld->setValue( o->iters );
   xSld->setValue( (int)( log( o->x ) / log( 2 ) + 0.5 ) );
   ySld->setValue( (int)( log( o->y ) / log( 2 ) + 0.5 ) );
   poresSld->setValue( o->pores );
   seedVal->setText( QString::number( o->randseed ) );
   lconcSld->setValue( o->lconc );
   rconcSld->setValue( o->rconc );
   selectivity->setChecked( o->selectivity );
   electrostatics->setChecked( o->electrostatics );
}


void
NernstCtrl::disableCtrl()
{
   // Set the first push button to "Pause" and disable all controls.
   stackedBtnLayout->setCurrentWidget( pauseBtn );

   xLbl->setEnabled( 0 );
   xSld->setEnabled( 0 );
   xVal->setEnabled( 0 );

   yLbl->setEnabled( 0 );
   ySld->setEnabled( 0 );
   yVal->setEnabled( 0 );

   itersLbl->setEnabled( 0 );
   itersSld->setEnabled( 0 );
   itersVal->setEnabled( 0 );

   poresLbl->setEnabled( 0 );
   poresSld->setEnabled( 0 );
   poresVal->setEnabled( 0 );

   seedLbl->setEnabled( 0 );
   seedVal->setEnabled( 0 );

   concBox->setEnabled( 0 );

   selectivity->setEnabled( 0 );
   electrostatics->setEnabled( 0 );
}


void
NernstCtrl::reenableCtrl()
{
   // Set the first push button to "Continue" and reenable a few controls.
   stackedBtnLayout->setCurrentWidget( continueBtn );

   itersLbl->setEnabled( 1 );
   itersSld->setEnabled( 1 );
   itersSld->setMinimum( currentIter );
   itersVal->setEnabled( 1 );

   poresLbl->setEnabled( 1 );
   poresSld->setEnabled( 1 );
   poresVal->setEnabled( 1 );

   selectivity->setEnabled( 1 );
   electrostatics->setEnabled( 1 );
}


void
NernstCtrl::resetCtrl()
{
   // Set the first push button to "Start", reenable all controls, and reset all control values to defaults.
   stackedBtnLayout->setCurrentWidget( startBtn );
   startBtn->setEnabled( 1 );

   xLbl->setEnabled( 1 );
   xSld->setEnabled( 1 );
   xSld->setValue( (int)( log( xDefault ) / log( 2 ) ) );
   xVal->setEnabled( 1 );

   yLbl->setEnabled( 1 );
   ySld->setEnabled( 1 );
   ySld->setValue( (int)( log( yDefault ) / log( 2 ) ) );
   yVal->setEnabled( 1 );

   itersLbl->setEnabled( 1 );
   itersSld->setEnabled( 1 );
   itersSld->setMinimum( 1 );
   itersSld->setValue( itersDefault );
   itersVal->setEnabled( 1 );

   poresLbl->setEnabled( 1 );
   poresSld->setEnabled( 1 );
   poresSld->setValue( poresDefault );
   poresVal->setEnabled( 1 );

   seedLbl->setEnabled( 1 );
   seedVal->setEnabled( 1 );
   seedVal->setText( QString::number( time( NULL ) ) );

   concBox->setEnabled( 1 );
   lconcSld->setValue( lconcDefault );
   rconcSld->setValue( rconcDefault );

   selectivity->setEnabled( 1 );
   selectivity->setChecked( selectivityDefault );
   electrostatics->setEnabled( 1 );
   electrostatics->setChecked( electrostaticsDefault );
}

