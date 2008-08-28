/* sim.h
 *
 *
 * Copyright (c) 2008, Jeffrey Gill, Barry Rountree, Kendrick Shaw, 
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

#ifndef SIM_H
#define SIM_H 

#include <QTime>

enum
{
   MIN_X = 16,
   MIN_Y = 16,
   MAX_X = 1024,
   MAX_Y = 1024,
   MIN_ITERS = 1,
   MAX_ITERS = 100000,
   MIN_CONC = 0,     // Minimum ion concentration (mM)
   MAX_CONC = 2000   // Maximum ion concentration (mM)
};


class NernstSim 
{
   public:
      NernstSim( struct options *options );
      void runSim();
      struct atom *world;
      unsigned long int direction_sz64;
      unsigned char *claimed;
      unsigned char *direction;


   protected:
      struct options *o;
      long maxatomsDefault;
      double elapsed;
      int currentIter;
      QTime *qtime;

      void initNernstSim();
      int preIter();
      void Iter();
      void postIter();
      void completeNernstSim();
   private:
      void initWorld( struct options *o );
};

#endif /* SIM_H */
