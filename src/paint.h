/* paint.h
 *
 */


#ifndef PAINT_H
#define PAINT_H 


#include <QGLWidget>


class PaintWidget : public QGLWidget
{
   Q_OBJECT

   public:
      PaintWidget( struct options *o, QGLWidget *parent = 0 );

   public slots:
      void startPaint();
      void resetPaint();
      void changePores( int pores );
      void changeLspacing( int lspacing );
      void changeRspacing( int rspacing );

   signals:
      void finished();

   protected:
      void initializeGL();
      void resizeGL( int width, int height );
      void paintGL();

   private:
      int previewPores;
      int previewLspacing;
      int previewRspacing;
      int previewMaxatoms;
      int running;

      GLfloat rotationX;
      GLfloat rotationY;
      GLfloat rotationZ;
      // QPoint lastPos;
      void draw();
};

#endif /* PAINT_H */

