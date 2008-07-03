/* gui.h
 *
 */

#ifndef GUI_H
#define GUI_H 

#include <QMainWindow>


class CtrlWidget;
class PaintWidget;


class NernstGUI : public QMainWindow
{
   Q_OBJECT

   public:
      NernstGUI( struct options *o, QWidget *parent = 0, Qt::WindowFlags flags = 0 );
 
   public slots:
      void setStatusMsg( QString msg );
      void closeEvent( QCloseEvent *event );

   signals:
      void itersChanged( int iters );
      void poresChanged( int pores );
      void lspacingChanged( int lspacing );
      void rspacingChanged( int rspacing );
      void selectivityChanged( bool selectivity );
      void electrostaticsChanged( bool electrostatics );
      void seedChanged( QString seed );
      void startBtnClicked();
      void pauseBtnClicked();
      void continueBtnClicked();
      void resetBtnClicked();
      void repaintWorld();
      void finished();
 
   private:
      CtrlWidget *ctrl;
      PaintWidget *canvas;
};

#endif /* GUI_H */
