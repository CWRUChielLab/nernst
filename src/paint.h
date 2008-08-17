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
      NernstPainter( struct options *o, QWidget *parent = 0 );

   public slots:
      void cleanUpdate();
      void startPaint();
      void resetPaint();

   signals:
      void previewRedrawn();

   protected:
      void mousePressEvent( QMouseEvent *event );
      void initializeGL();
      void resizeGL( int width, int height );
      void paintGL();

   private:
      struct options *o;
      int running;
      int cleanRedraw;

      GLfloat rotationX;
      GLfloat rotationY;
      GLfloat rotationZ;
      // QPoint lastPos;
      void draw();
};

#endif /* PAINT_H */

