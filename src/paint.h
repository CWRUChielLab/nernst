/* paint.h
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


#ifndef PAINT_H
#define PAINT_H 


#include <QGLWidget>
#include <QOpenGLFunctions_1_1>


class NernstPainter : public QGLWidget, public QOpenGLFunctions_1_1
{
   Q_OBJECT

   public:
      NernstPainter( struct options *o, int zoomOn = 0, QWidget *parent = 0 );

   public slots:
      void adjustPaintRegion();
      void zoomIn();
      void zoomOut();
      void startPaint();
      void resetPaint();

   signals:
      void ionMarked();

   protected:
      void mousePressEvent( QMouseEvent *event );
      void initializeGL();
      void resizeGL( int width, int height );
      void paintGL();

   private:
      struct options *o;
      int running;
      int zoom;
      int zoomXRange;
      int zoomYRange;
      int zoomXWindow;
      int zoomYWindow;
      int minX;
      int minY;
      int maxX;
      int maxY;

      GLfloat rotationX;
      GLfloat rotationY;
      GLfloat rotationZ;
      // QPoint lastPos;
      void draw();
};

#endif /* PAINT_H */

