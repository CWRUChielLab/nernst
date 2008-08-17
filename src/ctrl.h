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
      void changePores( int pores );
      void changeSeed( QString seed );
      void changeLconc( int lconc );
      void changeRconc( int rconc );
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
      int poresDefault;
      int lconcDefault;
      int rconcDefault;
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

      QLabel *poresLbl;
      QSlider *poresSld;
      QLabel *poresVal;

      QLabel *seedLbl;
      QLineEdit *seedVal;

      QLabel *lconcLbl;
      QSlider *lconcSld;
      QLabel *lconcVal;

      QLabel *rconcLbl;
      QSlider *rconcSld;
      QLabel *rconcVal;

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
      QGroupBox *concBox;
      QGridLayout *concLayout;
      QStackedLayout *stackedBtnLayout;
};

#endif /* CTRL_H */
