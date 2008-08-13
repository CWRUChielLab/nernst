/* sim.h
 *
 */

#ifndef SIM_H
#define SIM_H 

#include <QTime>


class NernstSim 
{
   public:
      NernstSim( struct options *options );
      void runSim();
      int getCurrentIter();

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
};

#endif /* SIM_H */
