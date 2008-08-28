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

	// constants
   double e;		//= 1.60218e-19;     // Elementary charge (C)
   double k;		//= 1.38056e-23;     // Boltzmann's constant (J K^-1)
   double R;		//= 8.31447;         // Molar gas constant (J K^-1 mol^-1)
   double F;		//= 96485.3;         // Faraday's constant (C mol^-1)
   double t;		//= 298;             // Temperature (K)
   double d;		//= 3.5e-10;         // Length of a lattice square (m)
   double a;		//= d * d;           // Membrane area per lattice quare (m^2)
   double eps0;		//= 8.85419e-12;  // Vacuum permittivity (F m^-1)
   double eps;		//= 250;           // Membrane dielectric constant
   double c;		//= eps * eps0 / d;  // Membrane capacitance (F m^-2)
   double cBoltz;	//= e * e / ( 2 * k * t * c * a ); // used in Boltzmann calculation
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

