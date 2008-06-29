/* main.cpp
 *
 */

// THIS IS A TEST

#include <QApplication>
// #include <QDesktopWidget>

#include "options.h"
#include "sim.h"


int
main( int argc, char *argv[] )
{
   QApplication app( argc, argv );

   struct options *o;
   o = parseOptions( argc, argv );
   
   NernstSim sim( o );

   if( o->use_gui )
   {
      // sim.setFixedSize( 831, 574 );

      /*
      // Center the GUI on the screen
      QRect desktop( QApplication::desktop()->screenGeometry() );
      int x = desktop.width() / 2 - sim.width() / 2;
      int y = desktop.height() / 2 - sim.height() / 2;
      sim.move( x, y );
      */

      sim.show();
      return app.exec();
   } else {
      sim.startSim();
      return 0;
   }
}

