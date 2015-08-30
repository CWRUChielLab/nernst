/* main.cpp
 *
 *
 * Copyright (c) 2015, Jeffrey Gill, Barry Rountree, Kendrick Shaw, 
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


#include <QApplication>

#include "options.h"
#include "sim.h"
#include "gui.h"


int
main( int argc, char *argv[] )
{
   QCoreApplication *app;

   struct options *o;
   o = parseOptions( argc, argv );

   if( o->use_gui )
   {
      app = new QApplication( argc, argv );
      NernstGUI gui( o );
      gui.show();
      return app->exec();
   } else {
      app = new QCoreApplication( argc, argv );
      NernstSim sim( o );
      sim.runSim();
      return 0;
   }
}

