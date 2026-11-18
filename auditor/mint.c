/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * Mint                           *
 * ****************************** */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "mint.h"
#include "param.h"
#include "poly_mult.h"
#include "poly_q.h"
#include "poly_red.h"
#include "sampleb.h"

void mint(POLY_Q *cn, POLY_Q *ck, uint64_t amt, const POLY_Q g[][N]) {
  uint64_t b[R];
  uint64_t i, j, k;
  POLY_Q r[M];

  /* r<--{-B,...,B}^{d*m} */
  sample_b_md(ck);

  /* ntt */
  for (i = 0; i < M; i++) {
    for (j = 0; j < D; j++) {
      r[i].poly[j] = ck[i].poly[j] + Q;
    }

    ntt_q(r + i);
  }

  /* C=Com_ck(b_0,...,b_{r-1};r) where ck=G */
  for (i = 0; i < N; i++) {
    memset(cn + i, 0, sizeof(POLY_Q));
  }

  for (i = 0; i < M; i++) {
    for (j = 0; j < N; j++) {
      mult_plus_rq(cn + j, g[i] + j, r + i);
    }
  }

  /* (b_0,...,b_{r-1})<--Bin(amt) */
  for (i = 0; i < R; i++) {
    b[i] = amt & 1;
    amt >>= 1;
  }

  for (i = 0; i < R; i++) {
    for (j = 0; j < N; j++) {
      for (k = 0; k < D; k++) {
        cn[j].poly[k] += (-b[i]) & g[M + i][j].poly[k];
      }
    }
  }

  for (i = 0; i < N; i++) {
    for (j = 0; j < D; j++) {
      cn[i].poly[j] = red_short_q(cn[i].poly[j]);
    }
  }
}
