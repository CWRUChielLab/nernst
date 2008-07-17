/* main.cpp
 *
 */


#include <QApplication>
#include <QCoreApplication>

#include "options.h"
#include "sim.h"


int
main( int argc, char *argv[] )
{
   struct options *o=NULL;
   QCoreApplication *app=NULL;

   o = parseOptions( argc, argv );

   if( o->use_gui )
   {
      app = new QApplication( argc, argv );
   } else {
      app = new QCoreApplication( argc, argv );
   }

   NernstSim sim( o );
   return app->exec();
}

