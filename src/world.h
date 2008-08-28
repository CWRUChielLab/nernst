/* world.h
 * 
 */


#ifndef WORLD_H
#define WORLD_H

#ifdef __cplusplus
extern "C" {
#endif


extern unsigned long int direction_sz64;
extern struct atom *world;
extern unsigned char *claimed;
extern unsigned char *direction;
void initWorld( struct options *o );

enum
{
   MIN_X = 16,
   MIN_Y = 16,
   MAX_X = 1024,
   MAX_Y = 1024
};

enum
{
   MIN_ITERS = 1,
   MAX_ITERS = 100000
};

enum
{
   MIN_CONC = 0,     // Minimum ion concentration (mM)
   MAX_CONC = 2000   // Maximum ion concentration (mM)
};



#ifdef __cplusplus
}
#endif

#endif /* WORLD_H */

