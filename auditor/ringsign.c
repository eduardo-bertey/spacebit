/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * RingSign                       *
 * ****************************** */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "param.h"
#include "poly_mult.h"
#include "poly_q.h"
#include "poly_red.h"
#include "ringsign.h"
#include "sampleb.h"

/* we assume if H != NULL then GenSerial=True */
void ringsign(POLY_Q *rho, POLY_Q *e, POLY_Q *f, const POLY_Q p[][N],
              const POLY_Q *pp, const POLY_Q g[][N], const POLY_Q *h) {
  uint64_t i, j;
  POLY_Q r[M];

  /* rho_0<--{-B_{big},...,B_{big}}^{d*m} */
  if (h != NULL) {
    /* from Spend-I, when GenSerial=True, B_{big}=B_{big,k} */
    sample_bbig_md(rho);
  } else {
    /* from Spend-I, when GenSerial=False, B_{big}=B'_{big,k} */
    sample_bbig_k_prime_md(rho);
  }

  /* ntt */
  for (i = 0; i < M; i++) {
    for (j = 0; j < D; j++) {
      r[i].poly[j] = rho[i].poly[j] + Q;
    }

    ntt_q(r + i);
  }

  /* we simplify the implementation since k=1 */
  /* E_0=\sum_{i=0}^{N-1} p_{i,0}*P_i+Com_ck(0;rho_0) */

  /* Com_ck(0;rho_0) where ck=G */
  for (i = 0; i < N; i++) {
    memset(e + i, 0, sizeof(POLY_Q));
  }

  for (i = 0; i < M; i++) {
    for (j = 0; j < N; j++) {
      mult_plus_rq(e + j, g[i] + j, r + i);
    }
  }

  /* \sum_{i=0}^{N-1} p_{i,0}*P_i */
  for (i = 0; i < N_SPENT; i++) {
    for (j = 0; j < N; j++) {
      mult_plus_rq(e + j, pp + i, p[i] + j);
    }
  }

  for (i = 0; i < N; i++) {
    for (j = 0; j < D; j++) {
      e[i].poly[j] = red_short_q(e[i].poly[j]);
    }
  }

  if (h != NULL) {
    /* F_0=H*rho_0 */
    memset(f, 0, sizeof(POLY_Q));

    for (i = 0; i < M; i++) {
      mult_plus_rq(f, h + i, r + i);
    }

    for (i = 0; i < D; i++) {
      f->poly[i] = red_short_q(f->poly[i]);
    }
  }
}
