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


struct options *o;
static int WORLD_SZ_MASK;
unsigned int WORLD_COUNTER;
int LRcharge;           // Net charge on left minus net charge on right
int initLHS_K, initRHS_K, initLHS_Cl, initRHS_Cl;  // Initial ion counts

signed int off_n, off_s, off_e, off_w, off_ne, off_nw, off_se, off_sw;
signed int *dir2offset;

unsigned int *positionsLHS;
unsigned int *positionsRHS;


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
ionCharge( uint8_t type )
{
   int q;
   switch( type )
   {
      case ATOM_K:
         q = 1;
         break;
      case ATOM_K_TRACK:
         q = 1;
         break;
      case ATOM_Cl:
         q = -1;
         break;
      case ATOM_Cl_TRACK:
         q = -1;
         break;
      default:
         q = 0;
         break;
   }
   return q;
}


int
isMembrane( unsigned int position )
{
   if( getX( position ) == 0 || getX( position ) == o->x - 1 )
   {
      return 1;
   }
   if( getX( position ) == o->x / 2 )
   {
      int i, y;
      for( i = 1; i <= o->pores; i++ )
      {
         y = (int)( ( (double)o->y / (double)( o->pores + 1 ) ) * (double)i );
         if( getY( position ) == y )
         {
            return 0;
         }
      }
      return 1;
   }
   return 0;
}


int
isPore( unsigned int position )
{
   if( getX( position ) == o->x / 2 )
   {
      int i, y;
      for( i = 1; i <= o->pores; i++ )
      {
         y = (int)( ( (double)o->y / (double)( o->pores + 1 ) ) * (double)i );
         if( getY( position ) == y )
         {
            return 1;
         }
      }
   }
   return 0;
}


int
isPermeable( uint8_t type )
{
   int p;
   switch( type )
   {
      case ATOM_K:
         p = 1;
         break;
      case ATOM_K_TRACK:
         p = 1;
         break;
      case ATOM_Cl:
         if( o->selectivity )
         {
            p = 0;
         } else {
            p = 1;
         }
         break;
      case ATOM_Cl_TRACK:
         if( o->selectivity )
         {
            p = 0;
         } else {
            p = 1;
         }
         break;
      default:
         p = 0;
         break;
   }
   return p;
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
   if( isMembrane( from ) )
   {
      world[ from ].delta_x = MEMBRANE;
      world[ from ].delta_y = MEMBRANE;
      world[ from ].color   = MEMBRANE;
   } else {
      world[ from ].delta_x = SOLVENT;
      world[ from ].delta_y = SOLVENT;
      world[ from ].color   = SOLVENT;
   }
}


static int
chargeFlux( unsigned int from, unsigned int to )
{
   int q = ionCharge( world[ from ].color );

   // If the moving atom is entering a pore from the left or exiting a pore to the right
   if( ( getX( to ) == o->x / 2 && getX( from ) < o->x / 2 ) ||
       ( getX( from ) == o->x / 2 && getX( to ) > o->x / 2 ) )
   {
      return( -q );
   } else {
      // If the moving atom is entering a pore from the right or exiting a pore to the left
      if( ( getX( to ) == o->x / 2 && getX( from ) > o->x / 2 ) ||
          ( getX( from ) == o->x / 2 && getX( to ) < o->x / 2 ) )
      {
         return( q );
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

   int q, dir;

   q = ionCharge( world[ from ].color );

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
shufflePositions( struct options *o )
{
   static int initialized = 0;
   unsigned int i, highest, lowest, range, rand, temp;

   if( !initialized )
   {
      positionsLHS = malloc( sizeof( unsigned int ) * ( MAX_X / 2 - 1 ) * ( MAX_Y ) );
      positionsRHS = malloc( sizeof( unsigned int ) * ( MAX_X / 2 - 2 ) * ( MAX_Y ) );

      assert( positionsLHS && positionsRHS );
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

   int x, y, i, numIons, current_idx = 0, placed = 0;

   // Initialize the Mersenne twister random number generator.
   init_gen_rand( (uint32_t)(o->randseed) );

   WORLD_SZ_MASK = o->x * o->y - 1;
   LRcharge = 0;
   initLHS_K = 0;
   initRHS_K = 0;
   initLHS_Cl = 0;
   initRHS_Cl = 0;

   // Set up the solvent.
   for( i = 0; i < o->x * o->y; i++ )
   {
      world[ i ].color = SOLVENT;
   }

   // Initialize LHS atoms.
   numIons = (int)( (double)( o->x / 2 - 1 ) * (double)( o->y ) * (double)( o->lconc ) / (double)MAX_CONC + 0.5 );
   for( i = 0; i < numIons && placed < o->max_atoms; i++ )
   {
      placed++;

      x = ( positionsLHS[ i ] % ( o->x / 2 - 1 ) ) + 1;
      y = positionsLHS[ i ] / ( o->x / 2 - 1 );
      current_idx = idx( x, y );

      world[ current_idx ].delta_x = 0;
      world[ current_idx ].delta_y = 0;

      if( i % 2 == 0 )
      {
         // ATOM_K
         world[ current_idx ].color = ATOM_K;
         LRcharge++;
         initLHS_K++;
      } else {
         // ATOM_Cl
         world[ current_idx ].color = ATOM_Cl;
         LRcharge--;
         initLHS_Cl++;
      }
   }

   // Initialize RHS atoms.
   numIons = (int)( (double)( o->x / 2 - 2 ) * (double)( o->y ) * (double)( o->rconc ) / (double)MAX_CONC + 0.5 );
   for( i = 0; i < numIons && placed < o->max_atoms; i++ )
   {
      placed++;

      x = ( positionsRHS[ i ] % ( o->x / 2 - 2 ) ) + ( o->x / 2 + 1 );
      y = positionsRHS[ i ] / ( o->x / 2 - 2 );
      current_idx = idx( x, y );

      world[ current_idx ].delta_x = 0;
      world[ current_idx ].delta_y = 0;

      if( i % 2 == 0 )
      {
         // ATOM_K
         world[ current_idx ].color = ATOM_K;
         LRcharge--;
         initRHS_K++;
      } else {
         // ATOM_Cl
         world[ current_idx ].color = ATOM_Cl;
         LRcharge++;
         initRHS_Cl++;
      }
   }

   // Set up the membrane.
   for( y = 0; y < o->y; y++ )
   {
      current_idx = idx( o->x / 2, y );
      world[ current_idx ].color = MEMBRANE;
      current_idx = idx( 0, y );
      world[ current_idx ].color = MEMBRANE;
      current_idx = idx( o->x - 1, y );
      world[ current_idx ].color = MEMBRANE;
   }

   // Punch holes in the membrane for pores.
   for( i = 1, x = o->x / 2, y = 0; i <= o->pores; i++ )
   {
      y = (int)( ( (double)o->y / (double)( o->pores + 1 ) ) * (double)i );
      world[ idx( x, y ) ].color = SOLVENT;
   }

   o->max_atoms = placed;  // Record this to print out later.
}


void
moveAtoms()
{
   unsigned int dir = 0, off = 0, from = 0, to = 0;
   int atomCount = 0, chargeTemp = LRcharge;
   unsigned int WORLD_SZ = o->x * o->y;

   // Only need to clear out claimed.
   memset( claimed, 0, o->x * o->y );

   // Get new set of directions.
   fill_array64( (uint64_t*)(direction), direction_sz64 / 8 );

   // Stake our claims for next turn.
   for( from = 0; from < WORLD_SZ; from++ )
   {
      if( world[ from ].color != SOLVENT && world[ from ].color != MEMBRANE )
      {                                            // If there's an atom present,
         claimed[ from ] ++;                       // block anyone from moving here,
         dir = direction[ from ] & DIR_MASK;       // get my direction,
         off = dir2offset[ dir ];                  // get my offset,
         if( o->electrostatics && getX( from ) == o->x / 2 )
         {
            to = idx(
                  getX( from ) + dirPore( from ),
                  getY( ( from + off ) & WORLD_SZ_MASK )
                  );
         } else {
            to = ( from + off ) & WORLD_SZ_MASK;   // add offset and normalize,
         }
         claimed[ to ]++;                          // and stake my claim.
         atomCount++;
      }

      // Don't run through the membrane
      if( world[ from ].color == MEMBRANE )
      {
         claimed[ from ]++;
      }
   }

   // Move those that are eligible.
   for( from = 0; from < WORLD_SZ; from++ )
   {
      if( claimed[ from ] == 1 && world[ from ].color != SOLVENT && world[ from ].color != MEMBRANE )
      {                                            // If there's an atom present,
         dir = direction[ from ] & DIR_MASK;       // get my direction,
         off = dir2offset[ dir ];                  // get my offset,
         if( o->electrostatics && getX( from ) == o->x / 2 )
         {
            to = idx(
                  getX( from ) + dirPore( from ),
                  getY( ( from + off ) & WORLD_SZ_MASK )
                  );
         } else {
            to = ( from + off ) & WORLD_SZ_MASK;   // and add offset and normalize.
         }

         if( claimed[ to ] == 1 && ( isPermeable( world[ from ].color ) || !isPore( to ) ) )
         {                                         // If it's safe to move,
            chargeTemp += chargeFlux( from, to );  // take care of any charges moving across the membrane,
            copyAtom( from, to, dir2dx[ dir ], dir2dy[ dir ] );   // copy the atom,
            claimed[ to ] = 0;                     // and make sure no other atoms move here.
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

      // Count atoms on LHS
      for( x = 0, K = 0, Cl = 0; x < o->x / 2; x++ )
      {
         for( y = 0; y < o->y; y++ )
         {
            switch( world[ idx( x, y ) ].color )
            {
               case ATOM_K:
                  K++;
                  break;
               case ATOM_K_TRACK:
                  K++;
                  break;
               case ATOM_Cl:
                  Cl++;
                  break;
               case ATOM_Cl_TRACK:
                  Cl++;
               default:
                  break;
            }
         }
      }
      fprintf( fp, "%d %d ", K, Cl );

      // Count atoms on RHS
      for( x = o->x / 2 + 1, K = 0, Cl = 0; x < o->x; x++ )
      {
         for( y = 0; y < o->y; y++ )
         {
            switch( world[ idx( x, y ) ].color )
            {
               case ATOM_K:
                  K++;
                  break;
               case ATOM_K_TRACK:
                  K++;
                  break;
               case ATOM_Cl:
                  Cl++;
                  break;
               case ATOM_Cl_TRACK:
                  Cl++;
                  break;
               default:
                  break;
            }
         }
      }
      fprintf( fp, "%d %d ", K, Cl );

      // Count atoms in pores
      for( K = 0, Cl = 0, x = o->x / 2, y = 0; y < o->y; y++ )
      {
         switch( world[ idx( x, y ) ].color )
         {
            case ATOM_K:
               K++;
               break;
            case ATOM_K_TRACK:
               K++;
               break;
            case ATOM_Cl:
               Cl++;
               break;
            case ATOM_Cl_TRACK:
               Cl++;
               break;
            default:
               break;
         }
      }
      fprintf( fp, "%d %d ", K, Cl );

      // Output net charge across membrane
      fprintf( fp, "%d\n", LRcharge );
   }
}


void
redistributePores()
{
   if( world == NULL )
   {
      return;
   }
   int x, y;
   for( x = o->x / 2, y = 0; y < o->y; y++ )
   {
      if( world[ idx( x, y ) ].color == MEMBRANE || world[ idx( x, y ) ].color == SOLVENT )
      {
         if( isMembrane( idx( x, y ) ) )
         {
            world[ idx( x, y ) ].color = MEMBRANE;
         }
         if( isPore( idx( x, y ) ) )
         {
            world[ idx( x, y ) ].color = SOLVENT;
         }
      }
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
            if( world[ idx( x, y ) ].color != SOLVENT && world[ idx( x, y ) ].color != MEMBRANE )
            {
               fprintf( fp, "%d %d %d\n",
                  world[ idx( x, y ) ].color == ATOM_K || world[ idx( x, y ) ].color == ATOM_K_TRACK,
                  world[ idx( x, y ) ].delta_x,
                  world[ idx( x, y ) ].delta_y );
            }
         }
      }
      fclose( fp );
   }
}

