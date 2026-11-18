/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * poly multiplications           *
 * ****************************** */

#ifndef _POLY_MULT_H
#define _POLY_MULT_H

#include "poly_q.h"
#include "poly_qbig.h"
#include <stdint.h>

void mult_plus_rq(POLY_Q *c, const POLY_Q *a, const POLY_Q *b);
void mult_minus_rq(POLY_Q *c, const POLY_Q *a, const POLY_Q *b);
void mult_plus_rqbig(POLY_QBIG *c, const POLY_QBIG *a, const POLY_QBIG *b);
void mult_plus_rqbig_pm(POLY_QBIG *c, const POLY_QBIG *a, const POLY_QBIG *b,
                        const uint64_t pm);
void mult_r(POLY_Q *out, const POLY_Q *a, const POLY_Q *b);
void mult_fxf(POLY_Q *out, const POLY_Q *f, const POLY_Q *x);

#endif
