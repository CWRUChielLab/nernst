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
class QGroupBox;
class QFrame;
class QLabel;
class QProgressBar;
class QwtPlot;
class QwtPlotCurve;


extern double x_iters[], y_volts[], y_ghk[]; // y_gibbs[], y_boltzmann[];


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
      void saveWorld();
      void loadWorld();

      void enableSaveInit();
      void disableSaveInit();
      void enableLoadInit();
      void disableLoadInit();
      void enableSaveWorld();
      void disableSaveWorld();
      void enableLoadWorld();
      void disableLoadWorld();

      void calcEquilibrium();
      void updatePlots( int currentIter );
      void resetPlots();
      void adjustTable();
      void updateTable();
      void fixRedraw();
      void shrinkWindow();

   signals:
      void settingsLoaded();
      void worldLoaded( int iter );
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

      QGroupBox *plotBox;
      QVBoxLayout *plotLayout;
      QLabel *curveLbl;

      QLabel *inLbl;
      QLabel *outLbl;
      QLabel *KLbl;
      QLabel *NaLbl;
      QLabel *ClLbl;
      QLabel *ImpChargeLbl;
      QLabel *ImpPartLbl;
      QLabel *KInLbl;
      QLabel *KOutLbl;
      QLabel *NaInLbl;
      QLabel *NaOutLbl;
      QLabel *ClInLbl;
      QLabel *ClOutLbl;
      QLabel *ImpChargeInLbl;
      QLabel *ImpChargeOutLbl;
      QLabel *ImpPartInLbl;
      QLabel *ImpPartOutLbl;

      QGridLayout *mainLayout;
      QGroupBox *concBox;
      QGridLayout *concLayout;
      QWidget *mainWidget;
      NernstStatusBar *statusBar;

      QMenu *fileMenu;
      QMenu *helpMenu;
      QAction *saveInitAct;
      QAction *loadInitAct;
      QAction *saveWorldAct;
      QAction *loadWorldAct;
      QAction *quitAct;
      QAction *aboutAct;
      QAction *aboutQtAct;

      QwtPlot *voltsPlot;
      QwtPlotCurve **curves;
      int numCurves;
      int currentNernstCurve;
      // double voltsNernst;
      double voltsGHK;
      // double voltsGibbs;
      // double voltsBoltzmann;

      double gibbsEnergy( int q );
      double boltzmannProbDiff( int q );
};

#endif /* GUI_H */
