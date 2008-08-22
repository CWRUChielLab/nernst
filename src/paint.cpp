/* paint.cpp
 *
 * GUI visualization
 */


#include <QApplication>
#include <QMouseEvent>
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
   shufflePositions( o );
 
   setFormat( QGLFormat( QGL::DoubleBuffer | QGL::DepthBuffer ) );
   rotationX = 0.0;
   rotationY = 0.0;
   rotationZ = 0.0;

   setFixedSize( o->x, o->y );
}


void
NernstPainter::mousePressEvent( QMouseEvent *event )
{
   int x, y;
   x = event->x();
   y = o->y - 1 - event->y();

   if( !running )
   {
      event->ignore();
      return;
   } else {
      event->accept();
   }

   switch( world[ idx( x, y ) ].color )
   {
      case ATOM_K:
         world[ idx( x, y ) ].color = ATOM_K_TRACK;
         break;
      case ATOM_Na:
         world[ idx( x, y ) ].color = ATOM_Na_TRACK;
         break;
      case ATOM_Cl:
         world[ idx( x, y ) ].color = ATOM_Cl_TRACK;
         break;
      case ATOM_K_TRACK:
         //world[ idx( x, y ) ].color = ATOM_K;
         //break;
      case ATOM_Na_TRACK:
         //world[ idx( x, y ) ].color = ATOM_Na;
         //break;
      case ATOM_Cl_TRACK:
         //world[ idx( x, y ) ].color = ATOM_Cl;
         //break;
      default:
         event->ignore();
         break;
   }

   update();
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
            int tracked = 0;

            // Nontracked atoms
            switch( world[ idx( x, y ) ].color )
            {
               case SOLVENT:
                  continue;
               case ATOM_K:
                  glColor3f( 1.f, 0.15f, 0.f );    // Red
                  break;
               case ATOM_Na:
                  glColor3f( 0.f, 0.f, 1.f );      // Blue
                  break;
               case ATOM_Cl:
                  glColor3f( 0.f, 0.70f, 0.35f );  // Green
                  break;
               case ATOM_K_TRACK:
                  glColor3f( 1.f, 0.15f, 0.f );    // Red
                  tracked = 1;
                  break;
               case ATOM_Na_TRACK:
                  glColor3f( 0.f, 0.f, 1.f );      // Blue
                  tracked = 1;
                  break;
               case ATOM_Cl_TRACK:
                  glColor3f( 0.f, 0.70f, 0.35f );  // Green
                  tracked = 1;
                  break;
               case PORE_K:
                  glColor3f( 1.f, 0.15f, 0.f );    // Red
                  break;
               case PORE_Na:
                  glColor3f( 0.f, 0.f, 1.f );      // Blue
                  break;
               case PORE_Cl:
                  glColor3f( 0.f, 0.70f, 0.35f );  // Green
                  break;
               case MEMBRANE:
                  glColor3f( 0.f, 0.f, 0.f );      // Black
                  break;
               default:
                  glColor3f( 1.f, 1.f, 1.f );      // White
                  break;
            }

            if( tracked )
            {
               glVertex3f( (GLfloat)( x - 2 ) / (GLfloat)o->x, (GLfloat)( y - 1 ) / (GLfloat)o->y, (GLfloat)0.0 );
               glVertex3f( (GLfloat)( x - 2 ) / (GLfloat)o->x, (GLfloat)( y     ) / (GLfloat)o->y, (GLfloat)0.0 );
               glVertex3f( (GLfloat)( x - 2 ) / (GLfloat)o->x, (GLfloat)( y + 1 ) / (GLfloat)o->y, (GLfloat)0.0 );

               glVertex3f( (GLfloat)( x - 1 ) / (GLfloat)o->x, (GLfloat)( y - 2 ) / (GLfloat)o->y, (GLfloat)0.0 );
               glVertex3f( (GLfloat)( x - 1 ) / (GLfloat)o->x, (GLfloat)( y - 1 ) / (GLfloat)o->y, (GLfloat)0.0 );
               glVertex3f( (GLfloat)( x - 1 ) / (GLfloat)o->x, (GLfloat)( y     ) / (GLfloat)o->y, (GLfloat)0.0 );
               glVertex3f( (GLfloat)( x - 1 ) / (GLfloat)o->x, (GLfloat)( y + 1 ) / (GLfloat)o->y, (GLfloat)0.0 );
               glVertex3f( (GLfloat)( x - 1 ) / (GLfloat)o->x, (GLfloat)( y + 2 ) / (GLfloat)o->y, (GLfloat)0.0 );

               glVertex3f( (GLfloat)( x     ) / (GLfloat)o->x, (GLfloat)( y - 2 ) / (GLfloat)o->y, (GLfloat)0.0 );
               glVertex3f( (GLfloat)( x     ) / (GLfloat)o->x, (GLfloat)( y - 1 ) / (GLfloat)o->y, (GLfloat)0.0 );
               glVertex3f( (GLfloat)( x     ) / (GLfloat)o->x, (GLfloat)( y     ) / (GLfloat)o->y, (GLfloat)0.0 );
               glVertex3f( (GLfloat)( x     ) / (GLfloat)o->x, (GLfloat)( y + 1 ) / (GLfloat)o->y, (GLfloat)0.0 );
               glVertex3f( (GLfloat)( x     ) / (GLfloat)o->x, (GLfloat)( y + 2 ) / (GLfloat)o->y, (GLfloat)0.0 );

               glVertex3f( (GLfloat)( x + 1 ) / (GLfloat)o->x, (GLfloat)( y - 2 ) / (GLfloat)o->y, (GLfloat)0.0 );
               glVertex3f( (GLfloat)( x + 1 ) / (GLfloat)o->x, (GLfloat)( y - 1 ) / (GLfloat)o->y, (GLfloat)0.0 );
               glVertex3f( (GLfloat)( x + 1 ) / (GLfloat)o->x, (GLfloat)( y     ) / (GLfloat)o->y, (GLfloat)0.0 );
               glVertex3f( (GLfloat)( x + 1 ) / (GLfloat)o->x, (GLfloat)( y + 1 ) / (GLfloat)o->y, (GLfloat)0.0 );
               glVertex3f( (GLfloat)( x + 1 ) / (GLfloat)o->x, (GLfloat)( y + 2 ) / (GLfloat)o->y, (GLfloat)0.0 );
                  
               glVertex3f( (GLfloat)( x + 2 ) / (GLfloat)o->x, (GLfloat)( y - 1 ) / (GLfloat)o->y, (GLfloat)0.0 );
               glVertex3f( (GLfloat)( x + 2 ) / (GLfloat)o->x, (GLfloat)( y     ) / (GLfloat)o->y, (GLfloat)0.0 );
               glVertex3f( (GLfloat)( x + 2 ) / (GLfloat)o->x, (GLfloat)( y + 1 ) / (GLfloat)o->y, (GLfloat)0.0 );
            } else {
               glVertex3f( (GLfloat)( x ) / (GLfloat)o->x, (GLfloat)( y ) / (GLfloat)o->y, (GLfloat)0.0 );
            }
         }
      }
      glEnd();

   } else {
      // World preview visualization
      glBegin( GL_POINTS );
      int numK, numNa, numCl;
      unsigned int *posK, *posNa, *posCl;
      int placed = 0, x, y, i;

      numK  = (int)( (double)( 1.0 ) * (double)( o->y / 2 ) / 3.0 * (double)( o->pK  ) + 0.5 );
      numNa = (int)( (double)( 1.0 ) * (double)( o->y / 2 ) / 3.0 * (double)( o->pNa ) + 0.5 );
      numCl = (int)( (double)( 1.0 ) * (double)( o->y / 2 ) / 3.0 * (double)( o->pCl ) + 0.5 );

      posK  = positionsPORES;
      posNa = positionsPORES + (int)( (double)( ( 1 ) * ( o->y / 2 ) ) * 1.0 / 3.0 );
      posCl = positionsPORES + (int)( (double)( ( 1 ) * ( o->y / 2 ) ) * 2.0 / 3.0 );

      for( y = 0; y < o->y; y++ )
      {
         // Left membrane
         glColor3f( 0.f, 0.f, 0.f );   // Black
         glVertex3f( (GLfloat)0.0, (GLfloat)y / (GLfloat)o->y, (GLfloat)0.0 );

         // Central membrane with pores
         glColor3f( 0.f, 0.f, 0.f );   // Black
         for( i = 0; i < numK; i++ )
         {
            if( y == (int)posK[ i ] )
            {
               glColor3f( 1.f, 0.15f, 0.f );    // Red
            }
         }
         for( i = 0; i < numNa; i++ )
         {
            if( y == (int)posNa[ i ] )
            {
               glColor3f( 0.f, 0.f, 1.f );      // Blue
            }
         }
         for( i = 0; i < numCl; i++ )
         {
            if( y == (int)posCl[ i ] )
            {
               glColor3f( 0.f, 0.70f, 0.35f );  // Green
            }
         }
         glVertex3f( (GLfloat)0.5, (GLfloat)y / (GLfloat)o->y, (GLfloat)0.0 );

         // Right membrane
         glColor3f( 0.f, 0.f, 0.f );   // Black
         glVertex3f( (GLfloat)( o->x - 1 ) / (GLfloat)o->x, (GLfloat)y / (GLfloat)o->y, (GLfloat)0.0 );
      }

      // Draw LHS atoms.
      numK  = (int)( (double)( o->x / 2 - 1 ) * (double)( o->y ) / 3.0 * (double)( o->lK  ) / (double)MAX_CONC + 0.5 );
      numNa = (int)( (double)( o->x / 2 - 1 ) * (double)( o->y ) / 3.0 * (double)( o->lNa ) / (double)MAX_CONC + 0.5 );
      numCl = (int)( (double)( o->x / 2 - 1 ) * (double)( o->y ) / 3.0 * (double)( o->lCl ) / (double)MAX_CONC + 0.5 );

      posK  = positionsLHS;
      posNa = positionsLHS + (int)( (double)( ( o->x / 2 - 1 ) * ( o->y ) ) * 1.0 / 3.0 );
      posCl = positionsLHS + (int)( (double)( ( o->x / 2 - 1 ) * ( o->y ) ) * 2.0 / 3.0 );

      for( i = 0; i < numK && placed < o->max_atoms; i++ )
      {
         x = ( posK[ i ] % ( o->x / 2 - 1 ) ) + 1;
         y =   posK[ i ] / ( o->x / 2 - 1 );

         glColor3f( 1.f, 0.15f, 0.f );    // Red
         glVertex3f( (GLfloat)x / (GLfloat)o->x, (GLfloat)y / (GLfloat)o->y, (GLfloat)0.0 );
         placed++;
      }

      for( i = 0; i < numNa && placed < o->max_atoms; i++ )
      {
         x = ( posNa[ i ] % ( o->x / 2 - 1 ) ) + 1;
         y =   posNa[ i ] / ( o->x / 2 - 1 );

         glColor3f( 0.f, 0.f, 1.f );      // Blue
         glVertex3f( (GLfloat)x / (GLfloat)o->x, (GLfloat)y / (GLfloat)o->y, (GLfloat)0.0 );
         placed++;
      }

      for( i = 0; i < numCl && placed < o->max_atoms; i++ )
      {
         x = ( posCl[ i ] % ( o->x / 2 - 1 ) ) + 1;
         y =   posCl[ i ] / ( o->x / 2 - 1 );

         glColor3f( 0.f, 0.70f, 0.35f );  // Green
         glVertex3f( (GLfloat)x / (GLfloat)o->x, (GLfloat)y / (GLfloat)o->y, (GLfloat)0.0 );
         placed++;
      }

      // Draw RHS atoms.
      numK  = (int)( (double)( o->x / 2 - 2 ) * (double)( o->y ) / 3.0 * (double)( o->rK  ) / (double)MAX_CONC + 0.5 );
      numNa = (int)( (double)( o->x / 2 - 2 ) * (double)( o->y ) / 3.0 * (double)( o->rNa ) / (double)MAX_CONC + 0.5 );
      numCl = (int)( (double)( o->x / 2 - 2 ) * (double)( o->y ) / 3.0 * (double)( o->rCl ) / (double)MAX_CONC + 0.5 );

      posK  = positionsRHS;
      posNa = positionsRHS + (int)( (double)( ( o->x / 2 - 2 ) * ( o->y ) ) * 1.0 / 3.0 );
      posCl = positionsRHS + (int)( (double)( ( o->x / 2 - 2 ) * ( o->y ) ) * 2.0 / 3.0 );

      for( i = 0; i < numK && placed < o->max_atoms; i++ )
      {
         x = ( posK[ i ] % ( o->x / 2 - 2 ) ) + o->x / 2 + 1;
         y =   posK[ i ] / ( o->x / 2 - 2 );

         glColor3f( 1.f, 0.15f, 0.f );    // Red
         glVertex3f( (GLfloat)x / (GLfloat)o->x, (GLfloat)y / (GLfloat)o->y, (GLfloat)0.0 );
         placed++;
      }

      for( i = 0; i < numNa && placed < o->max_atoms; i++ )
      {
         x = ( posNa[ i ] % ( o->x / 2 - 2 ) ) + o->x / 2 + 1;
         y =   posNa[ i ] / ( o->x / 2 - 2 );

         glColor3f( 0.f, 0.f, 1.f );      // Blue
         glVertex3f( (GLfloat)x / (GLfloat)o->x, (GLfloat)y / (GLfloat)o->y, (GLfloat)0.0 );
         placed++;
      }

      for( i = 0; i < numCl && placed < o->max_atoms; i++ )
      {
         x = ( posCl[ i ] % ( o->x / 2 - 2 ) ) + o->x / 2 + 1;
         y =   posCl[ i ] / ( o->x / 2 - 2 );

         glColor3f( 0.f, 0.70f, 0.35f );  // Green
         glVertex3f( (GLfloat)x / (GLfloat)o->x, (GLfloat)y / (GLfloat)o->y, (GLfloat)0.0 );
         placed++;
      }

      glEnd();

      if( cleanRedraw )
      {
         cleanRedraw = 0;
         emit previewRedrawn();
      }
   }
}

