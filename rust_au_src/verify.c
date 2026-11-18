/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * Verify                         *
 * ****************************** */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gnorm.h"
#include "hash.h"
#include "param.h"
#include "poly_mult.h"
#include "poly_q.h"
#include "poly_qbig.h"
#include "poly_red.h"
#include "spend.h"
#include "verify.h"

uint64_t verify(const ACT *a_in, const POLY_Q pk_out[][N],
                const POLY_Q cn_out[][N], const SPEND_OUT *in, const POLY_Q *s,
                const POLY_Q g[][N], const POLY_Q *h, POLY_QBIG g_hat[][N_HAT],
                const POLY_QBIG *t0, const POLY_QBIG *t1, const POLY_QBIG *t2) {
  static POLY_Q f00;
  static POLY_Q g_norm[BETA + M_SPENT * (R - 1) + S * R];
  static POLY_QBIG f_prime_ntt[BETA + M_SPENT * (R - 1) + S * R];
  static POLY_QBIG g_ntt[BETA + M_SPENT * (R - 1) + S * R];
  static POLY_QBIG zb_ntt[M_HAT];
  static POLY_QBIG x_ntt_qbig;
  static POLY_Q x_ntt_q;
  static POLY_QBIG a[N_HAT];
  static POLY_Q fc[R + 1], fc_ntt[R], zc_ntt[M], f_out_ntt[S * R],
      z_out_ntt[S][M];
  static POLY_Q d[N];
  static POLY_Q z_ntt[M_SPENT + 1][M], e[M_SPENT + 1][N], f[M_SPENT];
  static POLY_Q f0_ntt[BETA];
  static POLY_Q cn_out_sum[N], p[N_SPENT][N];
  static POLY_Q x;
  static POLY_Q g_out[S][N];

  uint64_t i, j, k;
  int64_t tmp, e_norm = 0;

  /* PARCHE imagen-cero: un serial todo cero es invalido (h degenerado) */
  {
    uint64_t s_nz = 0, mi, ki;
    for (mi = 0; mi < M_SPENT; mi++)
      for (ki = 0; ki < D; ki++)
        s_nz |= (uint64_t)(s[mi].poly[ki] != 0);
    if (!s_nz)
      return 0;
  }

  /* compare ||f||_{\infty} with B_a-p */
  for (i = 0; i < BETA - 1; i++) {
    for (j = 0; j < D; j++) {
      tmp = in->f0[i].poly[j];

      if ((tmp > BAP) || (tmp < -BAP)) {
        return 0;
      }
    }
  }

  /* compare ||f_r||_{\infty} with B_r-p */
  for (i = 0; i < M_SPENT * (R - 1) + S * R; i++) {
    for (j = 0; j < D; j++) {
      tmp = in->fr[i].poly[j];

      if ((tmp > BRP) || (tmp < -BRP)) {
        return 0;
      }
    }
  }

  /* when k=1, f_{0,0}=x-\sum_{i=1}^{beta-1} f_{0,i} */
  /* compare ||f_{0,0}|| with B_a*sqrt(d*(beta-1)) */
  for (j = 0; j < D; j++) {
    tmp = in->x.poly[j];

    for (i = 0; i < BETA - 1; i++) {
      tmp -= in->f0[i].poly[j];
    }

    e_norm += tmp * tmp;

    if (e_norm > F00_NORM) {
      return 0;
    }

    f00.poly[j] = tmp;
  }

  /* compute g and compare ||g|| with T_g */
  if (check_gnorm(g_norm, &f00, in->f0, in->fr, &(in->x))) {
    return 0;
  }

  /* compare ||z_b||_{\infty} with \hat{B_{big}}-Bpw */
  for (i = 0; i < M_HAT; i++) {
    for (j = 0; j < D; j++) {
      tmp = in->zb[i].poly[j];

      if ((tmp > ZB_NORM) || (tmp < -ZB_NORM)) {
        return 0;
      }
    }
  }

  /* compare ||z_c,z_{out,0},z_{out,1}||_{\infty} with B_{big,k}-Bpw */
  for (i = 0; i < M; i++) {
    for (j = 0; j < D; j++) {
      tmp = in->zc[i].poly[j];

      if ((tmp > ZC_NORM) || (tmp < -ZC_NORM)) {
        return 0;
      }

      tmp = in->z_out[0][i].poly[j];

      if ((tmp > ZC_NORM) || (tmp < -ZC_NORM)) {
        return 0;
      }

      tmp = in->z_out[1][i].poly[j];

      if ((tmp > ZC_NORM) || (tmp < -ZC_NORM)) {
        return 0;
      }
    }
  }

  /* when k=1, compare ||z^(i)||_{\infty} with B_{big,k}-Bpw for i<M */
  for (i = 0; i < M; i++) {
    for (j = 0; j < D; j++) {
      tmp = in->z[0][i].poly[j];

      if ((tmp > Z0_NORM) || (tmp < -Z0_NORM)) {
        return 0;
      }

#if M_SPENT == 2
      tmp = in->z[1][i].poly[j];

      if ((tmp > Z0_NORM) || (tmp < -Z0_NORM)) {
        return 0;
      }
#endif
    }
  }

  /* when k=1, compare ||z^(M)||_{\infty} with B'_{big,k}-(M+S+1)*Bpw */
  for (i = 0; i < M; i++) {
    for (j = 0; j < D; j++) {
      tmp = in->z[M_SPENT][i].poly[j];

      if ((tmp > ZM_NORM) || (tmp < -ZM_NORM)) {
        return 0;
      }
    }
  }

  /* ntt */
  for (i = 0; i < D; i++) {
    x_ntt_qbig.poly1[i] = QBIG1 + in->x.poly[i];
    x_ntt_qbig.poly2[i] = QBIG2 + in->x.poly[i];

    x_ntt_q.poly[i] = Q + in->x.poly[i];
  }

  ntt_qbig(&x_ntt_qbig);
  ntt_q(&x_ntt_q);

  for (i = 0; i < D; i++) {
    f_prime_ntt[0].poly1[i] = QBIG1 + f00.poly[i];
    f_prime_ntt[0].poly2[i] = QBIG2 + f00.poly[i];

    f0_ntt[0].poly[i] = f00.poly[i] + Q;
  }

  ntt_qbig(f_prime_ntt);
  ntt_q(f0_ntt);

  for (i = 1; i < BETA; i++) {
    for (j = 0; j < D; j++) {
      f_prime_ntt[i].poly1[j] = QBIG1 + in->f0[i - 1].poly[j];
      f_prime_ntt[i].poly2[j] = QBIG2 + in->f0[i - 1].poly[j];

      f0_ntt[i].poly[j] = in->f0[i - 1].poly[j] + Q;
    }

    ntt_qbig(f_prime_ntt + i);
    ntt_q(f0_ntt + i);
  }

  for (i = 0; i < M_SPENT * (R - 1) + S * R; i++) {
    for (j = 0; j < D; j++) {
      f_prime_ntt[BETA + i].poly1[j] = QBIG1 + in->fr[i].poly[j];
      f_prime_ntt[BETA + i].poly2[j] = QBIG2 + in->fr[i].poly[j];
    }

    ntt_qbig(f_prime_ntt + BETA + i);
  }

  for (i = 0; i < BETA + M_SPENT * (R - 1) + S * R; i++) {
    for (j = 0; j < D; j++) {
      g_ntt[i].poly1[j] = (QBIG + g_norm[i].poly[j]) % QBIG1;
      g_ntt[i].poly2[j] = (QBIG + g_norm[i].poly[j]) % QBIG2;
    }

    ntt_qbig(g_ntt + i);
  }

  for (i = 0; i < M_HAT; i++) {
    for (j = 0; j < D; j++) {
      zb_ntt[i].poly1[j] = QBIG1 + in->zb[i].poly[j];
      zb_ntt[i].poly2[j] = QBIG2 + in->zb[i].poly[j];
    }

    ntt_qbig(zb_ntt + i);
  }

  /* add the trapdoor */
  if ((t0 != NULL) && (t1 != NULL) && (t2 != NULL)) {
    for (i = 0; i < M_HAT; i++) {
      memcpy(g_hat[i] + N_HAT - 1, t0 + i, sizeof(POLY_QBIG));
    }

    for (i = 0; i < V_1; i++) {
      memcpy(g_hat[M_HAT + i] + N_HAT - 1, t1 + i, sizeof(POLY_QBIG));
    }

    for (i = 0; i < V_2; i++) {
      memcpy(g_hat[M_HAT + V_1 + i] + N_HAT - 1, t2 + i, sizeof(POLY_QBIG));
    }
  }

  /* A=Com_ck(f',g;z_b)-xB where ck=\hat{G} */
  for (i = 0; i < N_HAT; i++) {
    memset(a + i, 0, sizeof(POLY_QBIG));
  }

  for (i = 0; i < M_HAT; i++) {
    for (j = 0; j < N_HAT; j++) {
      mult_plus_rqbig(a + j, g_hat[i] + j, zb_ntt + i);
    }
  }

  for (i = 0; i < BETA + M_SPENT * (R - 1) + S * R; i++) {
    for (j = 0; j < N_HAT; j++) {
      mult_plus_rqbig(a + j, g_hat[M_HAT + i] + j, f_prime_ntt + i);
    }
  }

  for (i = 0; i < BETA + M_SPENT * (R - 1) + S * R; i++) {
    for (j = 0; j < N_HAT; j++) {
      mult_plus_rqbig(a + j,
                      g_hat[M_HAT + BETA + M_SPENT * (R - 1) + S * R + i] + j,
                      g_ntt + i);
    }
  }

  for (i = 0; i < N_HAT; i++) {
    mult_plus_rqbig_pm(a + i, &x_ntt_qbig, in->b + i, 1);
  }

  for (i = 0; i < N_HAT; i++) {
    for (j = 0; j < D; j++) {
      a[i].poly1[j] = red_short_qbig1(a[i].poly1[j]);
      a[i].poly2[j] = red_short_qbig2(a[i].poly2[j]);
    }
  }

/* f_{c,i}-2*f_{c,i+1} */
/* ntt */
#if M_SPENT == 1
  for (i = 0; i < D; i++) {
    fc_ntt[0].poly[i] = Q - 2 * in->fr[0].poly[i];
  }
  ntt_q(fc_ntt);

  for (i = 1; i < R - 1; i++) {
    for (j = 0; j < D; j++) {
      fc_ntt[i].poly[j] = in->fr[i - 1].poly[j] - 2 * in->fr[i].poly[j] + Q;
    }
    ntt_q(fc_ntt + i);
  }

  for (i = 0; i < D; i++) {
    fc_ntt[R - 1].poly[i] = in->fr[R - 2].poly[i] + Q;
  }
  ntt_q(fc_ntt + R - 1);
#else
  memset(fc, 0, sizeof(POLY_Q));
  memset(fc + R, 0, sizeof(POLY_Q));
  for (i = 0; i < R - 1; i++) {
    for (j = 0; j < D; j++) {
      fc[i + 1].poly[j] = in->fr[i + R - 1].poly[j] - in->fr[i].poly[j];
    }
  }

  for (i = 0; i < R; i++) {
    for (j = 0; j < D; j++) {
      fc_ntt[i].poly[j] = fc[i].poly[j] - 2 * fc[i + 1].poly[j] + Q;
    }
    ntt_q(fc_ntt + i);
  }
#endif

  for (i = 0; i < S * R; i++) {
    for (j = 0; j < D; j++) {
      f_out_ntt[i].poly[j] = in->fr[M_SPENT * (R - 1) + i].poly[j] + Q;
    }

    ntt_q(f_out_ntt + i);
  }

  for (i = 0; i < M; i++) {
    for (j = 0; j < D; j++) {
      zc_ntt[i].poly[j] = Q + in->zc[i].poly[j];

      z_out_ntt[0][i].poly[j] = Q + in->z_out[0][i].poly[j];
      z_out_ntt[1][i].poly[j] = Q + in->z_out[1][i].poly[j];

      z_ntt[0][i].poly[j] = in->z[0][i].poly[j] + Q;
      z_ntt[1][i].poly[j] = in->z[1][i].poly[j] + Q;

#if M_SPENT == 2
      z_ntt[2][i].poly[j] = in->z[2][i].poly[j] + Q;
#endif
    }

    ntt_q(zc_ntt + i);

    ntt_q(z_out_ntt[0] + i);
    ntt_q(z_out_ntt[1] + i);

    ntt_q(z_ntt[0] + i);
    ntt_q(z_ntt[1] + i);

#if M_SPENT == 2
    ntt_q(z_ntt[2] + i);
#endif
  }

  /* D=Com_ck(f_{c,0}-2f_{c,1},...,f_{c,r-1}-2f_{c,r};z_c)-xC and
   * G_i=Com_ck(f_{out,0}^(i),...,f_{out,r-1}^(i);z_{out,i})-xcn_{out,i} where
   * ck=G */
  for (i = 0; i < N; i++) {
    memset(d + i, 0, sizeof(POLY_Q));

    memset(g_out[0] + i, 0, sizeof(POLY_Q));
    memset(g_out[1] + i, 0, sizeof(POLY_Q));
  }

  for (i = 0; i < M; i++) {
    for (j = 0; j < N; j++) {
      mult_plus_rq(d + j, g[i] + j, zc_ntt + i);

      mult_plus_rq(g_out[0] + j, g[i] + j, z_out_ntt[0] + i);
      mult_plus_rq(g_out[1] + j, g[i] + j, z_out_ntt[1] + i);
    }
  }

  for (i = 0; i < R; i++) {
    for (j = 0; j < N; j++) {
      mult_plus_rq(d + j, g[M + i] + j, fc_ntt + i);

      mult_plus_rq(g_out[0] + j, g[M + i] + j, f_out_ntt + i);
      mult_plus_rq(g_out[1] + j, g[M + i] + j, f_out_ntt + R + i);
    }
  }

  for (i = 0; i < N; i++) {
    mult_minus_rq(d + i, &x_ntt_q, in->c + i);

    mult_minus_rq(g_out[0] + i, &x_ntt_q, cn_out[0] + i);
    mult_minus_rq(g_out[1] + i, &x_ntt_q, cn_out[1] + i);
  }

  for (i = 0; i < N; i++) {
    for (j = 0; j < D; j++) {
      d[i].poly[j] = red_short_q(d[i].poly[j]);

      g_out[0][i].poly[j] = red_short_q(g_out[0][i].poly[j]);
      g_out[1][i].poly[j] = red_short_q(g_out[1][i].poly[j]);
    }
  }

  /* E_0^(j)=\sum_{i=0}^{N-1} f_{0,i}*pk_{i,j}-Com_ck(0;z^(j)) for j<M where
   * ck=G */
  for (j = 0; j < N; j++) {
    memset(e[0] + j, 0, sizeof(POLY_Q));

    for (i = 0; i < BETA; i++) {
      mult_plus_rq(e[0] + j, f0_ntt + i, a_in[i].pk[0] + j);
    }
  }

  for (i = 0; i < M; i++) {
    for (j = 0; j < N; j++) {
      mult_minus_rq(e[0] + j, g[i] + j, z_ntt[0] + i);
    }
  }

  for (i = 0; i < N; i++) {
    for (j = 0; j < D; j++) {
      e[0][i].poly[j] = red_short_q(e[0][i].poly[j]);
    }
  }

#if M_SPENT == 2
  for (j = 0; j < N; j++) {
    memset(e[1] + j, 0, sizeof(POLY_Q));

    for (i = 0; i < BETA; i++) {
      mult_plus_rq(e[1] + j, f0_ntt + i, a_in[i].pk[1] + j);
    }
  }

  for (i = 0; i < M; i++) {
    for (j = 0; j < N; j++) {
      mult_minus_rq(e[1] + j, g[i] + j, z_ntt[1] + i);
    }
  }

  for (i = 0; i < N; i++) {
    for (j = 0; j < D; j++) {
      e[1][i].poly[j] = red_short_q(e[1][i].poly[j]);
    }
  }
#endif

  /* when k=1, F_0^(i)=x*s_i-H*z^(i) for i<M */
  mult_rq(f, &x_ntt_q, s);

  for (i = 0; i < M; i++) {
    mult_minus_rq(f, h + i, z_ntt[0] + i);
  }

  for (i = 0; i < D; i++) {
    f[0].poly[i] = red_short_q(f[0].poly[i]);
  }

#if M_SPENT == 2
  mult_rq(f + 1, &x_ntt_q, s + 1);

  for (i = 0; i < M; i++) {
    mult_minus_rq(f + 1, h + i, z_ntt[1] + i);
  }

  for (i = 0; i < D; i++) {
    f[1].poly[i] = red_short_q(f[1].poly[i]);
  }
#endif

  /* when S=2, P_j=cn_{out,0}+cn_{out,1}-sum(cn_{i,j})+C for i<=M */
  for (i = 0; i < N; i++) {
    for (j = 0; j < D; j++) {
      cn_out_sum[i].poly[j] =
          cn_out[0][i].poly[j] + cn_out[1][i].poly[j] + in->c[i].poly[j];
    }
  }

  for (i = 0; i < N_SPENT; i++) {
    for (j = 0; j < N; j++) {
      for (k = 0; k < D; k++) {
#if M_SPENT == 1
        p[i][j].poly[k] =
            red_short_q(cn_out_sum[j].poly[k] - a_in[i].cn[0][j].poly[k] + Q);
#else
        p[i][j].poly[k] =
            red_short_q(cn_out_sum[j].poly[k] - a_in[i].cn[0][j].poly[k] -
                        a_in[i].cn[1][j].poly[k] + (Q << 1));
#endif
      }
    }
  }

  /* when k=1, E_0^(M)=\sum_{i=0}^{N-1} f_{0,i}*P_{j}-Com_ck(0;z^(1)) where ck=G
   */
  for (j = 0; j < N; j++) {
    memset(e[M_SPENT] + j, 0, sizeof(POLY_Q));

    for (i = 0; i < BETA; i++) {
      mult_plus_rq(e[M_SPENT] + j, f0_ntt + i, p[i] + j);
    }
  }

  for (i = 0; i < M; i++) {
    for (j = 0; j < N; j++) {
      mult_minus_rq(e[M_SPENT] + j, g[i] + j, z_ntt[M_SPENT] + i);
    }
  }

  for (i = 0; i < N; i++) {
    for (j = 0; j < D; j++) {
      e[M_SPENT][i].poly[j] = red_short_q(e[M_SPENT][i].poly[j]);
    }
  }

  /* compute and compare hash values */
  hash(&x, a, in->b, in->c, d, e, f, g_out, s, a_in, pk_out, cn_out);

  for (i = 0; i < D; i++) {
    if (x.poly[i] != in->x.poly[i]) {
      return 0;
    }
  }

  return 1;
}
