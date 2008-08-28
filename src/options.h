/* options.h
 *
 */


#ifndef OPTIONS_H
#define OPTIONS_H
/*
#ifdef __cplusplus
extern "C" {
#endif
*/

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


struct options * parseOptions( int argc, char **argv );
void print_help( void );
void print_version( void );
void set_defaults( struct options *o );
void dump_options( struct options *o );
/*
#ifdef __cplusplus
}
#endif
*/
#endif /* OPTIONS_H */

