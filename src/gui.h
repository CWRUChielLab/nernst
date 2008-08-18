/* gui.h
 *
 */

#ifndef GUI_H
#define GUI_H 

#include <QMainWindow>


class XNernstSim;
class NernstCtrl;
class NernstPainter;
class NernstStatusBar;
class QVBoxLayout;
class QGridLayout;
class QFrame;
class QLabel;
class QProgressBar;
class QwtPlot;
class QwtPlotCurve;


extern double x_iters[], y_volts[], y_nernst[];


class NernstGUI : public QMainWindow
{
   Q_OBJECT

   public:
      NernstGUI( struct options *options, QWidget *parent = 0, Qt::WindowFlags flags = 0 );

   protected:
      void closeEvent( QCloseEvent *event );
 
   public slots:
      void about();
      void saveInit();
      void loadInit();
      void disableSaveLoad();
      void enableSaveLoad();
      void recalcNernst();
      void updatePlots( int currentIter );
      void resetPlots();
      void fixRedraw();
      void shrinkWindow();

   signals:
      void settingsLoaded();
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
      NernstStatusBar *statusBar;

      QMenu *fileMenu;
      QMenu *helpMenu;
      QAction *saveInitAct;
      QAction *loadInitAct;
      QAction *quitAct;
      QAction *aboutAct;
      QAction *aboutQtAct;

      QwtPlot *voltsPlot;
      QwtPlotCurve **curves;
      int numCurves;
      int currentNernstCurve;
};

#endif /* GUI_H */
