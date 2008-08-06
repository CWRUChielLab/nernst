/* gui.h
 *
 */

#ifndef GUI_H
#define GUI_H 

#include <QMainWindow>


class XNernstSim;
class NernstCtrl;
class NernstPainter;
class QVBoxLayout;
class QGridLayout;
class QFrame;
class QwtPlot;
class QwtPlotCurve;


class NernstGUI : public QMainWindow
{
   Q_OBJECT

   public:
      NernstGUI( struct options *options, QWidget *parent = 0, Qt::WindowFlags flags = 0 );

   protected:
      void closeEvent( QCloseEvent *event );
 
   public slots:
      void about();
      void setStatusMsg( QString msg );
      void updatePlots( int currentIter );
      void resetPlots();
      void fixRedraw();
      void shrinkWindow();

   signals:
      void repaintWorld();
      void finished();
 
   private:
      struct options *o;

      XNernstSim *sim;

      NernstCtrl *ctrl;
      QFrame *ctrlFrame;
      QVBoxLayout *ctrlLayout;

      NernstPainter *canvas;
      QFrame *canvasFrame;
      QVBoxLayout *canvasLayout;

      QGridLayout *mainLayout;
      QWidget *mainWidget;

      QMenu *fileMenu;
      QMenu *helpMenu;
      QAction *quitAct;
      QAction *aboutAct;

      QwtPlot *voltsPlot;
      QwtPlotCurve *voltsCurve;
      QwtPlotCurve *nernstCurve;
};

#endif /* GUI_H */
