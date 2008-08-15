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
      void changeLspacing( int lspacing );
      void changeRspacing( int rspacing );
      void changeSelectivity( bool selectivity );
      void changeElectrostatics( bool electrostatics );
      void reloadSettings();

      void disableCtrl();
      void reenableCtrl();
      void resetCtrl();

   signals:
      void startBtnClicked();
      void pauseBtnClicked();
      void continueBtnClicked();
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
      int lspacingDefault;
      int rspacingDefault;
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

      QLabel *lspacingLbl;
      QSlider *lspacingSld;
      QLabel *lspacingVal;

      QLabel *rspacingLbl;
      QSlider *rspacingSld;
      QLabel *rspacingVal;

      QCheckBox *selectivity;
      QCheckBox *electrostatics;

      QPushButton *startBtn;
      QPushButton *pauseBtn;
      QPushButton *continueBtn;
      QPushButton *resetBtn;
      QPushButton *quitBtn;

      QVBoxLayout *mainLayout;
      QGridLayout *ctrlLayout;
      QGroupBox *spacingBox;
      QGridLayout *spacingLayout;
      QStackedLayout *stackedBtnLayout;
};

#endif /* CTRL_H */
