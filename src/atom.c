/* atom.c
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
#include "const.h"


struct options *o;
static int WORLD_SZ_MASK;
unsigned int WORLD_COUNTER;
int LRcharge;           // Net charge on left minus net charge on right
int initLHS_K, initRHS_K, initLHS_Cl, initRHS_Cl;  // Initial ion counts

signed int off_n, off_s, off_e, off_w, off_ne, off_nw, off_se, off_sw;
signed int *dir2offset;


unsigned long int
idx( int x, int y )
{
   // Mask takes care of wrapping in the torus
   return( ( y * o->x + x ) & WORLD_SZ_MASK );
}


int
getX( unsigned long int pos )
{
   return( (int)( pos % o->x ) );
}


int
getY( unsigned long int pos )
{
   return( (int)( pos / o->x ) );
}


// Directions are now 8 bits.
enum
{
   DIRECTION_N  = 0x01,
   DIRECTION_S  = 0x02,
   DIRECTION_E  = 0x04,
   DIRECTION_W  = 0x08,
   DIRECTION_NE = 0x10,
   DIRECTION_NW = 0x20,
   DIRECTION_SE = 0x40,
   DIRECTION_SW = 0x80
};


// Dirs are still 3 bits.  
enum
{
   DIR_N    = 0x0,  // b000
   DIR_S    = 0x1,  // b001
   DIR_E    = 0x2,  // b010
   DIR_W    = 0x3,  // b011
   DIR_NE   = 0x4,  // b100
   DIR_NW   = 0x5,  // b101
   DIR_SE   = 0x6,  // b110
   DIR_SW   = 0x7,  // b111
   DIR_MASK = 0x7   // b111
};


/*
enum
{
   OFF_N  = ( -WORLD_X   ),
   OFF_S  = (  WORLD_X   ),
   OFF_E  = (          1 ),
   OFF_W  = (         -1 ),
   OFF_NE = ( -WORLD_X+1 ),
   OFF_NW = ( -WORLD_X-1 ),
   OFF_SE = (  WORLD_X+1 ),
   OFF_SW = (  WORLD_X-1 ),

   REV_N  = (  WORLD_X   ),
   REV_S  = ( -WORLD_X   ),
   REV_E  = (         -1 ),
   REV_W  = (          1 ),
   REV_NE = (  WORLD_X-1 ),
   REV_NW = (  WORLD_X+1 ),
   REV_SE = ( -WORLD_X-1 ),
   REV_SW = ( -WORLD_X+1 )
};


static unsigned int
dir2offset[ 8 ] =
{
   OFF_N, OFF_S, OFF_E, OFF_W, OFF_NE, OFF_NW, OFF_SE, OFF_SW
};
*/


/*
static unsigned char
dir2direction[] =
{
   DIRECTION_N,
   DIRECTION_S,
   DIRECTION_E,
   DIRECTION_W,
   DIRECTION_NE,
   DIRECTION_NW,
   DIRECTION_SE,
   DIRECTION_SW
};
*/


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
   world[ to ].delta_x   = world[ from ].delta_x + dx;
   world[ to ].delta_y   = world[ from ].delta_y + dy;
   world[ to ].color     = world[ from ].color;
   world[ from ].delta_x = SOLVENT;
   world[ from ].delta_y = SOLVENT;
   world[ from ].color   = SOLVENT;
}


static int
chargeFlux(unsigned int from, unsigned int to)
{
   // If the moving atom is entering a pore from the left or exiting a pore to the right
   if( ( getX( to ) == o->x / 2 && getX( from ) < o->x / 2 ) ||
       ( getX( from ) == o->x / 2 && getX( to ) > o->x / 2 ) )
   {
      if( world[ from ].color == ATOM_K )
      {
         return( -1 );
      } else {
         return( 1 );
      }
   } else {
      // If the moving atom is entering a pore from the right or exiting a pore to the left
      if( ( getX( to ) == o->x / 2 && getX( from ) > o->x / 2 ) ||
          ( getX( from ) == o->x / 2 && getX( to ) < o->x / 2 ) )
      {
         if( world[ from ].color == ATOM_K )
         {
            return( 1 );
         } else {
            return( -1 );
         }
      } else {
         return( 0 );
      }
   }
}


static int
dirPore( unsigned int from )
{
   // Return a -1 for move left, a 1 for move right, or 0 for don't move.

   const double constant = e * e / ( 2 * k * t * c * a );

   signed int q, dir;

   switch( world[ from ].color )
   {
      case ATOM_K:
         q = 1;
         break;
      case ATOM_Cl:
         q = -1;
         break;
      default:
         q = 0;
         break;
   }

   // Using direction[ from ] as a random number, not a random direction.
   if( direction[ from ] % 256 <= 16 * exp( constant * LRcharge * -q / o->y ) )
   {
      dir = -1;
   } else {
      if ( direction[ from ] % 256 <=
               16 * exp( constant * LRcharge * -q / o->y )
             + 16 * exp( constant * LRcharge *  q / o->y ) )
      {
         dir = 1;
      } else {
         dir = 0;
      }
   }
   return( dir );
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
   dir2offset[ 0 ] = off_n;
   dir2offset[ 1 ] = off_s;
   dir2offset[ 2 ] = off_e;
   dir2offset[ 3 ] = off_w;
   dir2offset[ 4 ] = off_ne;
   dir2offset[ 5 ] = off_nw;
   dir2offset[ 6 ] = off_se;
   dir2offset[ 7 ] = off_sw;

   int x = 0, y = 0, i, current_idx = 0, nAtoms = 0, atomBit = 0;

   // Initialize the Mersenne twister random number generator.
   init_gen_rand( (uint32_t)(o->randseed) );

   WORLD_SZ_MASK = o->x * o->y - 1;
   LRcharge = 0;
   initLHS_K = 0;
   initRHS_K = 0;
   initLHS_Cl = 0;
   initRHS_Cl = 0;

   //Set up the solvent.
   for( i = 0; i < o->x * o->y; i++ )
   {
      world[ i ].color = SOLVENT;
   }

   // Initialize LHS atoms.
   for( y = 0; ( y < o->y ) && ( nAtoms < o->max_atoms ); y += o->lspacing )
   {
      for( x = 1; ( x < o->x / 2 ) && ( nAtoms < o->max_atoms ); x += o->lspacing )
      {
         current_idx = idx( x, y );
         world[ current_idx ].delta_x = 0;
         world[ current_idx ].delta_y = 0;

         if( atomBit )
         {
            world[ current_idx ].color = ATOM_K;
            LRcharge++;
            initLHS_K++;
         } else {
            world[ current_idx ].color = ATOM_Cl;
            LRcharge--;
            initLHS_Cl++;
         }
         atomBit = !atomBit;
         nAtoms++;
      }
   }

   // Initialize RHS atoms.
   for( y = 0; ( y < o->y ) && ( nAtoms < o->max_atoms ); y += o->rspacing )
   {
      // The RHS will always have an even number of squares per row, and so in order to produce
      // a checkered pattern of K and Cl ions when a spacing of 1 is used, we need to switch
      // atomBit every time we start a new row.
      atomBit = !atomBit;
      for( x = o->x / 2 + 1; ( x < o->x - 1 ) && ( nAtoms < o->max_atoms ); x += o->rspacing )
      {
         current_idx = idx( x, y );
         world[ current_idx ].delta_x = 0;
         world[ current_idx ].delta_y = 0;

         if( atomBit )
         {
            world[ current_idx ].color = ATOM_K;
            LRcharge--;
            initRHS_K++;
         } else {
            world[ current_idx ].color = ATOM_Cl;
            LRcharge++;
            initRHS_Cl++;
         }
         atomBit = !atomBit;
         nAtoms++;
      }
   }

   // Set up the membrane.
   for( y = 0; y < o->y; y++ )
   {
      current_idx = idx( o->x / 2, y );
      world[ current_idx ].color = (uint8_t) MEMBRANE;
      current_idx = idx( 0, y );
      world[ current_idx ].color = (uint8_t) MEMBRANE;
      current_idx = idx( o->x - 1, y );
      world[ current_idx ].color = (uint8_t) MEMBRANE;
   }

   // Punch holes in the membrane for pores.
   for( i = 0; i < o->pores; i++ )
   {
      world[ idx( o->x / 2, (int)( ( (double)o->y / (double)( o->pores + 1 ) ) * (double)( i + 1 ) ) ) ].color = SOLVENT;
   }

   o->max_atoms = nAtoms;  //record this to print out later.
}


void
moveAtoms()
{
   unsigned int dir = 0, off = 0, from = 0, to = 0;
   int atomCount = 0, chargeTemp = LRcharge;

   // Only need to clear out claimed.
   memset( claimed, 0, o->x * o->y );

   // Get new set of directions.
   fill_array64( (uint64_t*)(direction), direction_sz64/8 );

   // Stake our claims for next turn.
   for( from = 0; from < (unsigned int)( o->x * o->y ); from++ )
   {
      if( world[ from ].color == ATOM_K
      ||  world[ from ].color == ATOM_Cl )
      {                                            // If there's an atom present,
         if( o->electrostatics )
         {
            if( getX( from ) == o->x/2 )
            {
               claimed[ from ]++;			         // block anyone from moving here,
               to = idx(
                  getX( from ) + dirPore( from ),
                  getY(
                     ( from + dir2offset[ direction[ from ] & DIR_MASK ] )
                     & WORLD_SZ_MASK
                  )
               );                                  // vertical movement is BIASED!
               claimed[ to ]++;                    // and stake my claim.
               atomCount++;
            } else {
               claimed[ from ]++;                  // block anyone from moving here,
               dir = direction[ from ] & DIR_MASK; // get my direction,
               off = dir2offset[ dir ];            // look up my offset,
               to = ( from + off ) & WORLD_SZ_MASK;// add offset and normalize,
               claimed[ to ]++;                    // and stake my claim.
               atomCount++;
            }
         } else {
            claimed[ from ]++;                     // block anyone from moving here,
            dir = direction[ from ] & DIR_MASK;    // get my direction,
            off = dir2offset[ dir ];               // look up my offset,
            to = ( from + off ) & WORLD_SZ_MASK;   // add offset and normalize,
            claimed[ to ]++;                       // and stake my claim.
            atomCount++;
         }
      }

      // Don't run through the membrane
      if( world[ from ].color == MEMBRANE )
      {
         claimed[ from ]++;
      }
   }

   // Move those that are eligible.
   for( from = 0; from < (unsigned int)( o->x * o->y ); from++ )
   {
      // Can get rid of this "if" statement with a -1 + !.
      if( claimed[ from ] == 1
      &&( world[ from ].color == ATOM_K
        ||world[ from ].color == ATOM_Cl ) )
      {                                      // If there's an atom present,
         if( o->electrostatics )                // If using electrostatics,
         {
            if( getX( from ) == o->x/2 )
            {
               to = idx(
                  getX( from ) + dirPore( from ),
                  getY(
                     ( from + dir2offset[ direction[ from ] & DIR_MASK ] )
                     & WORLD_SZ_MASK
                  )
               );                                  // vertical movement is BIASED!
            } else {
               dir = direction[ from ] & DIR_MASK; // get my direction,
               off = dir2offset[ dir ];            // look up my offset,
               to = ( from + off ) & WORLD_SZ_MASK;// add offset and normalize.
            }
         } else {                               // Else don't use electrostatics.
            dir = direction[ from ] & DIR_MASK;    // get my direction,
            off = dir2offset[ dir ];               // look up my offset,
            to = ( from + off ) & WORLD_SZ_MASK;   // add offset and normalize.
         }
         if( o->selectivity )                   // If only allowing K atoms through pores.
         {
            if( claimed[ to ] == 1
               && (world[ from ].color == ATOM_K || getX( to ) != o->x/2 ) )
            {
               chargeTemp += chargeFlux( from, to );
               copyAtom( from, to, dir2dx[ dir ], dir2dy[ dir ] );
               claimed[ to ] = 0;
            }
         } else {                               // Else allow any atom through the pores.
            if( claimed[ to ] == 1 )
            {
               chargeTemp += chargeFlux( from, to );
               copyAtom( from, to, dir2dx[ dir ], dir2dy[ dir ] );
               claimed[ to ] = 0;
            }
         }
         atomCount++;
      }
   }
   LRcharge = chargeTemp;
}


// Count atoms of each type on the LHS and RHS of the membrane and in pores.
void
takeCensus( int iter )
{
   int x, y;
   int K, Cl;
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
         fprintf( fp, "T LK LCl RK RCl PK PCl q\n" );
      }
   }

   if( fp )
   {
      fprintf( fp, "%d ", iter );

      // Count atoms on left half
      for( x = 0, K = 0, Cl = 0; x < o->x / 2; x++ )
      {
         for( y = 0; y < o->y; y++ )
         {
            if( world[ idx( x, y ) ].color == ATOM_K )
            {
               K++;
            } else {
               if( world[ idx( x, y ) ].color == ATOM_Cl )
               {
                  Cl++;
               }
            }
         }
      }
      fprintf( fp, "%d %d ", K, Cl );

      // Count atoms on right half
      for( x = o->x / 2 + 1, K = 0, Cl = 0; x < o->x; x++ )
      {
         for( y = 0; y < o->y; y++ )
         {
            if( world[ idx( x, y ) ].color == ATOM_K )
            {
               K++;
            } else {
               if( world[ idx( x, y ) ].color == ATOM_Cl )
               {
                  Cl++;
               }
            }
         }
      }
      fprintf( fp, "%d %d ", K, Cl );

      // Count atoms in pores
      for( K = 0, Cl = 0, y = 0; y < o->y; y++ )
      {
         if( world[ idx( o->x / 2, y ) ].color == ATOM_K )
         {
            K++;
         } else {
            if( world[ idx( o->x / 2, y ) ].color == ATOM_Cl )
            {
               Cl++;
            }
         }
      }
      fprintf( fp, "%d %d ", K, Cl );

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
      fprintf( fp, "ATOM_K? dx dy\n" );
      for( x = 0; x < o->x; x++ )
      {
         for( y = 0; y < o->y; y++ )
         {
            if( world[ idx(x,y) ].color == ATOM_K
             || world[ idx(x,y) ].color == ATOM_Cl )
            {
               fprintf( fp, "%d %d %d\n",
                  world[ idx( x, y ) ].color == ATOM_K,
                  world[ idx( x, y ) ].delta_x,
                  world[ idx( x, y ) ].delta_y );
            }
         }
      }
      fclose( fp );
   }
}

