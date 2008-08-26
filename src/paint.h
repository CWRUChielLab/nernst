/* paint.h
 *
 */


#ifndef PAINT_H
#define PAINT_H 


#include <QGLWidget>


class NernstPainter : public QGLWidget
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

