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
   capDefault = eps;
   lKDefault = o->lK;
   lNaDefault = o->lNa;
   lClDefault = o->lCl;
   rKDefault = o->rK;
   rNaDefault = o->rNa;
   rClDefault = o->rCl;
   pKDefault = o->pK;
   pNaDefault = o->pNa;
   pClDefault = o->pCl;
   selectivityDefault = o->selectivity;
   electrostaticsDefault = o->electrostatics;

   // Header
   headerLbl = new QLabel( "Control Panel" );
   headerLbl->setAlignment( Qt::AlignHCenter );

   // Iterations control
   itersLbl = new QLabel( "&Iterations" );
   itersLbl->setToolTip( "Set the number of iterations of the simulation\nbetween " + QString::number( MIN_ITERS )
         + " and " + QString::number( MAX_ITERS ) + "." );

   itersSld = new QSlider( Qt::Horizontal );
   itersSld->setMinimumWidth( 100 );
   itersSld->setRange( MIN_ITERS, MAX_ITERS );
   itersSld->setPageStep( 1000 );
   itersSld->setValue( itersDefault );
   itersSld->setToolTip( "Set the number of iterations of the simulation\nbetween " + QString::number( MIN_ITERS )
         + " and " + QString::number( MAX_ITERS ) + "." );

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

   // Capacitance control
   capLbl = new QLabel( "Epsilon" );
   capLbl->setToolTip( "Set the dielectric constant for the membrane." );

   capSld = new QSlider( Qt::Horizontal );
   capSld->setMinimumWidth( 100 );
   capSld->setRange( 1, 10000 );
   capSld->setValue( (int)( capDefault * 10.0 ) );
   capSld->setToolTip( "Set the dielectric constant for the membrane." );

   capLbl->setBuddy( capSld );

   capVal = new QLabel( QString::number( capDefault ) );
   capVal->setAlignment( Qt::AlignRight );

   /*
   capLbl->hide();
   capSld->hide();
   capVal->hide();
   */

   // Seed control
   seedLbl = new QLabel( "See&d" );
   seedLbl->setToolTip( "Set the seed for the random number generator.\nSimulations with matching seeds and world settings\nare identical." );

   seedVal = new QLineEdit( QString::number( o->randseed ) );
   seedVal->setValidator( new QIntValidator( this ) );
   seedVal->setToolTip( "Set the seed for the random number generator.\nSimulations with matching seeds and world settings\nare identical." );

   seedLbl->setBuddy( seedVal );

   // Ion controls
   inLbl = new QLabel( "Intracellular" );
   outLbl = new QLabel( "Extracellular" );
   permLbl = new QLabel( "Permeability" );

   KLbl = new QLabel( "<font color=#ff2600>K<sup>+</sup></font>" );
   NaLbl = new QLabel( "<font color=#0000ff>Na<sup>+</sup></font>" );
   ClLbl = new QLabel( "<font color=#00b259>Cl<sup>-</sup></font>" );

   lKSld = new QSlider( Qt::Horizontal );
   lKSld->setRange( MIN_CONC, MAX_CONC );
   lKSld->setValue( lKDefault );
   lKLbl = new QLabel( QString::number( lKDefault ) + " mM" );

   lNaSld = new QSlider( Qt::Horizontal );
   lNaSld->setRange( MIN_CONC, MAX_CONC );
   lNaSld->setValue( lNaDefault );
   lNaLbl = new QLabel( QString::number( lNaDefault ) + " mM" );

   lClSld = new QSlider( Qt::Horizontal );
   lClSld->setRange( MIN_CONC, MAX_CONC );
   lClSld->setValue( lClDefault );
   lClLbl = new QLabel( QString::number( lClDefault ) + " mM" );

   rKSld = new QSlider( Qt::Horizontal );
   rKSld->setRange( MIN_CONC, MAX_CONC );
   rKSld->setValue( rKDefault );
   rKLbl = new QLabel( QString::number( rKDefault ) + " mM" );

   rNaSld = new QSlider( Qt::Horizontal );
   rNaSld->setRange( MIN_CONC, MAX_CONC );
   rNaSld->setValue( rNaDefault );
   rNaLbl = new QLabel( QString::number( rNaDefault ) + " mM" );

   rClSld = new QSlider( Qt::Horizontal );
   rClSld->setRange( MIN_CONC, MAX_CONC );
   rClSld->setValue( rClDefault );
   rClLbl = new QLabel( QString::number( rClDefault ) + " mM" );

   pKSld = new QSlider( Qt::Horizontal );
   pKSld->setRange( 0, 100 );
   pKSld->setValue( (int)( (double)pKDefault * 100.0 ) );
   pKLbl = new QLabel( QString::number( pKDefault ) );

   pNaSld = new QSlider( Qt::Horizontal );
   pNaSld->setRange( 0, 100 );
   pNaSld->setValue( (int)( (double)pNaDefault * 100.0 ) );
   pNaLbl = new QLabel( QString::number( pNaDefault ) );

   pClSld = new QSlider( Qt::Horizontal );
   pClSld->setRange( 0, 100 );
   pClSld->setValue( (int)( (double)pClDefault * 100.0 ) );
   pClLbl = new QLabel( QString::number( pClDefault ) );

   // Selectivity control
   selectivity = new QCheckBox( "Se&lective Permeability" );
   selectivity->setChecked( selectivityDefault );
   selectivity->setToolTip( "Toggle selectivity of the membrane's pores so that\nonly ions of the right type can pass through any\ngiven pore." );

   // Electrostatics control
   electrostatics = new QCheckBox( "&Electrostatics" );
   electrostatics->setChecked( electrostaticsDefault );
   electrostatics->setToolTip( "Toggle electrostatic forces between ions needed for\nbalancing a diffusion gradient." );

   // Push buttons
   startBtn = new QPushButton( "&Start" );
   pauseBtn = new QPushButton( "&Pause" );
   continueBtn = new QPushButton( "&Continue" );
   clearTrackingBtn = new QPushButton( "Clear &Tracked Ions" );
   clearTrackingBtn->setEnabled( 0 );
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
  
   ctrlLayout->addWidget( capLbl, 4, 0 );
   ctrlLayout->addWidget( capSld, 4, 1 );
   ctrlLayout->addWidget( capVal, 4, 2 );

   ctrlLayout->setColumnMinimumWidth( 2, 50 );

   ctrlLayout->addWidget( seedLbl, 5, 0 );
   ctrlLayout->addWidget( seedVal, 5, 1, 1, 2 );

   sldBox = new QGroupBox();
   sldLayout = new QGridLayout( sldBox );

   sldLayout->addWidget( inLbl, 0, 1 );
   sldLayout->addWidget( outLbl, 0, 2 );
   sldLayout->addWidget( permLbl, 0, 3 );

   sldLayout->addWidget( KLbl, 1, 0 );
   sldLayout->addWidget( lKSld, 1, 1 );
   sldLayout->addWidget( lKLbl, 2, 1 );
   sldLayout->addWidget( rKSld, 1, 2 );
   sldLayout->addWidget( rKLbl, 2, 2 );
   sldLayout->addWidget( pKSld, 1, 3 );
   sldLayout->addWidget( pKLbl, 2, 3 );

   sldLayout->addWidget( NaLbl, 3, 0 );
   sldLayout->addWidget( lNaSld, 3, 1 );
   sldLayout->addWidget( lNaLbl, 4, 1 );
   sldLayout->addWidget( rNaSld, 3, 2 );
   sldLayout->addWidget( rNaLbl, 4, 2 );
   sldLayout->addWidget( pNaSld, 3, 3 );
   sldLayout->addWidget( pNaLbl, 4, 3 );

   sldLayout->addWidget( ClLbl, 5, 0 );
   sldLayout->addWidget( lClSld, 5, 1 );
   sldLayout->addWidget( lClLbl, 6, 1 );
   sldLayout->addWidget( rClSld, 5, 2 );
   sldLayout->addWidget( rClLbl, 6, 2 );
   sldLayout->addWidget( pClSld, 5, 3 );
   sldLayout->addWidget( pClLbl, 6, 3 );

   ctrlLayout->addWidget( sldBox, 6, 0, 1, 3 );

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
   mainLayout->addWidget( clearTrackingBtn );
   mainLayout->addWidget( resetBtn );
   mainLayout->addWidget( quitBtn );

   setLayout( mainLayout );

   // Signals
   connect( itersSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeIters( int ) ) );
   connect( xSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeX( int ) ) );
   connect( ySld, SIGNAL( valueChanged( int ) ), this, SLOT( changeY( int ) ) );
   connect( capSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeCapacitance( int ) ) );
   connect( seedVal, SIGNAL( textChanged( QString ) ), this, SLOT( changeSeed( QString ) ) );
   connect( lKSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeLeftK( int ) ) );
   connect( rKSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeRightK( int ) ) );
   connect( pKSld, SIGNAL( valueChanged( int ) ), this, SLOT( changePermK( int ) ) );
   connect( lNaSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeLeftNa( int ) ) );
   connect( rNaSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeRightNa( int ) ) );
   connect( pNaSld, SIGNAL( valueChanged( int ) ), this, SLOT( changePermNa( int ) ) );
   connect( lClSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeLeftCl( int ) ) );
   connect( rClSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeRightCl( int ) ) );
   connect( pClSld, SIGNAL( valueChanged( int ) ), this, SLOT( changePermCl( int ) ) );
   connect( selectivity, SIGNAL( toggled( bool ) ), this, SLOT( changeSelectivity( bool ) ) );
   connect( electrostatics, SIGNAL( toggled ( bool ) ), this, SLOT( changeElectrostatics( bool ) ) );

   connect( startBtn, SIGNAL( clicked() ), this, SIGNAL( startBtnClicked() ) );
   connect( pauseBtn, SIGNAL( clicked() ), this, SIGNAL( pauseBtnClicked() ) );
   connect( continueBtn, SIGNAL( clicked() ), this, SIGNAL( continueBtnClicked() ) );
   connect( clearTrackingBtn, SIGNAL( clicked() ), this, SIGNAL( clearTrackingBtnClicked() ) );
   connect( resetBtn, SIGNAL( clicked() ), this, SIGNAL( resetBtnClicked() ) );
   connect( quitBtn, SIGNAL( clicked() ), this, SIGNAL( quitBtnClicked() ) );

   connect( this, SIGNAL( startBtnClicked() ), this, SLOT( disableCtrl() ) );
   connect( this, SIGNAL( pauseBtnClicked() ), this, SLOT( reenableCtrl() ) );
   connect( this, SIGNAL( continueBtnClicked() ), this, SLOT( disableCtrl() ) );
   connect( this, SIGNAL( clearTrackingBtnClicked() ), this, SLOT( clearTrackedIons() ) );
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

   shufflePositions( o );
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

   shufflePositions( o );
   emit updatePreview();

   if( o->y < oldy )
   {
      emit worldShrunk();
   }
}


void
NernstCtrl::changeCapacitance( int cap )
{
   eps = (double)cap / 10.0;
   capVal->setNum( eps );
   c = eps * eps0 / d;
   cBoltz = e * e / ( 2 * k * t * c * a );
}


void
NernstCtrl::changeSeed( QString seed )
{
   o->randseed = seed.toInt();
   shufflePositions( o );
   emit updatePreview();
}


void
NernstCtrl::changeLeftK( int lK )
{
   o->lK = lK;
   lKLbl->setText( QString::number( o->lK ) + " mM" );
   emit updatePreview();
}


void
NernstCtrl::changeLeftNa( int lNa )
{
   o->lNa = lNa;
   lNaLbl->setText( QString::number( o->lNa ) + " mM" );
   emit updatePreview();
}


void
NernstCtrl::changeLeftCl( int lCl )
{
   o->lCl = lCl;
   lClLbl->setText( QString::number( o->lCl ) + " mM" );
   emit updatePreview();
}


void
NernstCtrl::changeRightK( int rK )
{
   o->rK = rK;
   rKLbl->setText( QString::number( o->rK ) + " mM" );
   emit updatePreview();
}


void
NernstCtrl::changeRightNa( int rNa )
{
   o->rNa = rNa;
   rNaLbl->setText( QString::number( o->rNa ) + " mM" );
   emit updatePreview();
}


void
NernstCtrl::changeRightCl( int rCl )
{
   o->rCl = rCl;
   rClLbl->setText( QString::number( o->rCl ) + " mM" );
   emit updatePreview();
}


void
NernstCtrl::changePermK( int pK )
{
   o->pK = (double)pK / 100.0;
   pKLbl->setNum( o->pK );
   distributePores( o );
   emit updatePreview();
}


void
NernstCtrl::changePermNa( int pNa )
{
   o->pNa = (double)pNa / 100.0;
   pNaLbl->setNum( o->pNa );
   distributePores( o );
   emit updatePreview();
}


void
NernstCtrl::changePermCl( int pCl )
{
   o->pCl = (double)pCl / 100.0;
   pClLbl->setNum( o->pCl );
   distributePores( o );
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
   emit updatePreview();
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
   emit updatePreview();
}


void
NernstCtrl::reloadSettings()
{
   itersSld->setValue( o->iters );
   xSld->setValue( (int)( log( o->x ) / log( 2 ) + 0.5 ) );
   ySld->setValue( (int)( log( o->y ) / log( 2 ) + 0.5 ) );
   capSld->setValue( (int)( eps * 10.0 ) );
   seedVal->setText( QString::number( o->randseed ) );
   lKSld->setValue( o->lK );
   lNaSld->setValue( o->lNa );
   lClSld->setValue( o->lCl );
   rKSld->setValue( o->rK );
   rNaSld->setValue( o->rNa );
   rClSld->setValue( o->rCl );
   pKSld->setValue( (int)( o->pK * 100.0 ) );
   pNaSld->setValue( (int)( o->pNa * 100.0 ) );
   pClSld->setValue( (int)( o->pCl * 100.0 ) );
   selectivity->setChecked( o->selectivity );
   electrostatics->setChecked( o->electrostatics );
}


void
NernstCtrl::clearTrackedIons()
{
   if( world != NULL )
   {
      for( int x = 0; x < o->x; x++ )
      {
         for( int y = 0; y < o->y; y++ )
         {
            switch( world[ idx( x, y ) ].color )
            {
               case ATOM_K_TRACK:
                  world[ idx( x, y ) ].color = ATOM_K;
                  break;
               case ATOM_Na_TRACK:
                  world[ idx( x, y ) ].color = ATOM_Na;
                  break;
               case ATOM_Cl_TRACK:
                  world[ idx( x, y ) ].color = ATOM_Cl;
                  break;
               default:
                  break;
            }
         }
      }
      emit updatePreview();
   }
}


void
NernstCtrl::disableCtrl()
{
   // Set the first push button to "Pause" and disable (most) controls.
   stackedBtnLayout->setCurrentWidget( pauseBtn );
   clearTrackingBtn->setEnabled( 1 );

   xLbl->setEnabled( 0 );
   xSld->setEnabled( 0 );
   xVal->setEnabled( 0 );

   yLbl->setEnabled( 0 );
   ySld->setEnabled( 0 );
   yVal->setEnabled( 0 );

   itersLbl->setEnabled( 0 );
   itersSld->setEnabled( 0 );
   itersVal->setEnabled( 0 );

   capLbl->setEnabled( 0 );
   capSld->setEnabled( 0 );
   capVal->setEnabled( 0 );

   seedLbl->setEnabled( 0 );
   seedVal->setEnabled( 0 );

   inLbl->setEnabled( 0 );
   outLbl->setEnabled( 0 );
   lKSld->setEnabled( 0 );
   lKLbl->setEnabled( 0 );
   lNaSld->setEnabled( 0 );
   lNaLbl->setEnabled( 0 );
   lClSld->setEnabled( 0 );
   lClLbl->setEnabled( 0 );
   rKSld->setEnabled( 0 );
   rKLbl->setEnabled( 0 );
   rNaSld->setEnabled( 0 );
   rNaLbl->setEnabled( 0 );
   rClSld->setEnabled( 0 );
   rClLbl->setEnabled( 0 );
   sldBox->setEnabled( 0 );

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

   capLbl->setEnabled( 1 );
   capSld->setEnabled( 1 );
   capVal->setEnabled( 1 );

   sldBox->setEnabled( 1 );

   selectivity->setEnabled( 1 );
   electrostatics->setEnabled( 1 );
}


void
NernstCtrl::resetCtrl()
{
   // Set the first push button to "Start", reenable all controls, and reset all control values to defaults.
   stackedBtnLayout->setCurrentWidget( startBtn );
   startBtn->setEnabled( 1 );
   clearTrackingBtn->setEnabled( 0 );

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

   capLbl->setEnabled( 1 );
   capSld->setEnabled( 1 );
   capSld->setValue( (int)( capDefault * 10.0 ) );
   capVal->setEnabled( 1 );

   seedLbl->setEnabled( 1 );
   seedVal->setEnabled( 1 );
   seedVal->setText( QString::number( time( NULL ) ) );

   inLbl->setEnabled( 1 );
   outLbl->setEnabled( 1 );
   lKSld->setEnabled( 1 );
   lKSld->setValue( lKDefault );
   lKLbl->setEnabled( 1 );
   lNaSld->setEnabled( 1 );
   lNaSld->setValue( lNaDefault );
   lNaLbl->setEnabled( 1 );
   lClSld->setEnabled( 1 );
   lClSld->setValue( lClDefault );
   lClLbl->setEnabled( 1 );
   rKSld->setEnabled( 1 );
   rKSld->setValue( rKDefault );
   rKLbl->setEnabled( 1 );
   rNaSld->setEnabled( 1 );
   rNaSld->setValue( rNaDefault );
   rNaLbl->setEnabled( 1 );
   rClSld->setEnabled( 1 );
   rClSld->setValue( rClDefault );
   rClLbl->setEnabled( 1 );
   pKSld->setValue( (int)( pKDefault * 100.0 ) );
   pNaSld->setValue( (int)( pNaDefault * 100.0 ) );
   pClSld->setValue( (int)( pClDefault * 100.0 ) );
   sldBox->setEnabled( 1 );

   selectivity->setEnabled( 1 );
   selectivity->setChecked( selectivityDefault );
   electrostatics->setEnabled( 1 );
   electrostatics->setChecked( electrostaticsDefault );
}

