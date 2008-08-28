/* xsim.h
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
 *
 *
 * Additional GUI components for simulation interface.
 * Note that this inherits from both NernstSim and QWidget.
 */

#ifndef XSIM_H
#define XSIM_H 

#include <QWidget>
#include <QTime>
#include "sim.h"

class NernstGUI;


class XNernstSim : public QWidget, public NernstSim
{
   Q_OBJECT

   public:
      XNernstSim( struct options *options, QWidget *parent = 0 );
      int getCurrentIter();
 
   public slots:
      void loadWorld( int iter );
      void runSim();
      void pauseSim();
      void unpauseSim();
      void resetSim();
      void quitSim();

   signals:
      void calcEquilibrium();
      void moveCompleted( int currentIter );
      void updateStatus( QString msg );
      void updateVoltsStatus( int currentIter, int avg );
      void finished();

   protected:
 
   private:
      int initialized;
      int paused;
      int resetting;
      int quitting;

      void initNernstSim();
      int preIter();
      void Iter();
      void postIter();
      void completeNernstSim();
};

#endif /* SIM_H */
