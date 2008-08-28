/* atom.c
 *
 *
 * Copyright (c) 2008, Jeffrey Gill, Barry Rountree, Kendrick Shaw, 
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
 *
 *
 * Atom-specific code. Includes atom movement.
 */


#define _GNU_SOURCE     // for posix_memalign()
#ifdef BLR_USEMAC
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#include <assert.h>
#include <stdlib.h>     // srand(), rand()
#include <stdio.h>
#include <math.h>       // sqrt(), ceil(), exp()
#include <unistd.h>
#include <SFMT.h>
#include <string.h>

#include "options.h"
#include "atom.h"
#include "world.h"
#include "util.h"


struct options *o;
static int WORLD_SZ_MASK;
unsigned int WORLD_COUNTER;
int LRcharge;           // Net charge on left minus net charge on right

int initLHS_K,  initRHS_K;
int initLHS_Na, initRHS_Na;
int initLHS_Cl, initRHS_Cl;

signed int off_n, off_s, off_e, off_w, off_ne, off_nw, off_se, off_sw;
signed int *dir2offset;

unsigned int *positionsLHS;
unsigned int *positionsRHS;
unsigned int *positionsPORES;


unsigned long int
idx( int x, int y )
{
   // Mask takes care of wrapping in the torus
   return( ( y * o->x + x ) & WORLD_SZ_MASK );
}


int
getX( unsigned int position )
{
   return( (int)( position % o->x ) );
}


int
getY( unsigned int position )
{
   return( (int)( position / o->x ) );
}


int
ionCharge( unsigned int position )
{
   int q;
   switch( world[ position ].color )
   {
      case ATOM_K:
      case ATOM_K_TRACK:
         q = 1;
         break;
      case ATOM_Na:
      case ATOM_Na_TRACK:
         q = 1;
         break;
      case ATOM_Cl:
      case ATOM_Cl_TRACK:
         q = -1;
         break;
      default:
         q = 0;
#ifndef QT_NO_DEBUG
         ASSERT( q != 0 );
#endif /* QT_NO_DEBUG */
         break;
   }
   return q;
}


int
isMembrane( unsigned int position )
{
   return ( world[ position ].color == MEMBRANE ); 
}


int
isSolvent( unsigned int position )
{
   return ( world[ position ].color == SOLVENT );
}


int
isPore( unsigned int position )
{
   return ( world[ position ].color == PORE_K  ||
            world[ position ].color == PORE_Na ||
            world[ position ].color == PORE_Cl );
}


int
isAtom( unsigned int position )
{
   return ( world[ position ].color == ATOM_K        ||
            world[ position ].color == ATOM_K_TRACK  ||
            world[ position ].color == ATOM_Na       ||
            world[ position ].color == ATOM_Na_TRACK ||
            world[ position ].color == ATOM_Cl       ||
            world[ position ].color == ATOM_Cl_TRACK );
}


int
isUntrackedAtom( unsigned int position )
{
   return ( world[ position ].color == ATOM_K        ||
            world[ position ].color == ATOM_Na       ||
            world[ position ].color == ATOM_Cl       );
}


int
isTrackedAtom( unsigned int position )
{
   return ( world[ position ].color == ATOM_K_TRACK  ||
            world[ position ].color == ATOM_Na_TRACK ||
            world[ position ].color == ATOM_Cl_TRACK );
}


int
isPermeable( unsigned int porePos, unsigned int ionPos )
{

   if( isPore( porePos ) && isAtom( ionPos ) )
   {
      if( !o->selectivity )
      {
         return 1;
      }

      uint8_t poreType = world[ porePos ].color;
      uint8_t ionType = world[ ionPos ].color;

      switch( poreType )
      {
         case PORE_K:
            return ( ionType == ATOM_K || ionType == ATOM_K_TRACK );
            break;
         case PORE_Na:
            return ( ionType == ATOM_Na || ionType == ATOM_Na_TRACK );
            break;
         case PORE_Cl:
            return ( ionType == ATOM_Cl || ionType == ATOM_Cl_TRACK );
            break;
         default:
#ifndef QT_NO_DEBUG
            ASSERT( poreType == PORE_K || poreType == PORE_Na || poreType == PORE_Cl );
#endif /* QT_NO_DEBUG */
            return 0;
      }
   } else {
#ifndef QT_NO_DEBUG
      ASSERT( isPore( porePos ) && isAtom( ionPos ) );
#endif /* QT_NO_DEBUG */
      return 0;
   }
}


static int 
dir2dx[] =
{
    0, // N
    0, // S
    1, // E
   -1, // W
    1, // NE
   -1, // NW
    1, // SE
   -1  // SW
};


static int 
dir2dy[] =
{
   -1, // N
    1, // S
    0, // E
    0, // W
   -1, // NE
   -1, // NW
    1, // SE
    1  // SW
};


static void
copyAtom( unsigned int from, unsigned int to, int dx, int dy )
{
   world[ to ].delta_x = world[ from ].delta_x + dx;
   world[ to ].delta_y = world[ from ].delta_y + dy;
   world[ to ].color   = world[ from ].color;

   world[ from ].delta_x = SOLVENT;
   world[ from ].delta_y = SOLVENT;
   world[ from ].color   = SOLVENT;
}


static int
shouldTransport( unsigned int from, unsigned int to )
{
   // Returns a 1 if the ion can move through the pore, else 0.
   // Assumes we have already checked that ion and pore types
   // match. Here we use direction[ to ] as a random number, not
   // a direction.

   if( !isSolvent( to ) )
   {
#ifndef QT_NO_DEBUG
      ASSERT( isSolvent( to ) );
#endif /* QT_NO_DEBUG */
      return 0;
   }

   if( !o->electrostatics )
   {
      return ( direction[ to ] % 256 <= 127 );
   }

   int q;
   q = ionCharge( from );

   // Moving left to right across membrane.
   if( getX( from ) == o->x / 2 - 1 && getX( to ) == o->x / 2 + 1 )
   {
      return ( direction[ to ] % 256 <= 16 * exp( o->cBoltz * LRcharge * q / o->y ) );
   } else {
      // Moving right to left across membrane.
      if( getX( from ) == o->x / 2 + 1 && getX( to ) == o->x / 2 - 1 )
      {
         return ( direction[ to ] % 256 <= 16 * exp( o->cBoltz * LRcharge * -q / o->y ) );
      } else {
#ifndef QT_NO_DEBUG
         ASSERT( ( getX( from ) == o->x / 2 - 1 && getX( to ) == o->x / 2 + 1 ) || ( getX( from ) == o->x / 2 + 1 && getX( from ) == o->x / 2 - 1 ) );
#endif /* QT_NO_DEBUG */
         return 0;
      }
   }
}


void
shufflePositions( struct options *o )
{
   static int initialized = 0;
   unsigned int i, highest, lowest, range, rand, temp;

   if( !initialized )
   {
      positionsLHS   = malloc( sizeof( unsigned int ) * ( MAX_X / 2 - 1 ) * ( MAX_Y ) );
      positionsRHS   = malloc( sizeof( unsigned int ) * ( MAX_X / 2 - 2 ) * ( MAX_Y ) );
      positionsPORES = malloc( sizeof( unsigned int ) * ( 1 )             * ( MAX_Y / 2 ) );

      assert( positionsLHS && positionsRHS && positionsPORES );
      initialized = 1;
   }

   // Initialize the position arrays
   for( i = 0; i < (unsigned int)( ( o->x / 2 - 1 ) * ( o->y ) ); i++ )
   {
      positionsLHS[ i ] = i;
   }

   for( i = 0; i < (unsigned int)( ( o->x / 2 - 2 ) * ( o->y ) ); i++ )
   {
      positionsRHS[ i ] = i;
   }

   for( i = 0; i < (unsigned int)( ( 1 ) * ( o->y / 2 ) ); i++ )
   {
      positionsPORES[ i ] = 2 * i;
   }

   // Shuffle the position arrays
   init_gen_rand( (uint32_t)(o->randseed) );

   highest = ( o->x / 2 - 1 ) * ( o->y ) - 1;
   for( i = 0; i < highest; i++ )
   {
      lowest = i + 1;
      range = highest - lowest + 1;
      rand = ( gen_rand32() % range ) + lowest;
      temp = positionsLHS[ i ];
      positionsLHS[ i ] = positionsLHS[ rand ];
      positionsLHS[ rand ] = temp;
   }

   highest = ( o->x / 2 - 2 ) * ( o->y ) - 1;
   for( i = 0; i < highest; i++ )
   {
      lowest = i + 1;
      range = highest - lowest + 1;
      rand = ( gen_rand32() % range ) + lowest;
      temp = positionsRHS[ i ];
      positionsRHS[ i ] = positionsRHS[ rand ];
      positionsRHS[ rand ] = temp;
   }

   highest = ( 1 ) * ( o->y / 2 ) - 1;
   for( i = 0; i < highest; i++ )
   {
      lowest = i + 1;
      range = highest - lowest + 1;
      rand = ( gen_rand32() % range ) + lowest;
      temp = positionsPORES[ i ];
      positionsPORES[ i ] = positionsPORES[ rand ];
      positionsPORES[ rand ] = temp;
   }
}


void
distributePores( struct options *o )
{
   if( world == NULL )
   {
      return;
   }

   int i, y;
   int current_idx, numK, numNa, numCl;
   unsigned int *posK, *posNa, *posCl;

   for( y = 0; y < o->y; y++ )
   {
      current_idx = idx( o->x / 2, y );
      world[ current_idx ].color = MEMBRANE;
   }

   numK  = (int)( (double)( 1.0 ) * (double)( o->y / 2 ) / 3.0 * o->pK  + 0.5 );
   numNa = (int)( (double)( 1.0 ) * (double)( o->y / 2 ) / 3.0 * o->pNa + 0.5 );
   numCl = (int)( (double)( 1.0 ) * (double)( o->y / 2 ) / 3.0 * o->pCl + 0.5 );

   posK  = positionsPORES;
   posNa = positionsPORES + (int)( (double)( ( 1 ) * ( o->y / 2 ) ) * 1.0 / 3.0 );
   posCl = positionsPORES + (int)( (double)( ( 1 ) * ( o->y / 2 ) ) * 2.0 / 3.0 );

   for( i = 0; i < numK; i++ )
   {
      y = posK[ i ];
      current_idx = idx( o->x / 2, y );
      world[ current_idx ].color = PORE_K;
   }
 
   for( i = 0; i < numNa; i++ )
   {
      y = posNa[ i ];
      current_idx = idx( o->x / 2, y );
      world[ current_idx ].color = PORE_Na;
   }

   for( i = 0; i < numCl; i++ )
   {
      y = posCl[ i ];
      current_idx = idx( o->x / 2, y );
      world[ current_idx ].color = PORE_Cl;
   }
}


void
initAtoms( struct options *options )
{
   // Walk through the array of atoms and assign each a position and color.

   o = options;

   off_n  = ( -o->x     );
   off_s  = (  o->x     );
   off_e  = (         1 );
   off_w  = (        -1 );
   off_ne = ( -o->x + 1 );
   off_nw = ( -o->x - 1 );
   off_se = (  o->x + 1 );
   off_sw = (  o->x - 1 );

   dir2offset = malloc( sizeof( unsigned int ) * 8 );
   assert( dir2offset );
   dir2offset[ 0 ] = off_n;
   dir2offset[ 1 ] = off_s;
   dir2offset[ 2 ] = off_e;
   dir2offset[ 3 ] = off_w;
   dir2offset[ 4 ] = off_ne;
   dir2offset[ 5 ] = off_nw;
   dir2offset[ 6 ] = off_se;
   dir2offset[ 7 ] = off_sw;

   int x, y, i, current_idx = 0, placed = 0;
   int numK, numNa, numCl;
   unsigned int *posK, *posNa, *posCl;

   // Initialize the Mersenne twister random number generator.
   init_gen_rand( (uint32_t)(o->randseed) );

   WORLD_SZ_MASK = o->x * o->y - 1;
   LRcharge   = 0;
   initLHS_K  = 0;
   initRHS_K  = 0;
   initLHS_Na = 0;
   initRHS_Na = 0;
   initLHS_Cl = 0;
   initRHS_Cl = 0;

   // Set up the solvent.
   for( i = 0; i < o->x * o->y; i++ )
   {
      world[ i ].color = SOLVENT;
   }

   // Initialize LHS atoms.
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
      current_idx = idx( x, y );

      world[ current_idx ].delta_x = 0;
      world[ current_idx ].delta_y = 0;
      world[ current_idx ].color   = ATOM_K;
      LRcharge++;
      initLHS_K++;

      placed++;
   }

   for( i = 0; i < numNa && placed < o->max_atoms; i++ )
   {
      x = ( posNa[ i ] % ( o->x / 2 - 1 ) ) + 1;
      y =   posNa[ i ] / ( o->x / 2 - 1 );
      current_idx = idx( x, y );

      world[ current_idx ].delta_x = 0;
      world[ current_idx ].delta_y = 0;
      world[ current_idx ].color   = ATOM_Na;
      LRcharge++;
      initLHS_Na++;

      placed++;
   }

   for( i = 0; i < numCl && placed < o->max_atoms; i++ )
   {
      x = ( posCl[ i ] % ( o->x / 2 - 1 ) ) + 1;
      y =   posCl[ i ] / ( o->x / 2 - 1 );
      current_idx = idx( x, y );

      world[ current_idx ].delta_x = 0;
      world[ current_idx ].delta_y = 0;
      world[ current_idx ].color   = ATOM_Cl;
      LRcharge--;
      initLHS_Cl++;

      placed++;
   }

   // Initialize RHS atoms.
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
      current_idx = idx( x, y );

      world[ current_idx ].delta_x = 0;
      world[ current_idx ].delta_y = 0;
      world[ current_idx ].color   = ATOM_K;
      LRcharge--;
      initRHS_K++;

      placed++;
   }

   for( i = 0; i < numNa && placed < o->max_atoms; i++ )
   {
      x = ( posNa[ i ] % ( o->x / 2 - 2 ) ) + o->x / 2 + 1;
      y =   posNa[ i ] / ( o->x / 2 - 2 );
      current_idx = idx( x, y );

      world[ current_idx ].delta_x = 0;
      world[ current_idx ].delta_y = 0;
      world[ current_idx ].color   = ATOM_Na;
      LRcharge--;
      initRHS_Na++;

      placed++;
   }

   for( i = 0; i < numCl && placed < o->max_atoms; i++ )
   {
      x = ( posCl[ i ] % ( o->x / 2 - 2 ) ) + o->x / 2 + 1;
      y =   posCl[ i ] / ( o->x / 2 - 2 );
      current_idx = idx( x, y );

      world[ current_idx ].delta_x = 0;
      world[ current_idx ].delta_y = 0;
      world[ current_idx ].color   = ATOM_Cl;
      LRcharge++;
      initRHS_Cl++;

      placed++;
   }

   LRcharge = 0;

   // Set up the membranes.
   for( y = 0; y < o->y; y++ )
   {
      current_idx = idx( 0, y );
      world[ current_idx ].color = MEMBRANE;
      current_idx = idx( o->x / 2, y );
      world[ current_idx ].color = MEMBRANE;
      current_idx = idx( o->x - 1, y );
      world[ current_idx ].color = MEMBRANE;
   }

   distributePores( o );

   // Record number of atoms placed to print out later.
   o->max_atoms = placed;
}


void
moveAtoms()
{
   unsigned int dir = 0, off = 0, from = 0, to = 0;
   unsigned int WORLD_SZ = o->x * o->y;

   // Only need to clear out claimed.
   memset( claimed, 0, o->x * o->y );

   // Get new set of directions.
   fill_array64( (uint64_t*)(direction), direction_sz64 / 8 );

   // Stake our claims for next turn.
   for( from = 0; from < WORLD_SZ; from++ )
   {
      if( isAtom( from ) )
      {                                            // If there's an atom present,
         claimed[ from ]++;                        // block anyone from moving here,
         dir = direction[ from ] & DIR_MASK;       // get my direction,
         off = dir2offset[ dir ];                  // get my offset,
         to = ( from + off ) & WORLD_SZ_MASK;      // add offset and normalize,
         claimed[ to ]++;                          // and stake my claim.
      }

      // Don't run through the membrane
      if( isMembrane( from ) || isPore( from ) )
      {
         claimed[ from ]++;
      }
   }

   // Move those that are eligible.
   for( from = 0; from < WORLD_SZ; from++ )
   {
      if( claimed[ from ] == 1 && isAtom( from ) )
      {                                            // If there's an atom present,
         dir = direction[ from ] & DIR_MASK;       // get my direction,
         off = dir2offset[ dir ];                  // get my offset,
         to = ( from + off ) & WORLD_SZ_MASK;      // and add offset and normalize.

         if( claimed[ to ] == 1 )
         {                                         // If it's safe to move,
            copyAtom( from, to, dir2dx[ dir ], dir2dy[ dir ] );   // copy the atom,
            claimed[ to ] = 0;                     // and make sure no other atoms move here.
         }
      }
   }

   // Transport atoms through pores.
   int y;
   unsigned int current_idx;
   for( y = 0; y < o->y; y++ )
   {
      current_idx = idx( o->x / 2, y );
      if( isPore( current_idx ) )
      {
         // Try movement from left to right
         from = idx( o->x / 2 - 1, y );
         to   = idx( o->x / 2 + 1, y );
         if( isAtom( from ) &&
             isSolvent( to ) &&
             isPermeable( current_idx, from ) &&
             shouldTransport( from, to ) )
         {
            int q = ionCharge( from );
            copyAtom( from, to, 2, 0 );
            LRcharge += -2 * q;
            switch( world[ to ].color )
            {
               case ATOM_K:
               case ATOM_K_TRACK:
                  initLHS_K--;
                  initRHS_K++;
                  break;
               case ATOM_Na:
               case ATOM_Na_TRACK:
                  initLHS_Na--;
                  initRHS_Na++;
                  break;
               case ATOM_Cl:
               case ATOM_Cl_TRACK:
                  initLHS_Cl--;
                  initRHS_Cl++;
                  break;
               default:
#ifndef QT_NO_DEBUG
                  ASSERT( isAtom( from ) );
#endif /* QT_NO_DEBUG */
                  break;
            }
         } else {
            // Try movement from right to left
            from = idx( o->x / 2 + 1, y );
            to   = idx( o->x / 2 - 1, y );
            if( isAtom( from ) &&
                isSolvent( to ) &&
                isPermeable( current_idx, from ) &&
                shouldTransport( from, to ) )
            {
               int q = ionCharge( from );
               copyAtom( from, to, -2, 0 );
               LRcharge += 2 * q;
               switch( world[ to ].color )
               {
                  case ATOM_K:
                  case ATOM_K_TRACK:
                     initLHS_K++;
                     initRHS_K--;
                     break;
                  case ATOM_Na:
                  case ATOM_Na_TRACK:
                     initLHS_Na++;
                     initRHS_Na--;
                     break;
                  case ATOM_Cl:
                  case ATOM_Cl_TRACK:
                     initLHS_Cl++;
                     initRHS_Cl--;
                     break;
                  default:
#ifndef QT_NO_DEBUG
                     ASSERT( isAtom( from ) );
#endif /* QT_NO_DEBUG */
                     break;
               }
            }
         }
      }
   }
}


// Count atoms of each type on the LHS and RHS of the membrane and in pores.
void
takeCensus( int iter )
{
   int x, y;
   int K, Na, Cl;
   static int initialized = 0;
   static FILE *fp;

   if( iter < 0 )
   {
      if( fp )
      {
         fclose( fp );
      }
      initialized = 0;
      return;
   }

   if( !initialized )
   {
      initialized = 1;
      fp = fopen( "static.out", "w" );
      if( fp )
      {
         fprintf( fp, "T LK LNa LCl RK RNa RCl q\n" );
      }
   }

   if( fp )
   {
      fprintf( fp, "%d ", iter );

      // Count atoms on LHS
      for( x = 0, K = 0, Na = 0, Cl = 0; x < o->x / 2; x++ )
      {
         for( y = 0; y < o->y; y++ )
         {
            switch( world[ idx( x, y ) ].color )
            {
               case ATOM_K:
               case ATOM_K_TRACK:
                  K++;
                  break;
               case ATOM_Na:
               case ATOM_Na_TRACK:
                  Na++;
                  break;
               case ATOM_Cl:
               case ATOM_Cl_TRACK:
                  Cl++;
               default:
                  break;
            }
         }
      }
      fprintf( fp, "%d %d %d ", K, Na, Cl );

      // Count atoms on RHS
      for( x = o->x / 2 + 1, K = 0, Na = 0, Cl = 0; x < o->x; x++ )
      {
         for( y = 0; y < o->y; y++ )
         {
            switch( world[ idx( x, y ) ].color )
            {
               case ATOM_K:
               case ATOM_K_TRACK:
                  K++;
                  break;
               case ATOM_Na:
               case ATOM_Na_TRACK:
                  Na++;
                  break;
               case ATOM_Cl:
               case ATOM_Cl_TRACK:
                  Cl++;
                  break;
               default:
                  break;
            }
         }
      }
      fprintf( fp, "%d %d %d ", K, Na, Cl );

      // Output net charge across membrane
      fprintf( fp, "%d\n", LRcharge );
   }
}


void
finalizeAtoms()
{
   FILE *fp;
   int x, y;
   takeCensus( -1 );
   fp = fopen( "world.out", "w" );
   if( fp )
   {
      fprintf( fp, "type dx dy\n" );
      for( x = 0; x < o->x; x++ )
      {
         for( y = 0; y < o->y; y++ )
         {
            if( isAtom( idx( x, y ) ) )
            {
               int type = 0;
               switch( world[ idx( x, y ) ].color )
               {
                  case ATOM_K:
                  case ATOM_K_TRACK:
                     type = 1;
                     break;
                  case ATOM_Na:
                  case ATOM_Na_TRACK:
                     type = 2;
                     break;
                  case ATOM_Cl:
                  case ATOM_Cl_TRACK:
                     type = 3;
                     break;
                  default:
                     break;
               }
               fprintf( fp, "%d %d %d\n",
                  type,
                  world[ idx( x, y ) ].delta_x,
                  world[ idx( x, y ) ].delta_y );
            }
         }
      }
      fclose( fp );
   }
}

