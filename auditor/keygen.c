/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * KeyGen                         *
 * ****************************** */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "keygen.h"
#include "param.h"
#include "poly_mult.h"
#include "poly_q.h"
#include "poly_red.h"
#include "sammat.h"
#include "sampleb.h"

void keygen(POLY_Q *pk, POLY_Q *sk, const POLY_Q g[][N]) {
  POLY_Q r[M];

  uint64_t i, j;

  /* r<--{-B,...,B}^{d*m} */
  sample_b_md(sk);

  /* ntt */
  for (i = 0; i < M; i++) {
    for (j = 0; j < D; j++) {
      r[i].poly[j] = sk[i].poly[j] + Q;
    }

    ntt_q(r + i);
  }

  /* c=G*r */
  for (i = 0; i < N; i++) {
    memset(pk + i, 0, sizeof(POLY_Q));
  }

  for (i = 0; i < M; i++) {
    for (j = 0; j < N; j++) {
      mult_plus_rq(pk + j, g[i] + j, r + i);
    }
  }

  for (i = 0; i < N; i++) {
    for (j = 0; j < D; j++) {
      pk[i].poly[j] = red_short_q(pk[i].poly[j]);
    }
  }
}
