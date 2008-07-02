/* gui.cpp
 *
 * The main application GUI.
 */


#include <QFrame>
#include <QVBoxLayout>
#include <QGridLayout>

#include "gui.h"
#include "options.h"
#include "ctrl.h"
#include "paint.h"


NernstGUI::NernstGUI( struct options *o, QWidget *parent )
   : QWidget( parent )
{
   // Initialization controls
   CtrlWidget *ctrl = new CtrlWidget( o );

   QFrame *ctrlFrame = new QFrame();
   ctrlFrame->setFrameStyle( QFrame::Box | QFrame::Sunken );

   QVBoxLayout *ctrlLayout = new QVBoxLayout();
   ctrlLayout->addWidget( ctrl );
   ctrlFrame->setLayout( ctrlLayout );

   // World visualization
   PaintWidget *canvas = new PaintWidget( o );

   QFrame *canvasFrame = new QFrame();
   canvasFrame->setFrameStyle( QFrame::Box | QFrame::Sunken );

   QVBoxLayout *canvasLayout = new QVBoxLayout();
   canvasLayout->addWidget( canvas );
   canvasFrame->setLayout( canvasLayout );

   // Main layout
   QGridLayout *mainLayout = new QGridLayout();
   mainLayout->addWidget( ctrlFrame, 0, 0 );
   mainLayout->addWidget( canvasFrame, 0, 1 );
   mainLayout->setColumnStretch( 1, 1 );
   setLayout( mainLayout );

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

   connect( ctrl, SIGNAL( quitBtnClicked() ), this, SIGNAL( quitBtnClicked() ) );

   connect( this, SIGNAL( repaint() ), canvas, SLOT( update() ) );

   connect( this, SIGNAL( finished() ), ctrl, SLOT( finish() ) );
}

