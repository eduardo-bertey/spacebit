/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * hash function                  *
 * ****************************** */

#ifndef _HASH_H
#define _HASH_H

#include "param.h"
#include "poly_q.h"
#include "poly_qbig.h"
#include "spend.h"

void hash(POLY_Q *x, const POLY_QBIG *a, const POLY_QBIG *b, const POLY_Q *c,
          const POLY_Q *d, const POLY_Q e_0[][N], const POLY_Q *f_0,
          const POLY_Q g_out[][N], const POLY_Q *s, const ACT *a_in,
          const POLY_Q pk_out[][N], const POLY_Q cn_out[][N]);

#endif
