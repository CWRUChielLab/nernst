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
 
   public slots:
      void runSim();
      void pauseSim();
      void unpauseSim();
      void resetSim();

   signals:
      void moveCompleted( int currentIter );
      void updateStatus( QString msg );
      void finished();

   protected:
 
   private:
      NernstGUI *gui;
      // struct options *o;
      // long maxatomsDefault;
      int paused;
      int resetting;
      // double elapsed;
      // int currentIter;
      // QTime *qtime;
      void initNernstSim();
      int preIter();
      void Iter();
      void postIter();
      void completeNernstSim();
};

#endif /* SIM_H */
