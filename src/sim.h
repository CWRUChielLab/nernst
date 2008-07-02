/* sim.h
 *
 */

#ifndef SIM_H
#define SIM_H 

#include <QWidget>


class NernstSim : public QWidget
{
   Q_OBJECT

   public:
      NernstSim( struct options *options, QWidget *parent = 0 );
 
   public slots:
      void changeIters( int iters );
      void changePores( int pores );
      void changeLspacing( int lspacing );
      void changeRspacing( int rspacing );
      void changeSelectivity( bool selectivity );
      void changeElectrostatics( bool electrostatics );
      void changeSeed( QString seed );
      void startSim();
      void pauseSim();
      void unpauseSim();
      void resetSim();
      void closeEvent( QCloseEvent *event );

   signals:
      void moveCompleted();
      void finished();
 
   private:
      struct options *o;
      long maxatomsDefault;
      int paused;
      int resetting;
      int quitting;
      double elapsed;
      int currentIter;
#ifdef BLR_USELINUX
      struct timeval tv_start, tv_stop;
#else
      time_t start, stop;
#endif
};

#endif /* SIM_H */
