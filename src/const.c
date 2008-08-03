/* const.c
 *
 * Holds physical, chemical, and biological
 * constants.
 */


#include "const.h"


double e, k, R, F, t, d, a, eps0, eps, c;

void
initConst()
{
   static int initialized = 0;

   if( !initialized )
   {
      e = 1.60218e-19;     // Elementary charge (C)
      k = 1.38056e-23;     // Boltzmann's constant (J K^-1)
      R = 8.31447;         // Molar gas constant (J K^-1 mol^-1)
      F = 96485.3;         // Faraday's constant (C mol^-1)
      t = 373.15 * 0.8;    // Temperature (K)

      d = 3.5e-10;         // Length of a lattice square (m)
      a = d * d;           // Membrane area per lattice quare (m^2)
      eps0 = 8.85419e-12;  // Vacuum permittivity (F m^-1)
      // eps = 2.59764;       // Membrane dielectric constant
      eps = 50;            // Membrane dielectric constant
      c = eps * eps0 / d;  // Membrane capacitance (F m^-2)

   initialized = 1;
   }
}

