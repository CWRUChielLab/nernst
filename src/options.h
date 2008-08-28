/* options.h
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


#ifndef OPTIONS_H
#define OPTIONS_H

#ifdef __cplusplus
extern "C" {
#endif


struct options
{
   // world options
                        // x & y dimensions of the world
   int x;               // -x[=256]
   int y;               // -y[=256]
   int iters;           // --iters[=50000]

   // atom options
   long max_atoms;      // --max-atoms[=LONG_MAX]
   int lK;              // --lK[=400]
   int lNa;             // --lNa[=50]
   int lCl;             // --lCl[=52]
   int rK;              // --rK[=20]
   int rNa;             // --rNa[=440]
   int rCl;             // --rCl[=560]
   double pK;           // --pK[=1.0]
   double pNa;          // --pNa[=0.04]
   double pCl;          // --pCl[=0.45]
   int selectivity;     // --selectivity[=1]
   int electrostatics;  // --electrostatics[=1]

   // gui options
   int use_gui;         // --[no-]gui
   int sleep;           // --sleep[=0]    How many seconds to sleep
                        //                between each iteration.

   // housekeeping
   int randseed;
   int verbose;
   int help;
   int version;
   int threads;         // --threads[=1]

   // runtime options
   int profiling;
   int progress;
};


int safe_strtol( char *str );
struct options * parseOptions( int argc, char **argv );
void print_help( void );
void print_version( void );
void set_defaults( struct options *o );
void dump_options( struct options *o );

#ifdef __cplusplus
}
#endif

#endif /* OPTIONS_H */

