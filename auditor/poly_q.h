/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * ntt on R_q                     *
 * ****************************** */

#ifndef _POLY_Q
#define _POLY_Q

#include "poly_param.h"
#include <stdint.h>

typedef struct {
  uint64_t poly[D];
} POLY_Q;

void ntt_q(POLY_Q *a);
void intt_q(POLY_Q *a);
void mult_rq(POLY_Q *out, const POLY_Q *a, const POLY_Q *b);

#endif
