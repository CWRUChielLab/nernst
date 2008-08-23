/* ctrl.h
 *
 */

#ifndef CTRL_H
#define CTRL_H

#include <QWidget>


class QLabel;
class QLineEdit;
class QSlider;
class QCheckBox;
class QPushButton;
class QVBoxLayout;
class QGridLayout;
class QGroupBox;
class QStackedLayout;


class NernstCtrl : public QWidget
{
   Q_OBJECT

   public:
      NernstCtrl( struct options *o, QWidget *parent = 0 );

   public slots:
      void updateIter( int iter );
      void changeIters( int iters );
      void changeX( int xpow );
      void changeY( int ypow );
      void changeCapacitance( int cap );
      void changeSeed( QString seed );
      void changeLeftK( int lK );
      void changeLeftNa( int lNa );
      void changeLeftCl( int lCl );
      void changeRightK( int rK );
      void changeRightNa( int rNa );
      void changeRightCl( int rCl );
      void changePermK( int pK );
      void changePermNa( int pNa );
      void changePermCl( int pCl );
      void changeSelectivity( bool selectivity );
      void changeElectrostatics( bool electrostatics );
      void reloadSettings();

      void clearTrackedIons();
      void disableCtrl();
      void reenableCtrl();
      void resetCtrl();

   signals:
      void startBtnClicked();
      void pauseBtnClicked();
      void continueBtnClicked();
      void clearTrackingBtnClicked();
      void resetBtnClicked();
      void quitBtnClicked();

      void updatePreview();
      void worldShrunk();
 
   private:
      struct options *o;

      int currentIter;
      int itersDefault;
      int xDefault;
      int yDefault;
      double capDefault;
      int lKDefault;
      int lNaDefault;
      int lClDefault;
      int rKDefault;
      int rNaDefault;
      int rClDefault;
      double pKDefault;
      double pNaDefault;
      double pClDefault;
      int selectivityDefault;
      int electrostaticsDefault;

      QLabel *headerLbl;

      QLabel *itersLbl;
      QSlider *itersSld;
      QLabel *itersVal;

      QLabel *xLbl;
      QSlider *xSld;
      QLabel *xVal;

      QLabel *yLbl;
      QSlider *ySld;
      QLabel *yVal;

      QLabel *capLbl;
      QSlider *capSld;
      QLabel *capVal;

      QLabel *seedLbl;
      QLineEdit *seedVal;

      QLabel *inLbl;
      QLabel *outLbl;
      QLabel *permLbl;
      QLabel *KLbl;
      QLabel *NaLbl;
      QLabel *ClLbl;
      QSlider *lKSld;
      QLabel *lKLbl;
      QSlider *lNaSld;
      QLabel *lNaLbl;
      QSlider *lClSld;
      QLabel *lClLbl;
      QSlider *rKSld;
      QLabel *rKLbl;
      QSlider *rNaSld;
      QLabel *rNaLbl;
      QSlider *rClSld;
      QLabel *rClLbl;
      QSlider *pKSld;
      QLabel *pKLbl;
      QSlider *pNaSld;
      QLabel *pNaLbl;
      QSlider *pClSld;
      QLabel *pClLbl;

      QCheckBox *selectivity;
      QCheckBox *electrostatics;

      QPushButton *startBtn;
      QPushButton *pauseBtn;
      QPushButton *continueBtn;
      QPushButton *clearTrackingBtn;
      QPushButton *resetBtn;
      QPushButton *quitBtn;

      QVBoxLayout *mainLayout;
      QGridLayout *ctrlLayout;
      QGroupBox *sldBox;
      QGridLayout *sldLayout;
      QStackedLayout *stackedBtnLayout;
};

#endif /* CTRL_H */
