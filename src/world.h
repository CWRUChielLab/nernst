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

extern double e;     // Elementary charge (C)
extern double k;     // Boltzmann's constant (J K^-1)
extern double R;     // Molar gas constant (J K^-1 mol^-1)
extern double F;     // Faraday's constant (C mol^-1)
extern double t;     // Temperature (K)

extern double d;     // Length of a lattice square (m)
extern double a;     // Membrane area per lattice quare (m^2)
extern double eps0;  // Vacuum permittivity (C^2 J^-1 m^-1)
extern double eps;   // Membrane dielectric constant
extern double c;     // Membrane capacitance (F m^-2)


#ifdef __cplusplus
}
#endif

#endif /* WORLD_H */

