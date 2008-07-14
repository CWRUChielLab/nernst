/* sim.h
 *
 */

#ifndef SIM_H
#define SIM_H 

#include <QWidget>
#include <QTime>

class NernstGUI;


class NernstSim : public QWidget
{
   Q_OBJECT

   public:
      NernstSim( struct options *options, QWidget *parent = 0 );
 
   public slots:
      void runSim();
      void pauseSim();
      void resetSim();

   signals:
      void moveCompleted( int currentIter );
      void updateStatus( QString msg );
      void finished();
 
   private:
      NernstGUI *gui;
      struct options *o;
      long maxatomsDefault;
      int paused;
      int resetting;
      double elapsed;
      int currentIter;
      QTime *qtime;
};

#endif /* SIM_H */
