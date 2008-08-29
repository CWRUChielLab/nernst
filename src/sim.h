/* sim.h
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
 */

#ifndef SIM_H
#define SIM_H 

#include <QTime>

enum
{
   MIN_X = 16,
   MIN_Y = 16,
   MAX_X = 1024,
   MAX_Y = 1024,
   MIN_ITERS = 1,
   MAX_ITERS = 100000,
   MIN_CONC = 0,     // Minimum ion concentration (mM)
   MAX_CONC = 2000,  // Maximum ion concentration (mM)
   // Things that need colors
   SOLVENT=0,
   ATOM_K,
   ATOM_K_TRACK,
   ATOM_Na,
   ATOM_Na_TRACK,
   ATOM_Cl,
   ATOM_Cl_TRACK,
   MEMBRANE,
   PORE_K,
   PORE_Na,
   PORE_Cl,
   DIR_MASK = 0x7
};


struct atom
{
   int delta_x, delta_y;   // 4 bytes, 4 bytes
   uint8_t color;          // 1 byte
                           // ----------------
                           // 17 bytes
};

class NernstSim 
{
   public:
      NernstSim( struct options *options );
      void runSim();
      struct atom *world;
      unsigned long int direction_sz64;
      unsigned char *claimed;
      unsigned char *direction;
      struct options *o;
      int LRcharge;           // (publicRO) Net charge on left minus net charge on right
      int initLHS_K,  initRHS_K;	 //publicRO
      int initLHS_Na, initRHS_Na;
      int initLHS_Cl, initRHS_Cl;
      unsigned int *positionsLHS;
      unsigned int *positionsRHS;
      unsigned int *positionsPORES;
      unsigned long int idx( int x, int y );
      int ionCharge( unsigned int position );
      int isUntrackedAtom( unsigned int position );
      int isTrackedAtom( unsigned int position );
      void shufflePositions( struct options *o );
      void distributePores( struct options *o );
      void initAtoms( struct options *options );
      QTime *qtime;
      int rpt;	//cells per thread.
      void initNernstSim();
      void completeNernstSim();
      double elapsed;
      void moveAtoms_prep(void);
      void moveAtoms_stakeclaim(void);
      void moveAtoms_move(unsigned int start_idx=0, unsigned int end_idx=0);
      void moveAtoms_poretransport(void);


   protected:
      long maxatomsDefault;
      int currentIter;

      int preIter();
      void Iter();
      void postIter();
   private:
      void initWorld( struct options *o );
      int WORLD_SZ_MASK;
      unsigned int WORLD_SZ;
      unsigned int off_n, off_s, off_e, off_w, off_ne, off_nw, off_se, off_sw;
      unsigned int *dir2offset;
      int getX( unsigned int position );
      int getY( unsigned int position );
      int isMembrane( unsigned int position );
      int isSolvent( unsigned int position );
      int isPore( unsigned int position );
      int isAtom( unsigned int position );
      int isPermeable( unsigned int porePos, unsigned int ionPos );
      void copyAtom( unsigned int from, unsigned int to, int dx, int dy );
      int shouldTransport( unsigned int from, unsigned int to );
      void takeCensus( int iter );
      void finalizeAtoms(void);
      void moveAtoms(unsigned int start_idx=0, unsigned int end_idx=0);
};

#endif /* SIM_H */
