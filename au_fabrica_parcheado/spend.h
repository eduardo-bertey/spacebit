/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * Spend                          *
 * ****************************** */

#ifndef _SPENT_H
#define _SPENT_H

#include "param.h"
#include "poly_q.h"
#include "poly_qbig.h"
#include <stdint.h>

typedef struct {
  POLY_Q pk[M_SPENT][N], cn[M_SPENT][N];
} ACT;

typedef struct {
  POLY_Q r[M], ck[M];
  uint64_t amt;
} ASK;

typedef struct {
  POLY_QBIG b[N_HAT];
  POLY_Q c[N];
  POLY_Q x;
  POLY_Q f0[BETA - 1], fr[M_SPENT * (R - 1) + S * R];
  POLY_Q zb[M_HAT], zc[M], z[M_SPENT + 1][M], z_out[S][M];
} SPEND_OUT;

void spend(POLY_Q cn_out[][N], POLY_Q *s, SPEND_OUT *out, const ACT *a_in,
           const uint64_t l, const ASK *ask, const POLY_Q pk_out[][N],
           uint64_t *amt_out, const POLY_Q g[][N], const POLY_Q *h,
           POLY_QBIG g_hat[][N_HAT], const POLY_QBIG *t0, const POLY_QBIG *t1,
           const POLY_QBIG *t2);

#endif
