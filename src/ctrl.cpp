/* ctrl.cpp
 *
 * GUI control panel
 */


#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QValidator>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QStackedLayout>
#include <time.h>
#include <math.h>

#include "ctrl.h"
#include "options.h"
#include "world.h"


CtrlWidget::CtrlWidget( struct options *o, QWidget *parent )
   : QWidget( parent )
{
   // Default values
   itersDefault = o->iters;
   poresDefault = o->pores;
   lspacingDefault = o->lspacing;
   rspacingDefault = o->rspacing;
   selectivityDefault = o->selectivity;
   electrostaticsDefault = o->electrostatics;

   // Header
   headerLbl = new QLabel( "Control Panel" );
   headerLbl->setAlignment( Qt::AlignHCenter );

   // Iterations control
   itersLbl = new QLabel( "&Iterations" );
   itersLbl->setToolTip( "Set the number of iterations of the simulation\nbetween 1 and 1,000,000." );

   itersSld = new QSlider( Qt::Horizontal );
   itersSld->setMinimumWidth( 100 );
   itersSld->setRange( 1, 1000000 );
   itersSld->setPageStep( 1000 );
   itersSld->setValue( itersDefault );
   itersSld->setToolTip( "Set the number of iterations of the simulation\nbetween 1 and 1,000,000." );

   itersLbl->setBuddy( itersSld );

   itersVal = new QLabel( QString::number( itersDefault ) );
   itersVal->setAlignment( Qt::AlignRight );

   // Pores control
   poresLbl = new QLabel( "P&ores" );
   poresLbl->setToolTip( "Set the number of ion channels contained in the\ncentral membrane between 0 and " + QString::number( WORLD_Y ) + "." );

   poresSld = new QSlider( Qt::Horizontal );
   poresSld->setMinimumWidth( 100 );
   poresSld->setRange( 0, WORLD_Y );
   poresSld->setValue( poresDefault );
   poresSld->setToolTip( "Set the number of ion channels contained in the\ncentral membrane between 0 and " + QString::number( WORLD_Y ) + "." );

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

   // Intracellular (left) ion spacing control
   lspacingLbl = new QLabel( "I&ntracellular" );

   lspacingSld = new QSlider( Qt::Horizontal );
   lspacingSld->setMinimumWidth( 100 );
   lspacingSld->setRange( 1, 24 );
   lspacingSld->setValue( lspacingDefault );

   lspacingLbl->setBuddy( lspacingSld );
   
   lspacingVal = new QLabel( QString::number( lspacingDefault ) );
   lspacingVal->setAlignment( Qt::AlignRight );

   // Extracellular (right) ion spacing control
   rspacingLbl = new QLabel( "E&xtracellular" );

   rspacingSld = new QSlider( Qt::Horizontal );
   rspacingSld->setMinimumWidth( 100 );
   rspacingSld->setRange( 1, 24 );
   rspacingSld->setValue( rspacingDefault );

   rspacingLbl->setBuddy( rspacingSld );
   
   rspacingVal = new QLabel( QString::number( rspacingDefault ) );
   rspacingVal->setAlignment( Qt::AlignRight );

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

   ctrlLayout->addWidget( poresLbl, 2, 0 );
   ctrlLayout->addWidget( poresSld, 2, 1 );
   ctrlLayout->addWidget( poresVal, 2, 2 );

   ctrlLayout->setColumnMinimumWidth( 2, 50 );

   ctrlLayout->addWidget( seedLbl, 3, 0 );
   ctrlLayout->addWidget( seedVal, 3, 1, 1, 2 );

   spacingBox = new QGroupBox( "Initial Ionic Spacing" );
   spacingLayout = new QGridLayout( spacingBox );

   spacingLayout->addWidget( lspacingLbl, 0, 0 );
   spacingLayout->addWidget( lspacingSld, 0, 1 );
   spacingLayout->addWidget( lspacingVal, 0, 2 );

   spacingLayout->addWidget( rspacingLbl, 1, 0);
   spacingLayout->addWidget( rspacingSld, 1, 1);
   spacingLayout->addWidget( rspacingVal, 1, 2);

   spacingLayout->setColumnMinimumWidth( 2, 16 );
   ctrlLayout->addWidget( spacingBox, 4, 0, 1, 3 );

   ctrlLayout->addWidget( selectivity, 5, 0, 1, 3 );
   ctrlLayout->addWidget( electrostatics, 6, 0, 1, 3 );

   mainLayout->addLayout( ctrlLayout );
   mainLayout->addStretch( 1 );

   startPauseLayout = new QStackedLayout();
   startPauseLayout->addWidget( startBtn );
   startPauseLayout->addWidget( pauseBtn );
   startPauseLayout->addWidget( continueBtn );
   startPauseLayout->setCurrentIndex( 0 );

   mainLayout->addLayout( startPauseLayout );
   mainLayout->addWidget( resetBtn );
   mainLayout->addWidget( quitBtn );

   setLayout( mainLayout );

   // Signals
   connect( itersSld, SIGNAL( valueChanged( int ) ), itersVal, SLOT( setNum( int ) ) );
   connect( itersSld, SIGNAL( valueChanged( int ) ), this, SIGNAL( itersChanged( int ) ) );
   connect( itersSld, SIGNAL( valueChanged( int ) ), this, SLOT( roundIters( int ) ) );

   connect( poresSld, SIGNAL( valueChanged( int ) ), poresVal, SLOT( setNum( int ) ) );
   connect( poresSld, SIGNAL( valueChanged( int ) ), this, SIGNAL( poresChanged( int ) ) );

   connect( seedVal, SIGNAL( textChanged( QString ) ), this, SIGNAL( seedChanged( QString ) ) );

   connect( lspacingSld, SIGNAL( valueChanged( int ) ), lspacingVal, SLOT( setNum( int ) ) );
   connect( lspacingSld, SIGNAL( valueChanged( int ) ), this, SIGNAL( lspacingChanged( int ) ) );

   connect( rspacingSld, SIGNAL( valueChanged( int ) ), rspacingVal, SLOT( setNum( int ) ) );
   connect( rspacingSld, SIGNAL( valueChanged( int ) ), this, SIGNAL( rspacingChanged( int ) ) );

   connect( selectivity, SIGNAL( toggled( bool ) ), this, SIGNAL( selectivityChanged( bool ) ) );
   connect( electrostatics, SIGNAL( toggled ( bool ) ), this, SIGNAL( electrostaticsChanged( bool ) ) );

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
CtrlWidget::roundIters( int value )
{
   // Round any value set on itersSld to the nearest multiple of 1000.
   int roundedValue = 1000 * (int)( ( (double)value + 500.0 ) / 1000.0 );
   if( value != roundedValue )
   {
      itersSld->setValue( roundedValue );
   }
}


void
CtrlWidget::disableCtrl()
{
   // Set the first push button to "Pause" and disable all controls.
   startPauseLayout->setCurrentIndex( 1 );
   itersLbl->setEnabled( 0 );
   itersSld->setEnabled( 0 );
   itersVal->setEnabled( 0 );
   poresLbl->setEnabled( 0 );
   poresSld->setEnabled( 0 );
   poresVal->setEnabled( 0 );
   seedLbl->setEnabled( 0 );
   seedVal->setEnabled( 0 );
   spacingBox->setEnabled( 0 );
   selectivity->setEnabled( 0 );
   electrostatics->setEnabled( 0 );
}


void
CtrlWidget::reenableCtrl()
{
   // Set the first push button to "Continue" and reenable a few controls.
   startPauseLayout->setCurrentIndex( 2 );
   selectivity->setEnabled( 1 );
   electrostatics->setEnabled( 1 );
}


void
CtrlWidget::resetCtrl()
{
   // Set the first push button to "Start", reenable all controls, and reset all control values to defaults.
   startPauseLayout->setCurrentIndex( 0 );
   startBtn->setEnabled( 1 );
   itersLbl->setEnabled( 1 );
   itersSld->setEnabled( 1 );
   itersSld->setValue( itersDefault );
   itersVal->setEnabled( 1 );
   poresLbl->setEnabled( 1 );
   poresSld->setEnabled( 1 );
   poresSld->setValue( poresDefault );
   poresVal->setEnabled( 1 );
   seedLbl->setEnabled( 1 );
   seedVal->setEnabled( 1 );
   seedVal->setText( QString::number( time( NULL ) ) );
   spacingBox->setEnabled( 1 );
   lspacingSld->setValue( lspacingDefault );
   rspacingSld->setValue( rspacingDefault );
   selectivity->setEnabled( 1 );
   selectivity->setChecked( selectivityDefault );
   electrostatics->setEnabled( 1 );
   electrostatics->setChecked( electrostaticsDefault );
}

void
CtrlWidget::finish()
{
   // Set the first push button to "Start" and disable it.
   startPauseLayout->setCurrentIndex( 0 );
   startBtn->setEnabled( 0 );
}
