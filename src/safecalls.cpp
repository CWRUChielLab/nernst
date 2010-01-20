/* safecalls.cpp
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


#include <cerrno>
#include <stdio.h>
#include "safecalls.h"

namespace SafeCalls{
long int
_safeStrtol( const char *str, const char* file, const int line ) {
	//Handle error checking on strtol.
	long int val;
	char *endptr;
	char msg[1024];
	errno = 0;
	val = strtol( str, &endptr, 10 );

	if( errno ){

		snprintf(msg, 1023, "strtol failed at %s::%d", file, line);
		perror( msg );
		exit( EXIT_FAILURE );

	}else if( endptr == str ) {

		fprintf( stderr, "No digits were found.  %s::%d.\n", file, line );
		exit( EXIT_FAILURE );
	}

	return val;
}

double
_safeStrtod( const char *str, const char* file, const int line ){
	//Handle error checking on strtol.
	double val;
	char *endptr;
	char msg[1024];
	errno = 0;
	val = strtod( str, &endptr );

	if( errno ){

		snprintf(msg, 1023, "strtod failed at %s::%d", file, line);
		perror( msg );
		exit( EXIT_FAILURE );

	}else if( endptr == str ) {

		fprintf( stderr, "No digits were found.  %s::%d.\n", file, line );
		exit( EXIT_FAILURE );
	}

	return val;
}
}
