/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * RingSign                       *
 * ****************************** */

#ifndef _RINGSIGN_H
#define _RINGSIGN_H

#include "param.h"
#include "poly_q.h"

void ringsign(POLY_Q *rho, POLY_Q *e, POLY_Q *f, const POLY_Q p[][N],
              const POLY_Q *pp, const POLY_Q g[][N], const POLY_Q *h);

#endif
