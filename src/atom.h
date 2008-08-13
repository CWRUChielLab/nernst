/* atom.h
 *
 */


#ifndef ATOM_H
#define ATOM_H

#ifdef __cplusplus
extern "C" {
#endif

extern int nAtoms;
extern int WORLD_T;
extern unsigned int WORLD_COUNTER;
extern int LRcharge;
extern int initLHS_K, initRHS_K, initLHS_Cl, initRHS_Cl;
// extern struct atom *atom;

struct atom
{
   int delta_x, delta_y;   // 4 bytes, 4 bytes
   uint8_t color;          // 1 byte
                           // ----------------
                           // 17 bytes
};

unsigned long int  idx( int x, int y );
int getX( unsigned int position );
int getY( unsigned int position );
int ionCharge( uint8_t type );
int isMembrane( unsigned int position );
int isPore( unsigned int position );
int isPermeable( uint8_t type );
void initAtoms( struct options *options );
void moveAtoms( void );
void redistributePores( void );
void finalizeAtoms( void );
void takeCensus( int iter );


// Things that need colors
enum
{
   SOLVENT,
   ATOM_K,
   ATOM_Cl,
   MEMBRANE
};

enum
{
   DIR_MASK = 0x7
};


#ifdef __cplusplus
}
#endif

#endif /* ATOM_H */

