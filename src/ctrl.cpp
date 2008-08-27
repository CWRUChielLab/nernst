/* ctrl.cpp
 *
 * GUI control panel
 *
 * Copyright (c) 2008, Jeffery Gill, Barry Rountree, Kendrick Shaw, 
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

   // Loaded values
   itersLoaded = o->iters;
   xLoaded = o->x;
   yLoaded = o->y;
   capLoaded = eps;
   seedLoaded = o->randseed;
   lKLoaded = o->lK;
   lNaLoaded = o->lNa;
   lClLoaded = o->lCl;
   rKLoaded = o->rK;
   rNaLoaded = o->rNa;
   rClLoaded = o->rCl;
   pKLoaded = o->pK;
   pNaLoaded = o->pNa;
   pClLoaded = o->pCl;
   selectivityLoaded = o->selectivity;
   electrostaticsLoaded = o->electrostatics;

   // Header
   headerLbl = safeNew( QLabel( "<b>Control Panel</b>" ) );
   headerLbl->setAlignment( Qt::AlignHCenter );

   // Iterations control
   itersLbl = safeNew( QLabel( "&Iterations" ) );
   itersLbl->setToolTip( "Set the number of iterations of the simulation\nbetween " + QString::number( MIN_ITERS )
         + " and " + QString::number( MAX_ITERS ) + "." );

   itersSld = safeNew( QSlider( Qt::Horizontal ) );
   itersSld->setMinimumWidth( 100 );
   itersSld->setRange( MIN_ITERS, MAX_ITERS );
   itersSld->setPageStep( 1000 );
   itersSld->setValue( itersDefault );
   itersSld->setToolTip( "Set the number of iterations of the simulation\nbetween " + QString::number( MIN_ITERS )
         + " and " + QString::number( MAX_ITERS ) + "." );

   itersLbl->setBuddy( itersSld );

   itersVal = safeNew( QLabel( QString::number( itersDefault ) ) );
   itersVal->setAlignment( Qt::AlignRight );

   // World width control
   xLbl = safeNew( QLabel( "&Width" ) );
   xLbl->setToolTip( "Set the width of the world." );

   xSld = safeNew( QSlider( Qt::Horizontal ) );
   xSld->setMinimumWidth( 100 );
   xSld->setRange( (int)( log( MIN_X ) / log( 2 ) + 0.5 ), (int)( log( MAX_X ) / log( 2 ) + 0.5 ) );
   xSld->setValue( (int)( log( xDefault ) / log( 2 ) + 0.5 ) );
   xSld->setToolTip( "Set the width of the world." );

   xLbl->setBuddy( xSld );

   xVal = safeNew( QLabel( QString::number( xDefault ) ) );
   xVal->setAlignment( Qt::AlignRight );

   // World height control
   yLbl = safeNew( QLabel( "Hei&ght" ) );
   yLbl->setToolTip( "Set the height of the world." );

   ySld = safeNew( QSlider( Qt::Horizontal ) );
   ySld->setMinimumWidth( 100 );
   ySld->setRange( (int)( log( MIN_Y ) / log( 2 ) + 0.5 ), (int)( log( MAX_Y ) / log( 2 ) + 0.5 ) );
   ySld->setValue( (int)( log( yDefault ) / log( 2 ) + 0.5 ) );
   ySld->setToolTip( "Set the height of the world." );

   yLbl->setBuddy( ySld );

   yVal = safeNew( QLabel( QString::number( yDefault ) ) );
   yVal->setAlignment( Qt::AlignRight );

   // Capacitance control
   capLbl = safeNew( QLabel( "C&apacitance" ) );
   capLbl->setToolTip( "Set the dielectric constant for the membrane." );

   capSld = safeNew( QSlider( Qt::Horizontal ) );
   capSld->setMinimumWidth( 100 );
   capSld->setRange( 1, 10000 );
   capSld->setValue( (int)( capDefault * 10.0 ) );
   capSld->setToolTip( "Set the dielectric constant for the membrane." );

   capLbl->setBuddy( capSld );

   capVal = safeNew( QLabel( QString::number( capDefault ) ) );
   capVal->setAlignment( Qt::AlignRight );

   /*
   capLbl->hide();
   capSld->hide();
   capVal->hide();
   */

   // Seed control
   seedLbl = safeNew( QLabel( "Ra&ndom Start" ) );
   seedLbl->setToolTip( "Set the seed for the random number generator.\nSimulations with matching seeds and world settings\nare identical." );

   seedVal = safeNew( QLineEdit( QString::number( o->randseed ) ) );
   seedVal->setValidator( safeNew( QIntValidator( this ) ) );
   seedVal->setToolTip( "Set the seed for the random number generator.\nSimulations with matching seeds and world settings\nare identical." );

   seedLbl->setBuddy( seedVal );

   // Ion controls
   inLbl = safeNew( QLabel( "Intracellular" ) );
   outLbl = safeNew( QLabel( "Extracellular" ) );
   permLbl = safeNew( QLabel( "Permeability" ) );

   mMLbl1 = safeNew( QLabel( "mM" ) );
   mMLbl2 = safeNew( QLabel( "mM" ) );
   mMLbl3 = safeNew( QLabel( "mM" ) );
   mMLbl4 = safeNew( QLabel( "mM" ) );
   mMLbl5 = safeNew( QLabel( "mM" ) );
   mMLbl6 = safeNew( QLabel( "mM" ) );

   KLbl = safeNew( QLabel( "<font color=#ff2600><b>K<sup>+</sup></b></font>" ) );
   NaLbl = safeNew( QLabel( "<font color=#0000ff><b>Na<sup>+</sup></b></font>" ) );
   ClLbl = safeNew( QLabel( "<font color=#00b259><b>Cl<sup>&ndash;</sup></b></font>" ) );

   lKSld = safeNew( QSlider( Qt::Horizontal ) );
   lKSld->setRange( MIN_CONC, MAX_CONC );
   lKSld->setValue( lKDefault );
   lKVal = safeNew( QLineEdit( QString::number( lKDefault ) ) );
   lKVal->setAlignment( Qt::AlignRight );
   lKVal->setValidator( safeNew( QIntValidator( MIN_CONC, MAX_CONC, this ) ) );

   lNaSld = safeNew( QSlider( Qt::Horizontal ) );
   lNaSld->setRange( MIN_CONC, MAX_CONC );
   lNaSld->setValue( lNaDefault );
   lNaVal = safeNew( QLineEdit( QString::number( lNaDefault ) ) );
   lNaVal->setAlignment( Qt::AlignRight );
   lNaVal->setValidator( safeNew( QIntValidator( MIN_CONC, MAX_CONC, this ) ) );

   lClSld = safeNew( QSlider( Qt::Horizontal ) );
   lClSld->setRange( MIN_CONC, MAX_CONC );
   lClSld->setValue( lClDefault );
   lClVal = safeNew( QLineEdit( QString::number( lClDefault ) ) );
   lClVal->setAlignment( Qt::AlignRight );
   lClVal->setValidator( safeNew( QIntValidator( MIN_CONC, MAX_CONC, this ) ) );

   rKSld = safeNew( QSlider( Qt::Horizontal ) );
   rKSld->setRange( MIN_CONC, MAX_CONC );
   rKSld->setValue( rKDefault );
   rKVal = safeNew( QLineEdit( QString::number( rKDefault ) ) );
   rKVal->setAlignment( Qt::AlignRight );
   rKVal->setValidator( safeNew( QIntValidator( MIN_CONC, MAX_CONC, this ) ) );

   rNaSld = safeNew( QSlider( Qt::Horizontal ) );
   rNaSld->setRange( MIN_CONC, MAX_CONC );
   rNaSld->setValue( rNaDefault );
   rNaVal = safeNew( QLineEdit( QString::number( rNaDefault ) ) );
   rNaVal->setAlignment( Qt::AlignRight );
   rNaVal->setValidator( safeNew( QIntValidator( MIN_CONC, MAX_CONC, this ) ) );

   rClSld = safeNew( QSlider( Qt::Horizontal ) );
   rClSld->setRange( MIN_CONC, MAX_CONC );
   rClSld->setValue( rClDefault );
   rClVal = safeNew( QLineEdit( QString::number( rClDefault ) ) );
   rClVal->setAlignment( Qt::AlignRight );
   rClVal->setValidator( safeNew( QIntValidator( MIN_CONC, MAX_CONC, this ) ) );

   pKSld = safeNew( QSlider( Qt::Horizontal ) );
   pKSld->setRange( 0, 100 );
   pKSld->setValue( (int)( (double)pKDefault * 100.0 ) );
   pKVal = safeNew( QLineEdit( QString::number( pKDefault ) ) );
   pKVal->setAlignment( Qt::AlignRight );
   pKVal->setValidator( safeNew( QDoubleValidator( 0.00, 1.00, 2, this ) ) );

   pNaSld = safeNew( QSlider( Qt::Horizontal ) );
   pNaSld->setRange( 0, 100 );
   pNaSld->setValue( (int)( (double)pNaDefault * 100.0 ) );
   pNaVal = safeNew( QLineEdit( QString::number( pNaDefault ) ) );
   pNaVal->setAlignment( Qt::AlignRight );
   pNaVal->setValidator( safeNew( QDoubleValidator( 0.00, 1.00, 2, this ) ) );

   pClSld = safeNew( QSlider( Qt::Horizontal ) );
   pClSld->setRange( 0, 100 );
   pClSld->setValue( (int)( (double)pClDefault * 100.0 ) );
   pClVal = safeNew( QLineEdit( QString::number( pClDefault ) ) );
   pClVal->setAlignment( Qt::AlignRight );
   pClVal->setValidator( safeNew( QDoubleValidator( 0.00, 1.00, 2, this ) ) );

   // Selectivity control
   selectivity = safeNew( QCheckBox( "Selective &Permeability" ) );
   selectivity->setChecked( selectivityDefault );
   selectivity->setToolTip( "Toggle selectivity of the membrane's pores so that\nonly ions of the right type can pass through any\ngiven pore." );

   // Electrostatics control
   electrostatics = safeNew( QCheckBox( "&Electrostatics" ) );
   electrostatics->setChecked( electrostaticsDefault );
   electrostatics->setToolTip( "Toggle electrostatic forces between ions needed for\nbalancing a diffusion gradient." );

   // Push buttons
   startBtn = safeNew( QPushButton( "&Start" ) );
   pauseBtn = safeNew( QPushButton( "&Pause" ) );
   continueBtn = safeNew( QPushButton( "&Continue" ) );
   resetCurrentBtn = safeNew( QPushButton( "&Reset with Same Initial Conditions" ) );
   resetCurrentBtn->setEnabled( 0 );
   resetLoadedBtn = safeNew( QPushButton( "Reset with &Loaded Initial Conditions" ) );
   resetLoadedBtn->setEnabled( 0 );
   resetDefaultBtn = safeNew( QPushButton( "Reset with &Default Settings" ) );
   quitBtn = safeNew( QPushButton( "&Quit" ) );

   // Layout
   mainLayout = safeNew( QVBoxLayout() );
   ctrlLayout = safeNew( QGridLayout() );

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

   sldBox = safeNew( QGroupBox() );
   sldLayout = safeNew( QGridLayout( sldBox ) );

   sldLayout->addWidget( inLbl, 0, 1, 1, 2 );
   sldLayout->addWidget( outLbl, 0, 3, 1, 2 );
   sldLayout->addWidget( permLbl, 0, 5 );

   sldLayout->addWidget( KLbl, 1, 0, 2, 1 );
   sldLayout->addWidget( lKSld, 1, 1, 1, 2 );
   sldLayout->addWidget( lKVal, 2, 1 );
   sldLayout->addWidget( mMLbl1, 2, 2 );
   sldLayout->addWidget( rKSld, 1, 3, 1, 2 );
   sldLayout->addWidget( rKVal, 2, 3 );
   sldLayout->addWidget( mMLbl2, 2, 4 );
   sldLayout->addWidget( pKSld, 1, 5 );
   sldLayout->addWidget( pKVal, 2, 5 );

   sldLayout->addWidget( NaLbl, 4, 0, 2, 1 );
   sldLayout->addWidget( lNaSld, 4, 1, 1, 2 );
   sldLayout->addWidget( lNaVal, 5, 1 );
   sldLayout->addWidget( mMLbl3, 5, 2 );
   sldLayout->addWidget( rNaSld, 4, 3, 1, 2 );
   sldLayout->addWidget( rNaVal, 5, 3 );
   sldLayout->addWidget( mMLbl4, 5, 4 );
   sldLayout->addWidget( pNaSld, 4, 5 );
   sldLayout->addWidget( pNaVal, 5, 5 );

   sldLayout->addWidget( ClLbl, 7, 0, 2, 1 );
   sldLayout->addWidget( lClSld, 7, 1, 1, 2 );
   sldLayout->addWidget( lClVal, 8, 1 );
   sldLayout->addWidget( mMLbl5, 8, 2 );
   sldLayout->addWidget( rClSld, 7, 3, 1, 2 );
   sldLayout->addWidget( rClVal, 8, 3 );
   sldLayout->addWidget( mMLbl6, 8, 4 );
   sldLayout->addWidget( pClSld, 7, 5 );
   sldLayout->addWidget( pClVal, 8, 5 );

   sldLayout->setRowMinimumHeight( 3, 10 );
   sldLayout->setRowMinimumHeight( 6, 10 );
   sldBox->setMaximumWidth( sldBox->minimumSizeHint().rwidth() );

   ctrlLayout->addWidget( sldBox, 6, 0, 1, 3 );

   checkBoxesLayout = safeNew( QHBoxLayout() );
   checkBoxesLayout->addWidget( selectivity );
   checkBoxesLayout->addWidget( electrostatics );

   ctrlLayout->addLayout( checkBoxesLayout, 7, 0, 1, 3 );

   mainLayout->addLayout( ctrlLayout );
   mainLayout->addStretch( 1 );

   stackedBtnLayout = safeNew( QStackedLayout() );
   stackedBtnLayout->addWidget( startBtn );
   stackedBtnLayout->addWidget( pauseBtn );
   stackedBtnLayout->addWidget( continueBtn );
   stackedBtnLayout->setCurrentWidget( startBtn );

   mainLayout->addLayout( stackedBtnLayout );
   mainLayout->addWidget( resetCurrentBtn );
   mainLayout->addWidget( resetLoadedBtn );
   mainLayout->addWidget( resetDefaultBtn );
   mainLayout->addWidget( quitBtn );

   setLayout( mainLayout );

   // Signals
   connect( itersSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeIters( int ) ) );
   connect( xSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeX( int ) ) );
   connect( ySld, SIGNAL( valueChanged( int ) ), this, SLOT( changeY( int ) ) );
   connect( capSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeCapacitance( int ) ) );
   connect( seedVal, SIGNAL( textChanged( QString ) ), this, SLOT( changeSeed( QString ) ) );

   connect( lKSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeLeftK( int ) ) );
   connect( lKVal, SIGNAL( textChanged( QString ) ), this, SLOT( changeLeftK( QString ) ) );
   connect( rKSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeRightK( int ) ) );
   connect( rKVal, SIGNAL( textChanged( QString ) ), this, SLOT( changeRightK( QString ) ) );
   connect( pKSld, SIGNAL( valueChanged( int ) ), this, SLOT( changePermK( int ) ) );
   connect( pKVal, SIGNAL( textChanged( QString ) ), this, SLOT( changePermK( QString ) ) );

   connect( lNaSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeLeftNa( int ) ) );
   connect( lNaVal, SIGNAL( textChanged( QString ) ), this, SLOT( changeLeftNa( QString ) ) );
   connect( rNaSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeRightNa( int ) ) );
   connect( rNaVal, SIGNAL( textChanged( QString ) ), this, SLOT( changeRightNa( QString ) ) );
   connect( pNaSld, SIGNAL( valueChanged( int ) ), this, SLOT( changePermNa( int ) ) );
   connect( pNaVal, SIGNAL( textChanged( QString ) ), this, SLOT( changePermNa( QString ) ) );

   connect( lClSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeLeftCl( int ) ) );
   connect( lClVal, SIGNAL( textChanged( QString ) ), this, SLOT( changeLeftCl( QString ) ) );
   connect( rClSld, SIGNAL( valueChanged( int ) ), this, SLOT( changeRightCl( int ) ) );
   connect( rClVal, SIGNAL( textChanged( QString ) ), this, SLOT( changeRightCl( QString ) ) );
   connect( pClSld, SIGNAL( valueChanged( int ) ), this, SLOT( changePermCl( int ) ) );
   connect( pClVal, SIGNAL( textChanged( QString ) ), this, SLOT( changePermCl( QString ) ) );

   connect( selectivity, SIGNAL( toggled( bool ) ), this, SLOT( changeSelectivity( bool ) ) );
   connect( electrostatics, SIGNAL( toggled ( bool ) ), this, SLOT( changeElectrostatics( bool ) ) );

   connect( startBtn, SIGNAL( clicked() ), this, SIGNAL( startBtnClicked() ) );
   connect( pauseBtn, SIGNAL( clicked() ), this, SIGNAL( pauseBtnClicked() ) );
   connect( continueBtn, SIGNAL( clicked() ), this, SIGNAL( continueBtnClicked() ) );
   connect( resetCurrentBtn, SIGNAL( clicked() ), this, SIGNAL( resetCurrentBtnClicked() ) );
   connect( resetLoadedBtn, SIGNAL( clicked() ), this, SIGNAL( resetLoadedBtnClicked() ) );
   connect( resetDefaultBtn, SIGNAL( clicked() ), this, SIGNAL( resetDefaultBtnClicked() ) );
   connect( quitBtn, SIGNAL( clicked() ), this, SIGNAL( quitBtnClicked() ) );

   connect( this, SIGNAL( startBtnClicked() ), this, SLOT( disableCtrl() ) );
   connect( this, SIGNAL( pauseBtnClicked() ), this, SLOT( reenableCtrl() ) );
   connect( this, SIGNAL( continueBtnClicked() ), this, SLOT( disableCtrl() ) );
   connect( this, SIGNAL( resetCurrentBtnClicked() ), this, SLOT( resetCurrentCtrl() ) );
   connect( this, SIGNAL( resetLoadedBtnClicked() ), this, SLOT( resetLoadedCtrl() ) );
   connect( this, SIGNAL( resetDefaultBtnClicked() ), this, SLOT( resetDefaultCtrl() ) );
}


void
NernstCtrl::updateIter( int iter )
{
   currentIter = iter + 1;
}


void
NernstCtrl::setNewLoadedSettings()
{
   itersLoaded = o->iters;
   xLoaded = o->x;
   yLoaded = o->y;
   capLoaded = eps;
   seedLoaded = o->randseed;
   lKLoaded = o->lK;
   lNaLoaded = o->lNa;
   lClLoaded = o->lCl;
   rKLoaded = o->rK;
   rNaLoaded = o->rNa;
   rClLoaded = o->rCl;
   pKLoaded = o->pK;
   pNaLoaded = o->pNa;
   pClLoaded = o->pCl;
   selectivityLoaded = o->selectivity;
   electrostaticsLoaded = o->electrostatics;

   resetLoadedBtn->setEnabled( 1 );
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
   o->x = (int)pow( 2, xpow );
   xVal->setNum( o->x );

   shufflePositions( o );
   emit worldSizeChange();
   emit updatePreview();
}


void
NernstCtrl::changeY( int ypow )
{
   o->y = (int)pow( 2, ypow );
   yVal->setNum( o->y );

   shufflePositions( o );
   emit worldSizeChange();
   emit updatePreview();
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
   if( lK != o->lK )
   {
      o->lK = lK;
      lKVal->setText( QString::number( o->lK ) );
      emit updatePreview();
      emit adjustTable();
   }
}


void
NernstCtrl::changeLeftK( QString lK )
{
   if( lK.toInt() != o->lK )
   {
      o->lK = lK.toInt();
      lKSld->setValue( o->lK );
      emit updatePreview();
      emit adjustTable();
   }
}


void
NernstCtrl::changeLeftNa( int lNa )
{
   if( lNa != o->lNa )
   {
      o->lNa = lNa;
      lNaVal->setText( QString::number( o->lNa ) );
      emit updatePreview();
      emit adjustTable();
   }
}


void
NernstCtrl::changeLeftNa( QString lNa )
{
   if( lNa.toInt() != o->lNa )
   {
      o->lNa = lNa.toInt();
      lNaSld->setValue( o->lNa );
      emit updatePreview();
      emit adjustTable();
   }
}


void
NernstCtrl::changeLeftCl( int lCl )
{
   if( lCl != o->lCl )
   {
      o->lCl = lCl;
      lClVal->setText( QString::number( o->lCl ) );
      emit updatePreview();
      emit adjustTable();
   }
}


void
NernstCtrl::changeLeftCl( QString lCl )
{
   if( lCl.toInt() != o->lCl )
   {
      o->lCl = lCl.toInt();
      lClSld->setValue( o->lCl );
      emit updatePreview();
      emit adjustTable();
   }
}


void
NernstCtrl::changeRightK( int rK )
{
   if( rK != o->rK )
   {
      o->rK = rK;
      rKVal->setText( QString::number( o->rK ) );
      emit updatePreview();
      emit adjustTable();
   }
}


void
NernstCtrl::changeRightK( QString rK )
{
   if( rK.toInt() != o->rK )
   {
      o->rK = rK.toInt();
      rKSld->setValue( o->rK );
      emit updatePreview();
      emit adjustTable();
   }
}


void
NernstCtrl::changeRightNa( int rNa )
{
   if( rNa != o->rNa )
   {
      o->rNa = rNa;
      rNaVal->setText( QString::number( o->rNa ) );
      emit updatePreview();
      emit adjustTable();
   }
}


void
NernstCtrl::changeRightNa( QString rNa )
{
   if( rNa.toInt() != o->rNa )
   {
      o->rNa = rNa.toInt();
      rNaSld->setValue( o->rNa );
      emit updatePreview();
      emit adjustTable();
   }
}


void
NernstCtrl::changeRightCl( int rCl )
{
   if( rCl != o->rCl )
   {
      o->rCl = rCl;
      rClVal->setText( QString::number( o->rCl ) );
      emit updatePreview();
      emit adjustTable();
   }
}


void
NernstCtrl::changeRightCl( QString rCl )
{
   if( rCl.toInt() != o->rCl )
   {
      o->rCl = rCl.toInt();
      rClSld->setValue( o->rCl );
      emit updatePreview();
      emit adjustTable();
   }
}


void
NernstCtrl::changePermK( int pK )
{
   if( (double)pK / 100.0 != o->pK )
   {
      o->pK = (double)pK / 100.0;
      pKVal->setText( QString::number( o->pK ) );
      distributePores( o );
      emit updatePreview();
   }
}


void
NernstCtrl::changePermK( QString pK )
{
   if( pK.toDouble() < 0 )
   {
      pK = QString::number( 0.00 );
      pKVal->setText( pK );
   }

   if( pK.toDouble() > 1 )
   {
      pK = QString::number( 1.00 );
      pKVal->setText( pK );
   }

   if( pK.toDouble() != o->pK )
   {
      o->pK = pK.toDouble();
      pKSld->setValue( (int)( o->pK * 100.0 ) );
      distributePores( o );
      emit updatePreview();
   }
}


void
NernstCtrl::changePermNa( int pNa )
{
   if( (double)pNa / 100.0 != o->pNa )
   {
      o->pNa = (double)pNa / 100.0;
      pNaVal->setText( QString::number( o->pNa ) );
      distributePores( o );
      emit updatePreview();
   }
}


void
NernstCtrl::changePermNa( QString pNa )
{
   if( pNa.toDouble() < 0 )
   {
      pNa = QString::number( 0.00 );
      pNaVal->setText( pNa );
   }

   if( pNa.toDouble() > 1 )
   {
      pNa = QString::number( 1.00 );
      pNaVal->setText( pNa );
   }

   if( pNa.toDouble() != o->pNa )
   {
      o->pNa = pNa.toDouble();
      pNaSld->setValue( (int)( o->pNa * 100.0 ) );
      distributePores( o );
      emit updatePreview();
   }
}


void
NernstCtrl::changePermCl( int pCl )
{
   if( (double)pCl / 100.0 != o->pCl )
   {
      o->pCl = (double)pCl / 100.0;
      pClVal->setText( QString::number( o->pCl ) );
      distributePores( o );
      emit updatePreview();
   }
}


void
NernstCtrl::changePermCl( QString pCl )
{
   if( pCl.toDouble() < 0 )
   {
      pCl = QString::number( 0.00 );
      pClVal->setText( pCl );
   }

   if( pCl.toDouble() > 1 )
   {
      pCl = QString::number( 1.00 );
      pClVal->setText( pCl );
   }

   if( pCl.toDouble() != o->pCl )
   {
      o->pCl = pCl.toDouble();
      pClSld->setValue( (int)( o->pCl * 100.0 ) );
      distributePores( o );
      emit updatePreview();
   }
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
   capVal->setText( QString::number( eps ) );
   seedVal->setText( QString::number( o->randseed ) );
   lKSld->setValue( o->lK );
   lKVal->setText( QString::number( o->lK ) );
   lNaSld->setValue( o->lNa );
   lNaVal->setText( QString::number( o->lNa ) );
   lClSld->setValue( o->lCl );
   lClVal->setText( QString::number( o->lCl ) );
   rKSld->setValue( o->rK );
   rKVal->setText( QString::number( o->rK ) );
   rNaSld->setValue( o->rNa );
   rNaVal->setText( QString::number( o->rNa ) );
   rClSld->setValue( o->rCl );
   rClVal->setText( QString::number( o->rCl ) );
   pKSld->setValue( (int)( o->pK * 100.0 ) );
   pKVal->setText( QString::number( o->pK ) );
   pNaSld->setValue( (int)( o->pNa * 100.0 ) );
   pNaVal->setText( QString::number( o->pNa ) );
   pClSld->setValue( (int)( o->pCl * 100.0 ) );
   pClVal->setText( QString::number( o->pCl ) );
   selectivity->setChecked( o->selectivity );
   electrostatics->setChecked( o->electrostatics );
   adjustTable();
}


void
NernstCtrl::disableCtrl()
{
   // Set the first push button to "Pause" and disable the controls.
   stackedBtnLayout->setCurrentWidget( pauseBtn );
   resetCurrentBtn->setEnabled( 1 );

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
   permLbl->setEnabled( 0 );
   mMLbl1->setEnabled( 0 );
   mMLbl2->setEnabled( 0 );
   mMLbl3->setEnabled( 0 );
   mMLbl4->setEnabled( 0 );
   mMLbl5->setEnabled( 0 );
   mMLbl6->setEnabled( 0 );
   lKSld->setEnabled( 0 );
   lKVal->setEnabled( 0 );
   lNaSld->setEnabled( 0 );
   lNaVal->setEnabled( 0 );
   lClSld->setEnabled( 0 );
   lClVal->setEnabled( 0 );
   rKSld->setEnabled( 0 );
   rKVal->setEnabled( 0 );
   rNaSld->setEnabled( 0 );
   rNaVal->setEnabled( 0 );
   rClSld->setEnabled( 0 );
   rClVal->setEnabled( 0 );
   pKSld->setEnabled( 0 );
   pKVal->setEnabled( 0 );
   pNaSld->setEnabled( 0 );
   pNaVal->setEnabled( 0 );
   pClSld->setEnabled( 0 );
   pClVal->setEnabled( 0 );

   selectivity->setEnabled( 0 );
   electrostatics->setEnabled( 0 );
}


void
NernstCtrl::reenableCtrl()
{
   // Set the first push button to "Continue" and reenable a few controls.
   stackedBtnLayout->setCurrentWidget( continueBtn );
   resetCurrentBtn->setEnabled( 1 );

   itersLbl->setEnabled( 1 );
   itersSld->setEnabled( 1 );
   itersSld->setMinimum( currentIter );
   itersVal->setEnabled( 1 );

   capLbl->setEnabled( 1 );
   capSld->setEnabled( 1 );
   capVal->setEnabled( 1 );

   permLbl->setEnabled( 1 );
   pKSld->setEnabled( 1 );
   pKVal->setEnabled( 1 );
   pNaSld->setEnabled( 1 );
   pNaVal->setEnabled( 1 );
   pClSld->setEnabled( 1 );
   pClVal->setEnabled( 1 );

   selectivity->setEnabled( 1 );
   electrostatics->setEnabled( 1 );
}


void
NernstCtrl::resetCurrentCtrl()
{
   // Set the first push button to "Start", reenable all controls, but keep the current values.
   stackedBtnLayout->setCurrentWidget( startBtn );
   startBtn->setEnabled( 1 );
   resetCurrentBtn->setEnabled( 0 );

   xLbl->setEnabled( 1 );
   xSld->setEnabled( 1 );
   xVal->setEnabled( 1 );

   yLbl->setEnabled( 1 );
   ySld->setEnabled( 1 );
   yVal->setEnabled( 1 );

   itersLbl->setEnabled( 1 );
   itersSld->setEnabled( 1 );
   itersSld->setMinimum( 1 );
   itersVal->setEnabled( 1 );

   capLbl->setEnabled( 1 );
   capSld->setEnabled( 1 );
   capVal->setEnabled( 1 );

   seedLbl->setEnabled( 1 );
   seedVal->setEnabled( 1 );

   inLbl->setEnabled( 1 );
   outLbl->setEnabled( 1 );
   permLbl->setEnabled( 1 );
   mMLbl1->setEnabled( 1 );
   mMLbl2->setEnabled( 1 );
   mMLbl3->setEnabled( 1 );
   mMLbl4->setEnabled( 1 );
   mMLbl5->setEnabled( 1 );
   mMLbl6->setEnabled( 1 );
   lKSld->setEnabled( 1 );
   lKVal->setEnabled( 1 );
   lNaSld->setEnabled( 1 );
   lNaVal->setEnabled( 1 );
   lClSld->setEnabled( 1 );
   lClVal->setEnabled( 1 );
   rKSld->setEnabled( 1 );
   rKVal->setEnabled( 1 );
   rNaSld->setEnabled( 1 );
   rNaVal->setEnabled( 1 );
   rClSld->setEnabled( 1 );
   rClVal->setEnabled( 1 );
   pKSld->setEnabled( 1 );
   pKVal->setEnabled( 1 );
   pNaSld->setEnabled( 1 );
   pNaVal->setEnabled( 1 );
   pClSld->setEnabled( 1 );
   pClVal->setEnabled( 1 );

   selectivity->setEnabled( 1 );
   electrostatics->setEnabled( 1 );

   adjustTable();
}


void
NernstCtrl::resetLoadedCtrl()
{
   // Set the first push button to "Start", reenable all controls, and load the most recent loaded values.
   stackedBtnLayout->setCurrentWidget( startBtn );
   startBtn->setEnabled( 1 );
   resetCurrentBtn->setEnabled( 0 );

   xLbl->setEnabled( 1 );
   xSld->setEnabled( 1 );
   xSld->setValue( (int)( log( xLoaded ) / log( 2 ) ) );
   xVal->setEnabled( 1 );

   yLbl->setEnabled( 1 );
   ySld->setEnabled( 1 );
   ySld->setValue( (int)( log( yLoaded ) / log( 2 ) ) );
   yVal->setEnabled( 1 );

   itersLbl->setEnabled( 1 );
   itersSld->setEnabled( 1 );
   itersSld->setMinimum( 1 );
   itersSld->setValue( itersLoaded );
   itersVal->setEnabled( 1 );

   capLbl->setEnabled( 1 );
   capSld->setEnabled( 1 );
   capSld->setValue( (int)( capLoaded * 10.0 ) );
   capVal->setEnabled( 1 );

   seedLbl->setEnabled( 1 );
   seedVal->setEnabled( 1 );
   seedVal->setText( QString::number( seedLoaded ) );

   inLbl->setEnabled( 1 );
   outLbl->setEnabled( 1 );
   permLbl->setEnabled( 1 );
   mMLbl1->setEnabled( 1 );
   mMLbl2->setEnabled( 1 );
   mMLbl3->setEnabled( 1 );
   mMLbl4->setEnabled( 1 );
   mMLbl5->setEnabled( 1 );
   mMLbl6->setEnabled( 1 );
   lKSld->setEnabled( 1 );
   lKSld->setValue( lKLoaded );
   lKVal->setEnabled( 1 );
   lNaSld->setEnabled( 1 );
   lNaSld->setValue( lNaLoaded );
   lNaVal->setEnabled( 1 );
   lClSld->setEnabled( 1 );
   lClSld->setValue( lClLoaded );
   lClVal->setEnabled( 1 );
   rKSld->setEnabled( 1 );
   rKSld->setValue( rKLoaded );
   rKVal->setEnabled( 1 );
   rNaSld->setEnabled( 1 );
   rNaSld->setValue( rNaLoaded );
   rNaVal->setEnabled( 1 );
   rClSld->setEnabled( 1 );
   rClSld->setValue( rClLoaded );
   rClVal->setEnabled( 1 );
   pKSld->setEnabled( 1 );
   pKSld->setValue( (int)( pKLoaded * 100.0 ) );
   pKVal->setEnabled( 1 );
   pNaSld->setEnabled( 1 );
   pNaSld->setValue( (int)( pNaLoaded * 100.0 ) );
   pNaVal->setEnabled( 1 );
   pClSld->setEnabled( 1 );
   pClSld->setValue( (int)( pClLoaded * 100.0 ) );
   pClVal->setEnabled( 1 );

   selectivity->setEnabled( 1 );
   selectivity->setChecked( selectivityLoaded );
   electrostatics->setEnabled( 1 );
   electrostatics->setChecked( electrostaticsLoaded );

   adjustTable();
}


void
NernstCtrl::resetDefaultCtrl()
{
   // Set the first push button to "Start", reenable all controls, and reset all control values to defaults.
   stackedBtnLayout->setCurrentWidget( startBtn );
   startBtn->setEnabled( 1 );
   resetCurrentBtn->setEnabled( 0 );

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
   permLbl->setEnabled( 1 );
   mMLbl1->setEnabled( 1 );
   mMLbl2->setEnabled( 1 );
   mMLbl3->setEnabled( 1 );
   mMLbl4->setEnabled( 1 );
   mMLbl5->setEnabled( 1 );
   mMLbl6->setEnabled( 1 );
   lKSld->setEnabled( 1 );
   lKSld->setValue( lKDefault );
   lKVal->setEnabled( 1 );
   lNaSld->setEnabled( 1 );
   lNaSld->setValue( lNaDefault );
   lNaVal->setEnabled( 1 );
   lClSld->setEnabled( 1 );
   lClSld->setValue( lClDefault );
   lClVal->setEnabled( 1 );
   rKSld->setEnabled( 1 );
   rKSld->setValue( rKDefault );
   rKVal->setEnabled( 1 );
   rNaSld->setEnabled( 1 );
   rNaSld->setValue( rNaDefault );
   rNaVal->setEnabled( 1 );
   rClSld->setEnabled( 1 );
   rClSld->setValue( rClDefault );
   rClVal->setEnabled( 1 );
   pKSld->setEnabled( 1 );
   pKSld->setValue( (int)( pKDefault * 100.0 ) );
   pKVal->setEnabled( 1 );
   pNaSld->setEnabled( 1 );
   pNaSld->setValue( (int)( pNaDefault * 100.0 ) );
   pNaVal->setEnabled( 1 );
   pClSld->setEnabled( 1 );
   pClSld->setValue( (int)( pClDefault * 100.0 ) );
   pClVal->setEnabled( 1 );

   selectivity->setEnabled( 1 );
   selectivity->setChecked( selectivityDefault );
   electrostatics->setEnabled( 1 );
   electrostatics->setChecked( electrostaticsDefault );

   adjustTable();
}

