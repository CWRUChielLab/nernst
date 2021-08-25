/* util.h
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


#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#define BREADCRUMB fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__, __func__);
#define ASSERT( test ) if( !(test) ) {fprintf(stderr, "Assertion failed:%s\n%s:%d:%s\n", #test, __FILE__, __LINE__, __func__);exit(-1);}
#ifdef __MINGW32__
#include <windows.h>
#define sleep(x) Sleep(x)
#endif


#ifdef __cplusplus
}
#endif

#endif /* UTIL_H */

