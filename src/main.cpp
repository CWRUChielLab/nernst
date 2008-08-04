/* main.cpp
 *
 */


#include <QApplication>

#include "options.h"
#include "sim.h"
#include "xsim.h"


int
main( int argc, char *argv[] )
{
   QCoreApplication *app;

   struct options *o;
   o = parseOptions( argc, argv );

   if( o->use_gui )
   {
      app = new QApplication( argc, argv );
      XNernstSim xsim( o );
      return app->exec();
   } else {
      app = new QCoreApplication( argc, argv );
      NernstSim sim( o );
      sim.runSim();
      return 0;
   }
}

