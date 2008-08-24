/* gui.h
 *
 *
 * Copyright (c) 2008, Jeffery Gill, Barry Rountree, Kendrick Shaw, 
 *    Catherine Kehl, Jocelyn Eckert, and Dr. Hillel J. Chiel
 *
 * This file is part of Nernst Potential Simulator.
 * 
 * Nernst Potential Simulator is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * Nernst Potential Simulator is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nernst Potential Simulator.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef GUI_H
#define GUI_H 

#include <QMainWindow>


class XNernstSim;
class NernstCtrl;
class NernstPainter;
class NernstStatusBar;
class QScrollArea;
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

      QLabel *inCanvasLbl;
      QLabel *outCanvasLbl;
      NernstPainter *canvas;
      QGridLayout *canvasLayout;
      QScrollArea *canvasScroll;

      QFrame *plotFrame;
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
      QFrame *concFrame;
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
