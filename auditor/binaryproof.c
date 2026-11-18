/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * BinaryProof                    *
 * ****************************** */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "binaryproof.h"
#include "param.h"
#include "poly_mult.h"
#include "poly_q.h"
#include "poly_qbig.h"
#include "poly_red.h"
#include "sampleb.h"

/* from Spend-I, v is constant 2*(beta+r-1+S*r) */
void binaryproof(POLY_Q *ra, POLY_Q *rb, POLY_QBIG *a, POLY_QBIG *b,
                 BINARYPROOF_IN *in, const POLY_QBIG g_hat[][N_HAT]) {
  POLY_QBIG ra_qbig[M_HAT], rb_qbig[M_HAT];

  uint64_t bits[V_HAT >> 1];
  uint64_t *bits_head = bits;

  POLY_QBIG a_ntt[V_HAT >> 1], d_ntt[V_HAT >> 1];
  POLY_QBIG *a_ntt_head = a_ntt, *d_ntt_head = d_ntt;

  uint64_t i, j, k, i_star;
  uint64_t tmp, mask;

  POLY_Q tmp_poly;

  /* r_b<--{-B,...,B}^{d*\hat{m}},
   * r_a<--{-\hat{B_{big}},...,\hat{B_{big}}}^{d*\hat{m}} */
  sample_b_mhatd(rb);
  sample_bbig_hat_mhatd(ra);

  /* ntt */
  for (i = 0; i < M_HAT; i++) {
    for (j = 0; j < D; j++) {
      ra_qbig[i].poly1[j] = QBIG1 + ra[i].poly[j];
      ra_qbig[i].poly2[j] = QBIG2 + ra[i].poly[j];

      rb_qbig[i].poly1[j] = rb[i].poly[j] + QBIG1;
      rb_qbig[i].poly2[j] = rb[i].poly[j] + QBIG2;
    }

    ntt_qbig(ra_qbig + i);
    ntt_qbig(rb_qbig + i);
  }

  for (j = 0; j < T; j++) {
    /* if Bool_j=True, B_j=B_a and s_j=beta from Spend-I */
    if (in[j].boo) {
      memset(in[j].a, 0, sizeof(POLY_Q));

      /* if b_0^(j)=1, i*<--{1,...,beta-1} */
      tmp = sample_beta();
      i_star = ((in[j].bits[0]) | (-in[j].bits[0])) & tmp;

      for (i = 1; i < in[j].s; i++) {
        sample_ba(in[j].a + i);
        sample_bap(&tmp_poly);

        /* if i != i* and b_i^(j)=0, a_{i}^(j)<--{-(B_a-p),...,(B_a-p)}^d;
         * otherwise a_{i}^(j)<--{-B_a,...,B_a}^d */
        mask = -((((i - i_star) | (i_star - i)) &
                  ((1LL << 63) ^ ((in[j].bits[i]) | (-in[j].bits[i])))) >>
                 63);
        for (k = 0; k < D; k++) {
          in[j].a[i].poly[k] ^= mask & (tmp_poly.poly[k] ^ in[j].a[i].poly[k]);

          /* a_0^(j)=-\sum_{i=1}^{beta-1} a_i^(j) */
          in[j].a[0].poly[k] -= in[j].a[i].poly[k];
        }
      }
    } else {
      /* if Bool_j=False, B_j=B_r from Spend-I */
      for (i = 0; i < in[j].s; i++) {
        /* a_i^(j)<--{-B_r,...,B_r}^d */
        sample_br(in[j].a + i);
      }
    }
  }

  /* ntt */
  for (i = 0; i < T; i++) {
    memcpy(bits_head, in[i].bits, sizeof(uint64_t) * in[i].s);
    bits_head += in[i].s;

    for (j = 0; j < in[i].s; j++) {
      for (k = 0; k < D; k++) {
        a_ntt_head->poly1[k] = in[i].a[j].poly[k] + QBIG1;
        a_ntt_head->poly2[k] = in[i].a[j].poly[k] + QBIG2;
      }

      ntt_qbig(a_ntt_head);
      mult_rqbig(d_ntt_head, a_ntt_head, a_ntt_head);

      a_ntt_head++;
      d_ntt_head++;
    }
  }

  /* B=Com_ck(b,c;r_b) and A=Com_ck(a,d;r_a) where ck=\hat{G} */
  for (i = 0; i < N_HAT; i++) {
    memset(b + i, 0, sizeof(POLY_QBIG));
    memset(a + i, 0, sizeof(POLY_QBIG));
  }

  for (i = 0; i < M_HAT; i++) {
    for (j = 0; j < N_HAT; j++) {
      mult_plus_rqbig(b + j, g_hat[i] + j, rb_qbig + i);
      mult_plus_rqbig(a + j, g_hat[i] + j, ra_qbig + i);
    }
  }

  for (i = 0; i < (V_HAT >> 1); i++) {
    for (j = 0; j < N_HAT; j++) {
      for (k = 0; k < D; k++) {
        b[j].poly1[k] += (-bits[i]) & g_hat[M_HAT + i][j].poly1[k];
        b[j].poly2[k] += (-bits[i]) & g_hat[M_HAT + i][j].poly2[k];
      }

      mult_plus_rqbig(a + j, g_hat[M_HAT + i] + j, a_ntt + i);
    }
  }

  for (i = 0; i < (V_HAT >> 1); i++) {
    for (j = 0; j < N_HAT; j++) {
      mult_plus_rqbig_pm(b + j, g_hat[M_HAT + (V_HAT >> 1) + i] + j, a_ntt + i,
                         bits[i]);
      mult_plus_rqbig_pm(a + j, g_hat[M_HAT + (V_HAT >> 1) + i] + j, d_ntt + i,
                         1);
    }
  }

  for (i = 0; i < N_HAT; i++) {
    for (j = 0; j < D; j++) {
      b[i].poly1[j] = red_short_qbig1(b[i].poly1[j]);
      b[i].poly2[j] = red_short_qbig2(b[i].poly2[j]);

      a[i].poly1[j] = red_short_qbig1(a[i].poly1[j]);
      a[i].poly2[j] = red_short_qbig2(a[i].poly2[j]);
    }
  }
}
