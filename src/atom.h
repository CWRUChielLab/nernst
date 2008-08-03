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
int getX( unsigned long int pos );
int getY( unsigned long int pos );
void initAtoms( struct options *options );
void moveAtoms( void );
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


#ifdef __cplusplus
}
#endif

#endif /* ATOM_H */

