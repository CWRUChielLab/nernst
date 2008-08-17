/* paint.cpp
 *
 * GUI visualization
 */


#include <QApplication>
#include <assert.h>
#include <SFMT.h>

#include "paint.h"
#include "options.h"
#include "atom.h"
#include "world.h"
#include "sim.h"


NernstPainter::NernstPainter( struct options *options, QWidget *parent ) 
	: QGLWidget( parent )
{
   o = options;
   running = 0;
   cleanRedraw = 0;
   randomizePositions( o );
 
   setFormat( QGLFormat( QGL::DoubleBuffer | QGL::DepthBuffer ) );
   rotationX = 0.0;
   rotationY = 0.0;
   rotationZ = 0.0;

   setFixedSize( o->x, o->y );
}


void
NernstPainter::cleanUpdate()
{
   cleanRedraw = 1;
   update();
}


void
NernstPainter::startPaint()
{
   running = 1;
   update();
}


void
NernstPainter::resetPaint()
{
   running = 0;
   update();
}


void
NernstPainter::initializeGL()
{
   qglClearColor( Qt::white );
   glShadeModel( GL_FLAT );
   glEnable( GL_DEPTH_TEST );
   glEnable( GL_CULL_FACE );
}


void
NernstPainter::resizeGL( int width, int height )
{
   glViewport( 0, 0, width, height );
   glMatrixMode( GL_PROJECTION );
   glLoadIdentity();
   // GLfloat x = (GLfloat)(width) / height;
   // glFrustum( -x, x, -1.0, 1.0, 4.0, 15.0 );
   glFrustum( 0, 0.5, 0.0, 0.5, 4.0, 15.0 );
   glMatrixMode( GL_MODELVIEW );
}


void
NernstPainter::paintGL()
{
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
   draw();
}


void
NernstPainter::draw()
{
   setFixedSize( o->x, o->y );

   /*
   static const GLfloat P1[ 3 ] = { 0.0, -1.0, +2.0 };
   static const GLfloat P2[ 3 ] = { +1.73205081, -1.0, -1.0 };
   static const GLfloat P3[ 3 ] = { -1.73205081, -1.0, -1.0 };
   static const GLfloat P4[ 3 ] = { 0.0, +2.0, 0.0 };

   static const GLfloat * const coords[ 4 ][ 3 ] =
   {
      { P1, P2, P3 }, { P1, P3, P4 }, { P1, P4, P2 }, { P2, P4, P3 }
   };
   */

   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity();
   glTranslatef( 0.0, 0.0, -8.0 );
   glRotatef( rotationX, 1.0, 0.0, 0.0 );
   glRotatef( rotationY, 0.0, 1.0, 0.0 );
   glRotatef( rotationZ, 0.0, 0.0, 1.0 );

   /*
   glLoadName( i );

   for( int i = 0; i < 4; ++i )
   {
      qglColor( faceColors[ i ] );
      for( int j = 0; j < 3; ++j )
      {
         glVertex3f( coords[ i ][ j ][ 0 ], coords[ i ][ j ][ 1 ], coords[ i ][ j ][ 2 ] );
      }
   }
   */

   // Realtime world visualization
   if( running )
   {
      glBegin( GL_POINTS );
      for( int y = 0; y < o->y; y++ )
      {
         for( int x = 0; x < o->x; x++ )
         {
            switch( world[ idx( x, y ) ].color )
            {
               case SOLVENT:
                  continue;
               case ATOM_K:
                  glColor3f( 1.f, 0.f, 0.f );
                  break;
               case ATOM_Cl:
                  glColor3f( 0.f, 0.f, 1.f );
                  break;
               case MEMBRANE:
                  glColor3f( 0.f, 0.f, 0.f );
                  break;
               default:
                  glColor3f( 0.f, 1.f, 0.f );
                  break;
            }
            glVertex3f( (GLfloat)x / (GLfloat)o->x, (GLfloat)y / (GLfloat)o->y, (GLfloat)0.0 );
         }
      }
      glEnd();

   } else {
      // World preview visualization
      glBegin( GL_POINTS );
      int numIons, placed = 0, x, y, i = 1, atomBit;

      for( y = 0; y < o->y; y++ )
      {
         glColor3f( 0.f, 0.f, 0.f );

         // Left membrane
         glVertex3f( (GLfloat)0.0, (GLfloat)y / (GLfloat)o->y, (GLfloat)0.0 );

         // Central membrane with pores
         if( y != (int)( ( (double)o->y / (double)( o->pores + 1 ) ) * (double)i ) )
         {
            glVertex3f( (GLfloat)0.5, (GLfloat)y / (GLfloat)o->y, (GLfloat)0.0 );
         } else {
            i++;
         }

         // Right membrane
         glVertex3f( (GLfloat)( o->x - 1 ) / (GLfloat)o->x, (GLfloat)y / (GLfloat)o->y, (GLfloat)0.0 );
      }

      // LHS
      atomBit = 1;
      numIons = (int)( (double)( o->x / 2 - 1 ) * (double)( o->y ) * (double)( o->lconc ) / (double)MAX_CONC + 0.5 );
      for( int i = 0; i < numIons && placed < o->max_atoms; i++ )
      {
         placed++;

         x = ( positionsLHS[ i ] % ( o->x / 2 - 1 ) ) + 1;
         y = positionsLHS[ i ] / ( o->x / 2 - 1 );
 
         if( atomBit )
         {
            // ATOM_K
            glColor3f( 1.f, 0.f, 0.f );
         } else {
            // ATOM_Cl
            glColor3f( 0.f, 0.f, 1.f );
         }
         atomBit = !atomBit;

         glVertex3f( (GLfloat)x / (GLfloat)o->x, (GLfloat)y / (GLfloat)o->y, (GLfloat)0.0 );
      }

      // RHS
      atomBit = 1;
      numIons = (int)( (double)( o->x / 2 - 2 ) * (double)( o->y ) * (double)( o->rconc ) / (double)MAX_CONC + 0.5 );
      for( int i = 0; i < numIons && placed < o->max_atoms; i++ )
      {
         placed++;

         x = ( positionsRHS[ i ] % ( o->x / 2 - 2 ) ) + ( o->x / 2 + 1 );
         y = positionsRHS[ i ] / ( o->x / 2 - 2 );

         if( atomBit )
         {
            glColor3f( 0.f, 0.f, 1.f );
         } else {
            glColor3f( 1.f, 0.f, 0.f );
         }
         atomBit = !atomBit;

         glVertex3f( (GLfloat)x / (GLfloat)o->x, (GLfloat)y / (GLfloat)o->y, (GLfloat)0.0 );
      }

      glEnd();

      if( cleanRedraw )
      {
         cleanRedraw = 0;
         emit previewRedrawn();
      }
   }
}

