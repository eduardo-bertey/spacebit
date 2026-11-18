/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * GenTdRow                       *
 * ****************************** */

#ifndef _GENTDROW_H
#define _GENTDROW_H

#include "param.h"
#include "poly_qbig.h"

void gentdrow(POLY_QBIG *t0, POLY_QBIG *t1, POLY_QBIG *t2, POLY_QBIG *td,
              const POLY_QBIG g_hat[][N_HAT]);

#endif
