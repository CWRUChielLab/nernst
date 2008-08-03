/* util.h
 *
 */


#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#define BREADCRUMB fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__, __func__);
#ifdef __MINGW32__
#include <windows.h>
#define sleep(x) Sleep(x)
#endif


#ifdef __cplusplus
}
#endif

#endif /* UTIL_H */

