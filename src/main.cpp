/* main.cpp
 *
 */


#include <QApplication>
#include <QCoreApplication>

#include "options.h"
#include "sim.h"
#include "xsim.h"


int
main( int argc, char *argv[] )
{
   struct options *o = NULL;
   QCoreApplication *app = NULL;

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

