/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * ntt on R_{\hat{q}}             *
 * ****************************** */

#ifndef _POLY_QBIG
#define _POLY_QBIG

#include "poly_param.h"
#include <stdint.h>

typedef struct {
  uint64_t poly1[D];
  uint64_t poly2[D];
} POLY_QBIG;

void ntt_qbig(POLY_QBIG *a);
void intt_qbig(POLY_QBIG *a);
void mult_rqbig(POLY_QBIG *out, const POLY_QBIG *a, const POLY_QBIG *b);

#endif
