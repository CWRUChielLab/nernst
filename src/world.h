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


#ifdef __cplusplus
}
#endif

#endif /* WORLD_H */

