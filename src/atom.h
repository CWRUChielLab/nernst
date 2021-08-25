/* atom.h
 *
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


#ifndef ATOM_H
#define ATOM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

extern int nAtoms;
extern int WORLD_T;
extern unsigned int WORLD_COUNTER;
extern int LRcharge;

extern int initLHS_K,  initRHS_K;
extern int initLHS_Na, initRHS_Na;
extern int initLHS_Cl, initRHS_Cl;

extern unsigned int *positionsLHS;
extern unsigned int *positionsRHS;
extern unsigned int *positionsPORES;

struct atom
{
   int delta_x, delta_y;   // 4 bytes, 4 bytes
   uint8_t color;          // 1 byte
                           // ----------------
                           // 17 bytes
};

unsigned long int  idx( int x, int y );
int getX( unsigned int position );
int getY( unsigned int position );
int ionCharge( unsigned int position );
int isMembrane( unsigned int position );
int isSolvent( unsigned int position );
int isPore( unsigned int position );
int isAtom( unsigned int position );
int isUntrackedAtom( unsigned int position );
int isTrackedAtom( unsigned int position );
int isPermeable( unsigned int porePos, unsigned int ionPos );
void shufflePositions( struct options *options );
void initAtoms( struct options *options );
void moveAtoms( void );
void distributePores( struct options *options );
void finalizeAtoms( void );
void takeCensus( int iter );


// Things that need colors
enum
{
   SOLVENT,
   ATOM_K,
   ATOM_K_TRACK,
   ATOM_Na,
   ATOM_Na_TRACK,
   ATOM_Cl,
   ATOM_Cl_TRACK,
   MEMBRANE,
   PORE_K,
   PORE_Na,
   PORE_Cl
};

enum
{
   DIR_MASK = 0x7
};


#ifdef __cplusplus
}
#endif

#endif /* ATOM_H */

