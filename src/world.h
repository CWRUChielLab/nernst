/* world.h
 * 
 */


#ifdef __cplusplus
extern "C" {
#endif


enum
{
   WORLD_X = 128,//512,
   WORLD_Y = 1024 //512
};

extern unsigned long int direction_sz64;
extern struct atom *world;
extern unsigned char *claimed;
extern unsigned char *direction;
void initWorld( void );


#ifdef __cplusplus
}
#endif
