/* options.h
 *
 */


#ifdef __cplusplus
extern "C" {
#endif


struct options
{
   // world options
                        // x & y dimensions of the world
   int x;               // -x[=512]
   int y;               // -y[=512]
   int iters;           // --iters[=50000]

   // atom options
   long max_atoms;      // --max-atoms[=LONG_MAX]
   int lspacing;        // --lspacing[=4]
   int rspacing;        // --rspacing[=12]
   int pores;           // --pores[=7]
   int selectivity;     // --selectivity[=1]
   int electrostatics;  // -- electrostatics[=1]

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
