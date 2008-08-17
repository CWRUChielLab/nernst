/* status.h
 *
 */

#ifndef STATUS_H
#define STATUS_H 

#include <QStatusBar>


class QLabel;
class QProgressBar;


class NernstStatusBar : public QStatusBar
{
   Q_OBJECT

   public:
      NernstStatusBar( struct options *options, QWidget *parent = 0 );

   protected:
      void mousePressEvent( QMouseEvent *event );
 
   public slots:
      void hideOrShow( QString msg );
      void setStatusLbl( QString msg );
      void updateProgressBar( int currentIter );
      void setVoltsLbl( int currentIter, int avg );

      void recalcProgress();
      void resetProgress();

   signals:
 
   private:
      struct options *o;
      int mode;

      QLabel *statusLbl;
      QProgressBar *progressBar;
      QLabel *voltsLbl;
};

#endif /* STATUS_H */
