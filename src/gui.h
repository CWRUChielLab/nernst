/* gui.h
 *
 */

#ifndef GUI_H
#define GUI_H 

#include <QMainWindow>


class NernstCtrl;
class NernstPainter;


class NernstGUI : public QMainWindow
{
   Q_OBJECT

   public:
      NernstGUI( struct options *o, QWidget *parent = 0, Qt::WindowFlags flags = 0 );

   protected:
      void closeEvent( QCloseEvent *event );
 
   public slots:
      void about();
      void setStatusMsg( QString msg );

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
      NernstCtrl *ctrl;
      NernstPainter *canvas;

      QMenu *fileMenu;
      QMenu *helpMenu;
      QAction *quitAct;
      QAction *aboutAct;
};

#endif /* GUI_H */
