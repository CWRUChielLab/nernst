/* xsim.h
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
