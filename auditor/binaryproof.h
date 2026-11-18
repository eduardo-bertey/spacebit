/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * BinaryProof                    *
 * ****************************** */

#ifndef _BINARYPROOF_H
#define _BINARYPROOF_H

#include "param.h"
#include "poly_q.h"
#include "poly_qbig.h"
#include <stdint.h>

typedef struct {
  uint64_t s, boo;
  uint64_t *bits;
  POLY_Q *a;
} BINARYPROOF_IN;

void binaryproof(POLY_Q *ra, POLY_Q *rb, POLY_QBIG *a, POLY_QBIG *b,
                 BINARYPROOF_IN *in, const POLY_QBIG g_hat[][N_HAT]);

#endif
