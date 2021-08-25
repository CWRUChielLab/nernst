/* paint.cpp
 *
 * GUI visualization
 *
 * Copyright (c) 2021, Jeffrey Gill, Barry Rountree, Kendrick Shaw, 
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


#include <QApplication>
#include <QMouseEvent>
#include <assert.h>
#include <SFMT.h>


#include "paint.h"
#include "options.h"
#include "atom.h"
#include "world.h"
#include "sim.h"


NernstPainter::NernstPainter( struct options *options, int zoomOn, QWidget *parent ) 
	: QGLWidget( parent )
{
   o = options;
   running = 0;
   zoom = zoomOn;

   shufflePositions( o );
 
   setFormat( QGLFormat( QGL::DoubleBuffer | QGL::DepthBuffer ) );
   rotationX = 0.0;
   rotationY = 0.0;
   rotationZ = 0.0;

   if( zoom )
   {
      zoomXRange  = 64;    // lattice sqaures wide
      zoomYRange  = 64;    // lattice squares high
      zoomXWindow = 210;   // pixels wide
      zoomYWindow = 210;   // pixels high
   }

   adjustPaintRegion();
}


void
NernstPainter::adjustPaintRegion()
{
   if( !zoom )
   {
      zoomXRange  = o->x;
      zoomYRange  = o->y;
      zoomXWindow = o->x;
      zoomYWindow = o->y;
   }

   minX = o->x / 2 - zoomXRange / 2;
   maxX = o->x / 2 + zoomXRange / 2;
   minY = o->y / 2 - zoomYRange / 2;
   maxY = o->y / 2 + zoomYRange / 2;
   setFixedSize( zoomXWindow, zoomYWindow );
}


void
NernstPainter::zoomIn()
{
   if( zoom )
   {
      zoomXRange /= 2;
      zoomYRange /= 2;

      if( zoomXRange < 16 || zoomYRange < 16 )
      {
         zoomXRange = 16;
         zoomYRange = 16;
      }

      adjustPaintRegion();
      update();
   }
}


void
NernstPainter::zoomOut()
{
   if( zoom )
   {
      zoomXRange *= 2;
      zoomYRange *= 2;

      if( zoomXRange > o->x && zoomYRange > o->y )
      {
         zoomXRange /= 2;
         zoomYRange /= 2;
      }

      adjustPaintRegion();
      update();
   }
}


void
NernstPainter::mousePressEvent( QMouseEvent *event )
{
   int mouseX, mouseY, x, y;
   mouseX = event->x() * ( zoomXRange ) / ( zoomXWindow ) + minX;
   mouseY = ( zoomYWindow - 1 - event->y() ) * ( zoomYRange ) / ( zoomYWindow ) + minY;

   if( !running )
   {
      event->ignore();
      return;
   } else {
      event->accept();
   }

   x = mouseX;
   y = mouseY;

   if( !isUntrackedAtom( idx( x, y ) ) )
   {
      int offset = 1, done = 0;
      double offsetMax = 5.0 * (double)zoomXWindow / (double)zoomXRange;

      while( !done && offset < offsetMax )
      {
         for( x = mouseX - offset; x <= mouseX + offset && !done; x++ )
         {
            for( y = mouseY - offset; y <= mouseY + offset && !done; y++ )
            {
               if( isUntrackedAtom( idx( x, y ) ) )
               {
                  done = 1;
               }
            }
         }
         offset++;
      }
   }

   x--;
   y--;

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
      default:
         event->ignore();
         break;
   }

   emit ionMarked();
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
   zoomXRange = 64;
   zoomYRange = 64;
   update();
}


void
NernstPainter::initializeGL()
{
   initializeOpenGLFunctions();

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
   adjustPaintRegion();

   glMatrixMode( GL_MODELVIEW );
   glLoadIdentity();
   glTranslatef( 0.0, 0.0, -8.0 );
   glRotatef( rotationX, 1.0, 0.0, 0.0 );
   glRotatef( rotationY, 0.0, 1.0, 0.0 );
   glRotatef( rotationZ, 0.0, 0.0, 1.0 );

   double xWindowPerLatticeSquare = 1.0 / (double)zoomXRange;
   double yWindowPerLatticeSquare = 1.0 / (double)zoomYRange;

   double atomDiameterX        = 1.0 * xWindowPerLatticeSquare;      // in percentage of the window
   double atomDiameterY        = 1.0 * yWindowPerLatticeSquare;      // in percentage of the window
   double atomRadiusX          = atomDiameterX / 2.0;                // in percentage of the window
   double atomRadiusY          = atomDiameterY / 2.0;                // in percentage of the window

   double trackedAtomDiameterX = 5.0 * xWindowPerLatticeSquare;      // in percentage of the window
   double trackedAtomDiameterY = 5.0 * yWindowPerLatticeSquare;      // in percentage of the window
   double trackedAtomRadiusX   = trackedAtomDiameterX / 2.0;         // in percentage of the window
   double trackedAtomRadiusY   = trackedAtomDiameterY / 2.0;         // in percentage of the window

   // Realtime world visualization
   if( running )
   {
      glBegin( GL_POINTS );

      for( int y = minY; y < maxY; y++ )
      {
         for( int x = minX; x < maxX; x++ )
         {
            if( x >= 0 && x < o->x && y >= 0 && y < o->y )
            {
               int tracked = 0;

               switch( world[ idx( x, y ) ].color )
               {
                  case SOLVENT:
                     continue;
                  case ATOM_K:
                     if( o->electrostatics )
                     {
                        glColor3f( 1.f, 0.15f, 0.f );    // Red
                     } else {
                        glColor3f( 1.f, 0.64f, 0.57f );  // Pale red
                     }
                     break;
                  case ATOM_Na:
                     if( o->electrostatics )
                     {
                        glColor3f( 0.f, 0.f, 1.f );      // Blue
                     } else {
                        glColor3f( 0.57f, 0.57f, 1.f );  // Pale blue
                     }
                     break;
                  case ATOM_Cl:
                     if( o->electrostatics )
                     {
                        glColor3f( 0.f, 0.70f, 0.35f );  // Green
                     } else {
                        glColor3f( 0.57f, 0.87f, 0.72f );// Pale green
                     }
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
                     if( o->selectivity )
                     {
                        glColor3f( 1.f, 0.64f, 0.57f );  // Pale red
                     } else {
                        glColor3f( 1.f, 1.f, 1.f );      // White
                     }
                     break;
                  case PORE_Na:
                     if( o->selectivity )
                     {
                        glColor3f( 0.57f, 0.57f, 1.f );  // Pale blue
                     } else {
                        glColor3f( 1.f, 1.f, 1.f );      // White
                     }
                     break;
                  case PORE_Cl:
                     if( o->selectivity )
                     {
                        glColor3f( 0.57f, 0.87f, 0.72f );// Pale green
                     } else {
                        glColor3f( 1.f, 1.f, 1.f );      // White
                     }
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
                  // Tracked ions
                  for( double xOff = -trackedAtomRadiusX; xOff < trackedAtomRadiusX; xOff += 1.0 / (double)zoomXWindow )
                  {
                     for( double yOff = -trackedAtomRadiusY; yOff < trackedAtomRadiusY; yOff += 1.0 / (double)zoomYWindow )
                     {
                        glVertex3f( (GLfloat)( ( x - minX + 1 ) / (double)zoomXRange + xOff ),
                                    (GLfloat)( ( y - minY + 1 ) / (double)zoomYRange + yOff ),
                                    (GLfloat)0.0 );
                     }
                  }
               } else {
                  // Nontracked ions
                  for( double xOff = -atomRadiusX; xOff < atomRadiusX; xOff += 1.0 / (double)zoomXWindow )
                  {
                     for( double yOff = -atomRadiusY; yOff < atomRadiusY; yOff += 1.0 / (double)zoomYWindow )
                     {
                        glVertex3f( (GLfloat)( ( x + 1 - minX ) / (double)zoomXRange + xOff ),
                                    (GLfloat)( ( y + 1 - minY ) / (double)zoomYRange + yOff ),
                                    (GLfloat)0.0 );
                     }
                  }
               }
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

         for( double xOff = -atomRadiusX; xOff < atomRadiusX; xOff += 1.0 / (double)zoomXWindow )
         {
            for( double yOff = -atomRadiusY; yOff < atomRadiusY; yOff += 1.0 / (double)zoomYWindow )
            {
               glVertex3f( (GLfloat)( ( 0.0 + 1 - minX ) / (double)zoomXRange + xOff ),
                           (GLfloat)( ( y + 1 - minY ) / (double)zoomYRange + yOff ),
                           (GLfloat)0.0 );
            }
         }

         // Central membrane with pores
         glColor3f( 0.f, 0.f, 0.f );   // Black
         for( i = 0; i < numK; i++ )
         {
            if( y == (int)posK[ i ] )
            {
               if( o->selectivity )
               {
                  glColor3f( 1.f, 0.64f, 0.57f );  // Pale red
               } else {
                  glColor3f( 1.f, 1.f, 1.f );      // White
               }
            }
         }
         for( i = 0; i < numNa; i++ )
         {
            if( y == (int)posNa[ i ] )
            {
               if( o->selectivity )
               {
                  glColor3f( 0.57f, 0.57f, 1.f );  // Pale blue
               } else {
                  glColor3f( 1.f, 1.f, 1.f );      // White
               }
            }
         }
         for( i = 0; i < numCl; i++ )
         {
            if( y == (int)posCl[ i ] )
            {
               if( o->selectivity )
               {
                  glColor3f( 0.57f, 0.87f, 0.72f );// Pale green
               } else {
                  glColor3f( 1.f, 1.f, 1.f );      // White
               }
            }
         }

         for( double xOff = -atomRadiusX; xOff < atomRadiusX; xOff += 1.0 / (double)zoomXWindow )
         {
            for( double yOff = -atomRadiusY; yOff < atomRadiusY; yOff += 1.0 / (double)zoomYWindow )
            {
               glVertex3f( (GLfloat)( ( o->x / 2 + 1 - minX ) / (double)zoomXRange + xOff ),
                           (GLfloat)( ( y + 1 - minY ) / (double)zoomYRange + yOff ),
                           (GLfloat)0.0 );
            }
         }

         // Right membrane
         glColor3f( 0.f, 0.f, 0.f );   // Black

         for( double xOff = -atomRadiusX; xOff < atomRadiusX; xOff += 1.0 / (double)zoomXWindow )
         {
            for( double yOff = -atomRadiusY; yOff < atomRadiusY; yOff += 1.0 / (double)zoomYWindow )
            {
               glVertex3f( (GLfloat)( ( o->x - minX ) / (double)zoomXRange + xOff ),
                           (GLfloat)( ( y + 1 - minY ) / (double)zoomYRange + yOff ),
                           (GLfloat)0.0 );
            }
         }
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

         if( o->electrostatics )
         {
            glColor3f( 1.f, 0.15f, 0.f );    // Red
         } else {
            glColor3f( 1.f, 0.64f, 0.57f );  // Pale red
         }

         for( double xOff = -atomRadiusX; xOff < atomRadiusX; xOff += 1.0 / (double)zoomXWindow )
         {
            for( double yOff = -atomRadiusY; yOff < atomRadiusY; yOff += 1.0 / (double)zoomYWindow )
            {
               glVertex3f( (GLfloat)( ( x + 1 - minX ) / (double)zoomXRange + xOff ),
                           (GLfloat)( ( y + 1 - minY ) / (double)zoomYRange + yOff ),
                           (GLfloat)0.0 );
            }
         }
         placed++;
      }

      for( i = 0; i < numNa && placed < o->max_atoms; i++ )
      {
         x = ( posNa[ i ] % ( o->x / 2 - 1 ) ) + 1;
         y =   posNa[ i ] / ( o->x / 2 - 1 );

         if( o->electrostatics )
         {
            glColor3f( 0.f, 0.f, 1.f );      // Blue
         } else {
            glColor3f( 0.57f, 0.57f, 1.f );  // Pale blue
         }

         for( double xOff = -atomRadiusX; xOff < atomRadiusX; xOff += 1.0 / (double)zoomXWindow )
         {
            for( double yOff = -atomRadiusY; yOff < atomRadiusY; yOff += 1.0 / (double)zoomYWindow )
            {
               glVertex3f( (GLfloat)( ( x + 1 - minX ) / (double)zoomXRange + xOff ),
                           (GLfloat)( ( y + 1 - minY ) / (double)zoomYRange + yOff ),
                           (GLfloat)0.0 );
            }
         }
         placed++;
      }

      for( i = 0; i < numCl && placed < o->max_atoms; i++ )
      {
         x = ( posCl[ i ] % ( o->x / 2 - 1 ) ) + 1;
         y =   posCl[ i ] / ( o->x / 2 - 1 );

         if( o->electrostatics )
         {
            glColor3f( 0.f, 0.70f, 0.35f );  // Green
         } else {
            glColor3f( 0.57f, 0.87f, 0.72f );// Pale green
         }

         for( double xOff = -atomRadiusX; xOff < atomRadiusX; xOff += 1.0 / (double)zoomXWindow )
         {
            for( double yOff = -atomRadiusY; yOff < atomRadiusY; yOff += 1.0 / (double)zoomYWindow )
            {
               glVertex3f( (GLfloat)( ( x + 1 - minX ) / (double)zoomXRange + xOff ),
                           (GLfloat)( ( y + 1 - minY ) / (double)zoomYRange + yOff ),
                           (GLfloat)0.0 );
            }
         }
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

         if( o->electrostatics )
         {
            glColor3f( 1.f, 0.15f, 0.f );    // Red
         } else {
            glColor3f( 1.f, 0.64f, 0.57f );  // Pale red
         }

         for( double xOff = -atomRadiusX; xOff < atomRadiusX; xOff += 1.0 / (double)zoomXWindow )
         {
            for( double yOff = -atomRadiusY; yOff < atomRadiusY; yOff += 1.0 / (double)zoomYWindow )
            {
               glVertex3f( (GLfloat)( ( x + 1 - minX ) / (double)zoomXRange + xOff ),
                           (GLfloat)( ( y + 1 - minY ) / (double)zoomYRange + yOff ),
                           (GLfloat)0.0 );
            }
         }
        placed++;
      }

      for( i = 0; i < numNa && placed < o->max_atoms; i++ )
      {
         x = ( posNa[ i ] % ( o->x / 2 - 2 ) ) + o->x / 2 + 1;
         y =   posNa[ i ] / ( o->x / 2 - 2 );

         if( o->electrostatics )
         {
            glColor3f( 0.f, 0.f, 1.f );      // Blue
         } else {
            glColor3f( 0.57f, 0.57f, 1.f );  // Pale blue
         }

         for( double xOff = -atomRadiusX; xOff < atomRadiusX; xOff += 1.0 / (double)zoomXWindow )
         {
            for( double yOff = -atomRadiusY; yOff < atomRadiusY; yOff += 1.0 / (double)zoomYWindow )
            {
               glVertex3f( (GLfloat)( ( x + 1 - minX ) / (double)zoomXRange + xOff ),
                           (GLfloat)( ( y + 1 - minY ) / (double)zoomYRange + yOff ),
                           (GLfloat)0.0 );
            }
         }
         placed++;
      }

      for( i = 0; i < numCl && placed < o->max_atoms; i++ )
      {
         x = ( posCl[ i ] % ( o->x / 2 - 2 ) ) + o->x / 2 + 1;
         y =   posCl[ i ] / ( o->x / 2 - 2 );

         if( o->electrostatics )
         {
            glColor3f( 0.f, 0.70f, 0.35f );  // Green
         } else {
            glColor3f( 0.57f, 0.87f, 0.72f );// Pale green
         }

         for( double xOff = -atomRadiusX; xOff < atomRadiusX; xOff += 1.0 / (double)zoomXWindow )
         {
            for( double yOff = -atomRadiusY; yOff < atomRadiusY; yOff += 1.0 / (double)zoomYWindow )
            {
               glVertex3f( (GLfloat)( ( x + 1 - minX ) / (double)zoomXRange + xOff ),
                           (GLfloat)( ( y + 1 - minY ) / (double)zoomYRange + yOff ),
                           (GLfloat)0.0 );
            }
         }
         placed++;
      }

      glEnd();
   }
}

