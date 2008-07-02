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


class CtrlWidget : public QWidget
{
   Q_OBJECT

   public:
      CtrlWidget( struct options *o, QWidget *parent = 0 );

   public slots:
      void roundIters( int value );
      void disableCtrl();
      void reenableCtrl();
      void resetCtrl();
      void finish();

   signals:
      void itersChanged( int iters );
      void poresChanged( int pores );
      void seedChanged( QString seed );
      void lspacingChanged( int lspacing );
      void rspacingChanged( int rspacing );
      void selectivityChanged( bool selectivity );
      void electrostaticsChanged( bool electrostatics );
      void startBtnClicked();
      void pauseBtnClicked();
      void continueBtnClicked();
      void resetBtnClicked();
      void quitBtnClicked();
 
   private:
      int itersDefault;
      int poresDefault;
      int lspacingDefault;
      int rspacingDefault;
      int selectivityDefault;
      int electrostaticsDefault;

      QLabel *headerLbl;

      QLabel *itersLbl;
      QSlider *itersSld;
      QLabel *itersVal;

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
      QStackedLayout *startPauseLayout;
};

#endif /* CTRL_H */
