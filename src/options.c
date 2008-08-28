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
#define _GNU_SOURCE
#include <getopt.h>
#include <time.h>
#include <string.h>

#include "options.h"
#include "world.h"


char
*version[] =
{
   "Nernst potential simulator.",
   "(C) 2008  Jeffrey Gill, Barry Rountree, Kendrick Shaw, Catherine Kehl,",
   "          Jocelyn Eckert, and Dr. Hillel J. Chiel",
   "",
   "Version 1.0.0",
   "Released under the GPL version 3 or any later version.",
   "This is free software; see the source for copying conditions. There is NO",
   "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.",
   NULL
};


char
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
   NULL
};


int 
safe_strtol( char *str )
{
   //Handle error checking on strtol.
   long int val;
   char *endptr;
   errno = 0;
   val = strtol( str, &endptr, 10 );

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
}


struct options *
parseOptions(int argc, char **argv)
{
   // When running the application on a Mac through Finder or "open" in Terminal, an extra option is
   // added that looks something like "-psn_0_58333".  This option needs to be removed when we parse
   // our options, so here we duplicate argc and argv, removing the unwanted argument.
   char **argv2 = malloc( sizeof( char* ) * argc );
   int i, rmParams, argc2;
   for( i = 0, rmParams = 0, argc2 = argc; i < argc; i++ )
   {
      if( strncmp( argv[ i ], "-psn", 4 ) != 0 )
      {
         argv2[ i - rmParams ] = malloc( sizeof( char ) * ( strlen( argv[ i ] ) + 1 ) );
         strcpy( argv2[ i - rmParams ], argv[ i ] );
      } else {
         argc2--;
         rmParams++;
      }
   }

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
      { 0,                   0, 0,  0  }
   };

   struct options *options = malloc( sizeof( struct options ) );

   assert( options );
   set_defaults( options );
   while( 1 )
   {
      c = getopt_long( argc2, argv2, "a:A:B:C:eghi:l:L:M:N:pPr:R:sS:t:T:vVx:y:", long_options, &option_index );
      if( c == -1 )
      {
         break;
      }

      switch( c )
      {
         case 'a':
            options->max_atoms = safe_strtol( optarg );
            break;
         case 'A':
            options->pK = safe_strtol( optarg );
            break;
         case 'B':
            options->pNa = safe_strtol( optarg );
            break;
         case 'C':
            options->pCl = safe_strtol( optarg );
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
            options->iters = safe_strtol( optarg );
            break;
         case 'l':
            options->sleep = safe_strtol( optarg );
            break;
         case 'L':
            options->lK = safe_strtol( optarg );
            break;
         case 'M':
            options->lNa = safe_strtol( optarg );
            break;
         case 'N':
            options->lCl = safe_strtol( optarg );
            break;
         case 'p':
            options->profiling = 1;
            break;
         case 'P':
            options->progress = 1;
            break;
         case 'r':
            options->randseed = safe_strtol( optarg );
            break;
         case 'R':
            options->rK = safe_strtol( optarg );
            break;
         case 's':
            options->selectivity = 0;
            break;
         case 'S':
            options->rNa = safe_strtol( optarg );
            break;
         case 't':
            options->threads = safe_strtol( optarg );
            break;
         case 'T':
            options->rCl = safe_strtol( optarg );
            break;
         case 'v':
            options->verbose = 1;
            break;
         case 'V':
            print_version();
            exit( 0 );
            break;
         case 'x':
            options->x = safe_strtol( optarg );
            break;
         case 'y':
            options->y = safe_strtol( optarg );
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

