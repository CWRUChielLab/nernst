/* options.c
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
 * NOTES ON ADDING MORE OPTIONS:
 *
 * 1.  Put the option in "struct options" [options.h].
 * 2.  Add an appropriate default value in set_defaults().
 * 3.  Add a long and short form to the struct option long_options[] definition
 *        in parseOptions().
 * 4.  Add the short form to the getopt_long call in parseOptions.  
 * 5.  Add an entry based on the short form in the switch statement in 
 *        parseOptions().
 * 6.  If you're feeling particularly virtuous, update print_help().
 * 7.  Finally, update dump_options().
 */

//options that only take long-opt form should be indexed here.
//In order not to clash with single-letter options, start from
//'z'+1 (recall that 'z' > 'Z').
enum{
	OPT_ELEMENTARY_CHARGE = 'z'+1,
	OPT_BOLTZMAN,
	OPT_MOLAR_GAS,
	OPT_FARADAY,
	OPT_TEMPERATURE,
	OPT_LATTICE_LENGTH,
	OPT_MEMBRANE_AREA,
	OPT_VACUUM_PERM,
	OPT_MEMBRANE_DIELECTRIC,
	OPT_MEMBRANE_CAPACITACE,
	OPT_CBOLTZ,
	OPT_NUM_OPTIONS_THAT_ONLY_TAKE_LONG_FORM	//bleah.
};	

#ifdef BLR_USEMAC
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#include <assert.h>
#include <stdlib.h>  // strtol()
#include <limits.h>
#include <stdio.h>
#include <errno.h>
// g++ defines _GNU_SOURCE on the command line.
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <getopt.h>
#include <time.h>
#include <string.h>

#include "options.h"
#include "world.h"
#include "safecalls.h"
using namespace SafeCalls;
const char
*version[] =
{
   "Nernst potential simulator.",
   "(C) 2008  Jeffrey Gill, Barry Rountree, Kendrick Shaw, Catherine Kehl,",
   "          Jocelyn Eckert, and Dr. Hillel J. Chiel",
   "",
   "Version 1.0.0-blr1",
   "Released under the GPL version 3 or any later version.",
   "This is free software; see the source for copying conditions. There is NO",
   "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.",
   NULL
};


const char
*help[] =
{
   "-a, --max-atoms           Use no more than this number of atoms, even if",
   "                             the spacing scheme will accomodate more.",
   "-A, --pK                  Permeability of K. Default=1.0.",
   "-B, --pNa                 Permeability of Na. Default=0.04.",
   "-C, --pCl                 Permeability of Cl. Default=0.45.",
   "-e, --no-electrostatics   Turn off electrostatics.",
   "-g, --no-gui              Don't use the GUI.",
   "-h, --help                Display this information.",
   "-i, --iters               Number of iterations. Default=50000.",
   "-l, --sleep               Seconds between each iteration (for debugging).",
   "                             Default=0.",
   "-L, --lK                  Concentration of K in LHS in mM. Default=400.",
   "-M, --lNa                 Concentration of Na in LHS in mM. Default=50.",
   "-N, --lCl                 Concentration of Cl in LHS in mM. Default=52.",
   "-o, --pores               Number of pores in the membrane. Default=12.",
   "-p, --profiling           Output performance information and some",
   "                             settings.",
   "-P, --progress            Print the percentage complete periodically.",
   "-r, --randseed            Random number seed (integer). Default is set by",
   "                             system time.",
   "-R, --rK                  Concentration of K in RHS in mM. Default=20.",
   "-s, --no-selectivity      Turn off pore selectivity.",
   "-S, --rNa                 Concentration of Na in RHS in mM. Default=440.",
   "-t, --threads             Number of threads per machine.  Not yet",
   "                             implemented.",
   "-T, --rCl                 Concentration of Cl in RHS in mM. Default=560.",
   "-v, --verbose             Print debugging information (occasionally",
   "                             implemented).",
   "-V, --version             Print version information.",
   "-x, --x                   Horizontal world size.  Must be a power of 2.",
   "                             Default=256.",
   "-y, --y                   Vertical world size.  Must be a power of 2.",
   "                             Default=256.",
   "",
   "--elementary-charge        Charge of one proton, (C).         (1.60218e-19)",
   "--boltzmann                Boltzmann's constant (J K^-1).     (1.38056e-23)",
   "--molar-gas                Molar gas constant (C mol^-1).     (8.31447)",
   "--faraday                  Faraday's constant (C mol^-1).     (96485.3)",
   "--temperature              Temperature (K).                   (298)",
   "--lattice-length           Length of a lattice square (m).    (3.5e-10)",
   "--membrane-area            Membrane area per lattice (m^2).   ((3.5e-10)^2)",
   "--vacuum-perm              Vacuum permittivity (F m^-1)       (8.85419e-12)",
   "--membrane-dielectric      Membrane dielectric (unitless)     (250)",
   "--membrane-capacitance     Membrane capacitance (F m^-2)      (see doc)", //FIXME
   "--cboltz                   Constant used in Boltzmann coef    (see doc)",
   NULL
};
/*
int 
safeStrtol( char *str )
{
   //Handle error checking on strtol.
   long int val;
   char *endptr;
   errno = 0;
   val = strtol( str, &endptr, 10 );
   char msg[1024];

   if( ( errno == ERANGE && ( val == LONG_MAX || val == LONG_MIN ) ) || ( errno != 0 && val == 0 ) )
   {
      perror( "strtol" );
      exit( EXIT_FAILURE );
   }

   if( endptr == str )
   {
      fprintf( stderr, "No digits were found\n" );
      exit( EXIT_FAILURE );
   }

   return val;
}
*/

void
print_help()
{
   fprintf( stderr, "---------------------------------------------------------------------------\n" );
   int i = 0;
   while( help[ i ] != NULL )
   {
      fprintf( stderr, "%s\n", help[ i ] );
      i++;
   }
   fprintf( stderr, "---------------------------------------------------------------------------\n" );
   exit(0);
}


void
print_version()
{
   fprintf( stderr, "---------------------------------------------------------------------------\n" );
   int i = 0;
   while( version[ i ] != NULL )
   {
      fprintf( stderr, "%s\n", version[ i ] );
      i++;
   }
   fprintf( stderr, "Compiled at %s on %s.\n", __TIME__, __DATE__ );
   fprintf( stderr, "---------------------------------------------------------------------------\n" );
}


void
set_defaults( struct options *o )
{
   o->x              = 256;
   o->y              = 256;
   o->iters          = 50000;

   o->max_atoms      = LONG_MAX;
   o->lK             = 400;
   o->lNa            = 50;
   o->lCl            = 52;
   o->rK             = 20;
   o->rNa            = 440;
   o->rCl            = 560;
   o->pK             = 1.0;
   o->pNa            = 0.04;
   o->pCl            = 0.45;
   o->selectivity    = 1;
   o->electrostatics = 1;

   o->use_gui        = 1;
   o->sleep          = 0;

   o->randseed       = time( NULL );
   o->verbose        = 0;
   o->help           = 0;
   o->version        = 0;
   o->threads        = 1;

   o->profiling      = 0;
   o->progress       = 0;

   o->e 	= 1.60218e-19;     // Elementary charge (C)
   o->k 	= 1.38056e-23;     // Boltzmann's constant (J K^-1)
   o->R 	= 8.31447;         // Molar gas constant (J K^-1 mol^-1)
   o->F 	= 96485.3;         // Faraday's constant (C mol^-1)
   o->t 	= 298;             // Temperature (K)
   o->d 	= 3.5e-10;         // Length of a lattice square (m)
   o->a 	= o->d * o->d;           // Membrane area per lattice quare (m^2)
   o->eps0 	= 8.85419e-12;  // Vacuum permittivity (F m^-1)
   o->eps 	= 250;           // Membrane dielectric constant
   o->c 	= o->eps * o->eps0 / o->d;  // Membrane capacitance (F m^-2)
   o->cBoltz 	= o->e * o->e / ( 2 * o->k * o->t * o->c * o->a ); //used in Boltzmann calc
}


void
dump_options( struct options *o )
{
   print_version();

   fprintf( stderr, "x =              %d\n", o->x );
   fprintf( stderr, "y =              %d\n", o->y );
   fprintf( stderr, "iters =          %d\n", o->iters );

   fprintf( stderr, "max_atoms =      %ld\n", o->max_atoms );
   fprintf( stderr, "lK =             %d\n", o->lK );
   fprintf( stderr, "lNa =            %d\n", o->lNa );
   fprintf( stderr, "lCl =            %d\n", o->lCl );
   fprintf( stderr, "rK =             %d\n", o->rK );
   fprintf( stderr, "rNa =            %d\n", o->rNa );
   fprintf( stderr, "rCl =            %d\n", o->rCl );
   fprintf( stderr, "pK =             %f\n", o->pK );
   fprintf( stderr, "pNa =            %f\n", o->pNa );
   fprintf( stderr, "pCl =            %f\n", o->pCl );
   fprintf( stderr, "selectivity =    %d\n", o->selectivity );
   fprintf( stderr, "electrostatics = %d\n", o->electrostatics );

   fprintf( stderr, "use_gui =        %d\n", o->use_gui );
   fprintf( stderr, "sleep =          %d\n", o->sleep );

   fprintf( stderr, "randseed =       %d\n", o->randseed );
   fprintf( stderr, "verbose =        %d\n", o->verbose );
   fprintf( stderr, "help =           %d\n", o->help );
   fprintf( stderr, "version =        %d\n", o->version );
   fprintf( stderr, "threads =        %d\n", o->threads );

   fprintf( stderr, "progress =       %d\n", o->progress );
   fprintf( stderr, "profiling =      %d\n", o->profiling );
   fprintf( stderr, "---------------------------------------------------------------------------\n" );
   fprintf( stderr, "elementary-charge     %lf\n", o->e		);
   fprintf( stderr, "boltzmann		 %lf\n", o->k		);
   fprintf( stderr, "molar-gas             %lf\n", o->R		);
   fprintf( stderr, "faraday               %lf\n", o->F		);
   fprintf( stderr, "temperature	         %lf\n", o->t		);
   fprintf( stderr, "lattice-length        %lf\n", o->d		);
   fprintf( stderr, "membrane-area         %lf\n", o->a		);
   fprintf( stderr, "vacuum-perm           %lf\n", o->eps0	);
   fprintf( stderr, "membrane-dielectric   %lf\n", o->eps		);
   fprintf( stderr, "membrane-capacitance  %lf\n", o->c		);
   fprintf( stderr, "cboltz                %lf\n", o->cBoltz 	);

   fprintf( stderr, "---------------------------------------------------------------------------\n" );
}


struct options *
parseOptions(int argc, char **argv)
{

   // This is gratefully stolen, as always, from the sample code found in man -S3 getopt.
   int option_index = 0, c;
   struct option long_options[] =
   {
   // { "long_option_name", "noarg(0), requiredarg(1), optarg(2)", NULL, retval }

   // Keep this ordered by short opt name.
      { "max-atoms",         1, 0, 'a' },
      { "pK",                1, 0, 'A' },
      { "pNa",               1, 0, 'B' },
      { "pCl",               1, 0, 'C' },
      { "no-electrostatics", 0, 0, 'e' },
      { "no-gui",            0, 0, 'g' },
      { "help",              0, 0, 'h' },
      { "iters",             1, 0, 'i' },
      { "sleep",             1, 0, 'l' },
      { "lK",                1, 0, 'L' },
      { "lNa",               1, 0, 'M' },
      { "lCl",               1, 0, 'N' },
      { "profiling",         0, 0, 'p' },
      { "progress",          0, 0, 'P' },
      { "randseed",          1, 0, 'r' },
      { "rK",                1, 0, 'R' },
      { "no-selectivity",    0, 0, 's' },
      { "rNa",               1, 0, 'S' },
      { "threads",           1, 0, 't' },
      { "rCl",               1, 0, 'T' },
      { "verbose",           0, 0, 'v' },
      { "version",           0, 0, 'V' },
      { "x",                 1, 0, 'x' },
      { "y",                 1, 0, 'y' },
      { "elementary-charge",    	1, 0, OPT_ELEMENTARY_CHARGE},
      { "boltzmann",		 	1, 0, OPT_BOLTZMAN},
      { "molar-gas",            	1, 0, OPT_MOLAR_GAS},
      { "faraday",              	1, 0, OPT_FARADAY},
      { "temperature",	        	1, 0, OPT_TEMPERATURE},
      { "lattice-length",       	1, 0, OPT_LATTICE_LENGTH},
      { "membrane-area",        	1, 0, OPT_MEMBRANE_AREA},
      { "vacuum-perm",          	1, 0, OPT_VACUUM_PERM},
      { "membrane-dielectric",  	1, 0, OPT_MEMBRANE_DIELECTRIC},
      { "membrane-capacitance", 	1, 0, OPT_MEMBRANE_CAPACITACE},
      { "cboltz",               	1, 0, OPT_CBOLTZ},
      { 0,                   0, 0,  0  }
   };

   struct options *options = (struct options *)malloc( sizeof( struct options ) );

   assert( options );
   set_defaults( options );
   while( 1 )
   {
      c = getopt_long( argc, argv, "a:A:B:C:eghi:l:L:M:N:p::Pr:R:sS:t:T:vVx:y:", long_options, &option_index );
      if( c == -1 )
      {
         break;
      }

      switch( c )
      {
         case 'a':
            options->max_atoms = safeStrtol( optarg );
            break;
         case 'A':
            options->pK = safeStrtol( optarg );
            break;
         case 'B':
            options->pNa = safeStrtol( optarg );
            break;
         case 'C':
            options->pCl = safeStrtol( optarg );
            break;
         case 'e':
            options->electrostatics = 0;
            break;
         case 'g':
            options->use_gui = 0;
            break;
         case 'h':
            print_help();
            exit( 0 );
            break;
         case 'i':
            options->iters = safeStrtol( optarg );
            break;
         case 'l':
            options->sleep = safeStrtol( optarg );
            break;
         case 'L':
            options->lK = safeStrtol( optarg );
            break;
         case 'M':
            options->lNa = safeStrtol( optarg );
            break;
         case 'N':
            options->lCl = safeStrtol( optarg );
            break;
         case 'p':
            // When running the application on a Mac through Finder or "open"
            // in Terminal, an extra option is added that looks something like
            // "-psn_0_58333".  This is the process serial number.  We don't
            // have any use for it yet.  If optarg is nonzero (indicating a
            // value is present), then ignore it.  Otherwise, turn on
            // profiling.
            if(!optarg){
                    options->profiling = 1;
            }
            break;
         case 'P':
            options->progress = 1;
            break;
         case 'r':
            options->randseed = safeStrtol( optarg );
            break;
         case 'R':
            options->rK = safeStrtol( optarg );
            break;
         case 's':
            options->selectivity = 0;
            break;
         case 'S':
            options->rNa = safeStrtol( optarg );
            break;
         case 't':
            options->threads = safeStrtol( optarg );
            break;
         case 'T':
            options->rCl = safeStrtol( optarg );
            break;
         case 'v':
            options->verbose = 1;
            break;
         case 'V':
            print_version();
            exit( 0 );
            break;
         case 'x':
            options->x = safeStrtol( optarg );
            break;
         case 'y':
            options->y = safeStrtol( optarg );
            break;
	 case OPT_ELEMENTARY_CHARGE:
            options->e	=safeStrtod( optarg );
	    break;
	 case OPT_BOLTZMAN:
            options->k	=safeStrtod( optarg );
	    break;
	 case OPT_MOLAR_GAS:
            options->R	=safeStrtod( optarg );
	    break;
	 case OPT_FARADAY:
            options->F	=safeStrtod( optarg );
	    break;
	 case OPT_TEMPERATURE:
            options->t	=safeStrtod( optarg );
	    break;
	 case OPT_LATTICE_LENGTH:
            options->d	=safeStrtod( optarg );
	    break;
	 case OPT_MEMBRANE_AREA:
            options->a	=safeStrtod( optarg );
	    break;
	 case OPT_VACUUM_PERM:
            options->eps0	=safeStrtod( optarg );
	    break;
	 case OPT_MEMBRANE_DIELECTRIC:
            options->eps	=safeStrtod( optarg );
	    break;
	 case OPT_MEMBRANE_CAPACITACE:
            options->c	=safeStrtod( optarg );
	    break;
	 case OPT_CBOLTZ:
            options->cBoltz = safeStrtod( optarg );
	    break;
         default:
            fprintf( stderr, "Unknown option.  Try --help for a full list.\n" );
            exit( -1 );
            break;
      }
   }

   if( options->verbose )
   {
      dump_options( options );
   }
   return options;
}

