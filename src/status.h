/* status.h
 *
 *
 * Copyright (c) 2015, Jeffrey Gill, Barry Rountree, Kendrick Shaw, 
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

#ifndef STATUS_H
#define STATUS_H 

#include <QStatusBar>


class QLabel;
class QProgressBar;


class NernstStatusBar : public QStatusBar
{
   Q_OBJECT

   public:
      NernstStatusBar( struct options *options, QWidget *parent = 0 );

   protected:
      void mousePressEvent( QMouseEvent *event );
 
   public slots:
      void hideOrShow( QString msg );
      void setStatusLbl( QString msg );
      void updateProgressBar( int currentIter );
      void setVoltsLbl( int currentIter, int avg );

      void recalcProgress();
      void resetProgress();

   signals:
 
   private:
      struct options *o;
      int mode;

      QLabel *statusLbl;
      QProgressBar *progressBar;
      QLabel *voltsLbl;
};

#endif /* STATUS_H */
