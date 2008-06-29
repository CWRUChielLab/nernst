/* gui.h
 *
 */

#ifndef GUI_H
#define GUI_H 

#include <QWidget>


class CtrlWidget;
class PaintWidget;


class NernstGUI : public QWidget
{
   Q_OBJECT

   public:
      NernstGUI( struct options *o, QWidget *parent = 0 );
 
   public slots:

   signals:
      void itersChanged( QString iters );
      void poresChanged( QString pores );
      void lspacingChanged( int lspacing );
      void rspacingChanged( int rspacing );
      void selectivityChanged( bool selectivity );
      void electrostaticsChanged( bool electrostatics );
      void seedChanged( QString seed );
      void startBtnClicked();
      void pauseBtnClicked();
      void continueBtnClicked();
      void resetBtnClicked();
      void quitBtnClicked();
      void repaint();
      void finished();
 
   private:
      CtrlWidget *ctrl;
      PaintWidget *canvas;
};

#endif /* GUI_H */
