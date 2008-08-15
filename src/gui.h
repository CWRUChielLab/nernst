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
class QLabel;
class QProgressBar;
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
      void aboutSFMT();
      void saveInit();
      void loadInit();
      void updateProgress( int currentIter );
      void recalcProgress();
      void resetProgress();
      void recalcNernst();
      void updatePlots( int currentIter );
      void resetPlots();
      void updateVoltsStatus( int currentIter, int avg );
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
      QProgressBar *progressBar;
      QLabel *voltsLbl;

      QMenu *fileMenu;
      QMenu *helpMenu;
      QAction *saveInitAct;
      QAction *loadInitAct;
      QAction *quitAct;
      QAction *aboutAct;
      QAction *aboutSFMTAct;
      QAction *aboutQtAct;

      QwtPlot *voltsPlot;
      QwtPlotCurve *voltsCurve;
      QwtPlotCurve *nernstCurve;
};

#endif /* GUI_H */
