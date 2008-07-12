/* main.cpp
 *
 */


#include <QApplication>

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
      return app.exec();
   } else {
      sim.runSim();
      return 0;
   }
}

