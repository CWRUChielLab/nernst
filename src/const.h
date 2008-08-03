/* const.h
 *
 */


#ifndef CONST_H
#define CONST_H 

#ifdef __cplusplus
extern "C" {
#endif


void initConst( void );

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

#endif /* CONST_H */

