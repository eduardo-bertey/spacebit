/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * Spend                          *
 * ****************************** */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "binaryproof.h"
#include "comp.h"
#include "gnorm.h"
#include "hash.h"
#include "mint.h"
#include "param.h"
#include "poly_mult.h"
#include "poly_q.h"
#include "poly_qbig.h"
#include "poly_red.h"
#include "ringsign.h"
#include "sampleb.h"
#include "serialgen.h"
#include "spend.h"

void spend(POLY_Q cn_out[][N], POLY_Q *s, SPEND_OUT *out, const ACT *a_in,
           const uint64_t l, const ASK *ask, const POLY_Q pk_out[][N],
           uint64_t *amt_out, const POLY_Q g[][N], const POLY_Q *h,
           POLY_QBIG g_hat[][N_HAT], const POLY_QBIG *t0, const POLY_QBIG *t1,
           const POLY_QBIG *t2) {
  static uint64_t c_bits_in[R + 1], c_bits_out[R + 1], c_bits[R + 1],
      c_bits_ntt[R];
  static POLY_Q ck_out[S][M];
  static uint64_t delta[BETA];
  static BINARYPROOF_IN binaryproof_in[T];
  static POLY_Q a0[BETA], ac_in[R + 1], ac_out[R + 1], ac[R + 1], a_out[S * R];
  static uint64_t amt_out_bin[S][R];
  static POLY_Q ra[M_HAT], rb[M_HAT];
  static POLY_QBIG a[N_HAT];
  static POLY_Q rc[M], rd[M];
  static POLY_Q rc_ntt[M], rd_ntt[M];
  static POLY_Q ac_ntt[R];
  static POLY_Q d[N];
  static POLY_Q p[N_SPENT][N], p_cn[N_SPENT][N], cn_out_sum[N];
  static POLY_Q pp[N_SPENT];
  static POLY_Q pk_concat[M_SPENT][N_SPENT][N];
  static POLY_Q rho[M_SPENT + 1][M], e[M_SPENT + 1][N], f[M_SPENT];
  static POLY_Q f00;
  static POLY_Q rm_c[M], rm;
  static POLY_Q g_norm[BETA + M_SPENT * (R - 1) + S * R];
  static POLY_Q r_g[S][M], r_g_ntt[S][M], g_out[S][N], a_out_ntt[S * R];

  uint64_t i, j, k, rej, e_norm;
  int64_t tmp;

  /* pk_{i,0},...,pk_{i,N-1} for i<M */
  for (i = 0; i < N_SPENT; i++) {
    for (j = 0; j < N; j++) {
      memcpy(pk_concat[0][i] + j, a_in[i].pk[0] + j, sizeof(POLY_Q));

#if M_SPENT == 2
      memcpy(pk_concat[1][i] + j, a_in[i].pk[1] + j, sizeof(POLY_Q));
#endif
    }
  }

  for (i = 0; i < R; i++) {
    amt_out_bin[0][i] = (amt_out[0] >> i) & 0x1;
    amt_out_bin[1][i] = (amt_out[1] >> i) & 0x1;
  }

  c_bits[0] = 0;
  c_bits[R] = 0;

#if M_SPENT == 1
  /* for M=1 and S=2,
   * c_{i+1}=(c_i+amt_{out,0}[i]+amt_{out,1}[i]-amt_{in,0}[i])/2 */
  for (i = 0; i < R - 1; i++) {
    c_bits[i + 1] = (c_bits[i] + amt_out_bin[0][i] + amt_out_bin[1][i] -
                     ((ask[0].amt >> i) & 0x1)) >>
                    1;
  }
#else
  /* for M=2 and S=2, c_{in,i+1}=(c_{in,i}+amt_{in,0}[i]+amt_{in,1}[i])/2,
   * c_{out,i+1}=(c_{out,i}+amt_{out,0}[i]+amt_{out,1}[i])/2, and
   * c_i=c_{out,i}-c_{in,i} */
  c_bits_in[0] = 0;
  c_bits_in[R] = 0;
  c_bits_out[0] = 0;
  c_bits_out[R] = 0;

  for (i = 0; i < R - 1; i++) {
    c_bits_in[i + 1] = (c_bits_in[i] + ((ask[0].amt >> i) & 0x1) +
                        ((ask[1].amt >> i) & 0x1)) >>
                       1;
    c_bits_out[i + 1] =
        (c_bits_out[i] + amt_out_bin[0][i] + amt_out_bin[1][i]) >> 1;
    c_bits[i + 1] = c_bits_out[i + 1] - c_bits_in[i + 1];
  }
#endif

  /* compute c_{i}-2*c_{i+1} in Montgomery form */
  for (i = 0; i < R; i++) {
    c_bits_ntt[i] = montgomery_q(c_bits[i] - 2 * c_bits[i + 1] + Q,
                                 MONTGOMERY_CONVERT_FACTOR_Q);
  }

  /* (cn_{out,i},ck_{out,i})<--Mint(amt_{out,i}) */
  mint(cn_out[0], ck_out[0], amt_out[0], g);
  mint(cn_out[1], ck_out[1], amt_out[1], g);

  /* for S=2, compute cn_{out,0}+cn_{out,1}-sum(cn_{i,j}) for i<=M,j=0,...,N-1
   * (determined part of P_j) */
  for (i = 0; i < N; i++) {
    for (j = 0; j < D; j++) {
      cn_out_sum[i].poly[j] = cn_out[0][i].poly[j] + cn_out[1][i].poly[j];
    }
  }

  for (i = 0; i < N_SPENT; i++) {
    for (j = 0; j < N; j++) {
      for (k = 0; k < D; k++) {
#if M_SPENT == 1
        p_cn[i][j].poly[k] =
            cn_out_sum[j].poly[k] - a_in[i].cn[0][j].poly[k] + Q;
#else
        p_cn[i][j].poly[k] = cn_out_sum[j].poly[k] - a_in[i].cn[0][j].poly[k] -
                             a_in[i].cn[1][j].poly[k] + (Q << 1);
#endif
      }
    }
  }

  /* for S=2, compute r_{out,0}+r_{out,1}-sum(ck_{0,l}) for i<=M (determined
   * part of r_{M,l}) */
  for (i = 0; i < M; i++) {
    for (j = 0; j < D; j++) {
#if M_SPENT == 1
      rm_c[i].poly[j] =
          ck_out[0][i].poly[j] + ck_out[1][i].poly[j] - ask[0].ck[i].poly[j];
#else
      rm_c[i].poly[j] = ck_out[0][i].poly[j] + ck_out[1][i].poly[j] -
                        ask[0].ck[i].poly[j] - ask[1].ck[i].poly[j];
#endif
    }
  }

  for (i = 0; i < BETA; i++) {
    delta[i] = 1 ^ (((i - l) | (l - i)) >> 63);
  }

  memset(ac, 0, sizeof(POLY_Q));
  memset(ac + R, 0, sizeof(POLY_Q));

#if M_SPENT == 2
  memset(ac_in, 0, sizeof(POLY_Q));
  memset(ac_in + R, 0, sizeof(POLY_Q));
  memset(ac_out, 0, sizeof(POLY_Q));
  memset(ac_out + R, 0, sizeof(POLY_Q));
#endif

  /* BinaryProof input b */
  /* (beta,True,(delta_{l_j,0},...,delta_{l_j,beta-1}),B_a) */
  binaryproof_in[0].s = BETA;
  binaryproof_in[0].boo = 1;
  binaryproof_in[0].bits = delta;
  binaryproof_in[0].a = a0;

#if M_SPENT == 1
  /* (r-1,False,(c_1,...,c_{r-1}),B_r) */
  binaryproof_in[1].s = R - 1;
  binaryproof_in[1].boo = 0;
  binaryproof_in[1].bits = c_bits + 1;
  binaryproof_in[1].a = ac + 1;

  /* (r,False,Bin(amt_{out,j}),B_r) */
  binaryproof_in[2].s = R;
  binaryproof_in[2].boo = 0;
  binaryproof_in[2].bits = amt_out_bin[0];
  binaryproof_in[2].a = a_out;

  binaryproof_in[3].s = R;
  binaryproof_in[3].boo = 0;
  binaryproof_in[3].bits = amt_out_bin[1];
  binaryproof_in[3].a = a_out + R;
#else
  /* (r-1,False,(c_{in,1},...,c_{in,r-1}),B_r) */
  binaryproof_in[1].s = R - 1;
  binaryproof_in[1].boo = 0;
  binaryproof_in[1].bits = c_bits_in + 1;
  binaryproof_in[1].a = ac_in + 1;

  /* (r-1,False,(c_{out,1},...,c_{out,r-1}),B_r) */
  binaryproof_in[2].s = R - 1;
  binaryproof_in[2].boo = 0;
  binaryproof_in[2].bits = c_bits_out + 1;
  binaryproof_in[2].a = ac_out + 1;

  /* (r,False,Bin(amt_{out,j}),B_r) */
  binaryproof_in[3].s = R;
  binaryproof_in[3].boo = 0;
  binaryproof_in[3].bits = amt_out_bin[0];
  binaryproof_in[3].a = a_out;

  binaryproof_in[4].s = R;
  binaryproof_in[4].boo = 0;
  binaryproof_in[4].bits = amt_out_bin[1];
  binaryproof_in[4].a = a_out + R;
#endif

  /* s_i=SerialGen(r_{i,l}) for i<M */
  serialgen(s, ask[0].r, h);

#if M_SPENT == 2
  serialgen(s + 1, ask[1].r, h);
#endif

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

  do {
    rej = 0;

    /* rejection from here */
    /* r_c<--{-B,...,B}^{d*m}, r_d<--{-B_{big},...,B_{big}}^{d*m},
     * r_g^(i)<--{-B_{big},...,B_{big}}^{d*m} */
    sample_b_md(rc);
    sample_bbig_md(rd);

    sample_bbig_md(r_g[0]);
    sample_bbig_md(r_g[1]);

    /* (r_a,r_b),(A,B),(a_{0,0},...,a_{k-1,beta-1},a_{c,1},a_{c,r-1},a_{out,0}^(0),...,a_{out,r-1}^{S-1})<--BinaryProof(k+1+S,b,B,B_{big})
     */
    binaryproof(ra, rb, a, out->b, binaryproof_in, g_hat);

    /* ntt */
    for (i = 0; i < M; i++) {
      for (j = 0; j < D; j++) {
        rc_ntt[i].poly[j] = rc[i].poly[j] + Q;
        rd_ntt[i].poly[j] = rd[i].poly[j] + Q;

        r_g_ntt[0][i].poly[j] = r_g[0][i].poly[j] + Q;
        r_g_ntt[1][i].poly[j] = r_g[1][i].poly[j] + Q;
      }

      ntt_q(rc_ntt + i);
      ntt_q(rd_ntt + i);

      ntt_q(r_g_ntt[0] + i);
      ntt_q(r_g_ntt[1] + i);
    }

    /* C=Com_ck(c_0-2c_1,...,c_{r-1}-2c_r;r_c),
     * D=Com_ck(a_{c,0}-2a_{c,1},...,a_{c,r-1}-2a_{c,r};r_d), and
     * G_i=Com_ck(a_{out,0}^(i),...,a_{out,r-1}^(i);r_g^(i)) where ck=G */
    for (i = 0; i < N; i++) {
      memset(out->c + i, 0, sizeof(POLY_Q));
      memset(d + i, 0, sizeof(POLY_Q));

      memset(g_out[0] + i, 0, sizeof(POLY_Q));
      memset(g_out[1] + i, 0, sizeof(POLY_Q));
    }

    for (i = 0; i < M; i++) {
      for (j = 0; j < N; j++) {
        mult_plus_rq(out->c + j, g[i] + j, rc_ntt + i);
        mult_plus_rq(d + j, g[i] + j, rd_ntt + i);

        mult_plus_rq(g_out[0] + j, g[i] + j, r_g_ntt[0] + i);
        mult_plus_rq(g_out[1] + j, g[i] + j, r_g_ntt[1] + i);
      }
    }

#if M_SPENT == 2
    for (i = 0; i < R - 1; i++) {
      for (j = 0; j < D; j++) {
        ac[i + 1].poly[j] = ac_out[i + 1].poly[j] - ac_in[i + 1].poly[j];
      }
    }
#endif

    for (i = 0; i < R; i++) {
      for (j = 0; j < D; j++) {
        ac_ntt[i].poly[j] = ac[i].poly[j] - 2 * ac[i + 1].poly[j] + Q;

        a_out_ntt[i].poly[j] = a_out[i].poly[j] + Q;
        a_out_ntt[i + R].poly[j] = a_out[i + R].poly[j] + Q;
      }

      ntt_q(ac_ntt + i);

      ntt_q(a_out_ntt + i);
      ntt_q(a_out_ntt + i + R);
    }

    for (i = 0; i < R; i++) {
      for (j = 0; j < N; j++) {
        for (k = 0; k < D; k++) {
          out->c[j].poly[k] += montgomery_q(g[M + i][j].poly[k], c_bits_ntt[i]);
        }

        mult_plus_rq(d + j, g[M + i] + j, ac_ntt + i);

        mult_plus_rq(g_out[0] + j, g[M + i] + j, a_out_ntt + i);
        mult_plus_rq(g_out[1] + j, g[M + i] + j, a_out_ntt + i + R);
      }
    }

    for (i = 0; i < N; i++) {
      for (j = 0; j < D; j++) {
        out->c[i].poly[j] = red_short_q(out->c[i].poly[j]);
        d[i].poly[j] = red_short_q(d[i].poly[j]);

        g_out[0][i].poly[j] = red_short_q(g_out[0][i].poly[j]);
        g_out[1][i].poly[j] = red_short_q(g_out[1][i].poly[j]);
      }
    }

    /* compute P_j */
    for (i = 0; i < N_SPENT; i++) {
      for (j = 0; j < N; j++) {
        for (k = 0; k < D; k++) {
          p[i][j].poly[k] = red_short_q(p_cn[i][j].poly[k] + out->c[j].poly[k]);
        }
      }
    }

    /* when k=1, p_{0,0},...,p_{N-1,0}=a_{0,0},...,a_{0,beta-1} */
    for (i = 0; i < N_SPENT; i++) {
      for (j = 0; j < D; j++) {
        pp[i].poly[j] = a0[i].poly[j] + Q;
      }

      ntt_q(pp + i);
    }

    /* when k=1,
     * rho_0^(i),E_0^(i),F_0^(i)<--RingSign(True,(pk_{i,0}...pk_{i,N-1}),p,B,B_{big,k})
     * for i<M
     * rho_0^(M),E_0^(M),F_0^(M)<--RingSign(False,(P_0,...,P_{N-1}),p,B,B'_{big,k})
     */
    ringsign(rho[0], e[0], f, pk_concat[0], pp, g, h);

#if M_SPENT == 2
    ringsign(rho[1], e[1], f + 1, pk_concat[1], pp, g, h);
#endif

    ringsign(rho[M_SPENT], e[M_SPENT], NULL, p, pp, g, NULL);

    /* x=Hash */
    hash(&(out->x), a, out->b, out->c, d, e, f, g_out, s, a_in, pk_out, cn_out);

    /* norm check */

    /* f_{j,i}=x*delta_{l_j,i}+a_{j,i} with inf norm <= B_a-p for i=1,...,beta-1
     */
    for (i = 1; i < BETA; i++) {
      for (j = 0; j < D; j++) {
        tmp = ((-delta[i]) & out->x.poly[j]) + a0[i].poly[j];

        rej |= ct_lt_s64(BAP, tmp) | ct_lt_s64(tmp, -BAP);

        out->f0[i - 1].poly[j] = tmp;
      }
    }

    if (rej) {
      continue;
    }

    /* f_{j,0} with euclidean norm <= B_a*sqrt(d*(beta-1)) */
    e_norm = 0;
    for (i = 0; i < D; i++) {
      tmp = ((-delta[0]) & out->x.poly[i]) + a0[0].poly[i];

      e_norm += tmp * tmp;

      f00.poly[i] = tmp;
    }

    rej = (F00_NORM - e_norm) >> 63;
    if (rej) {
      continue;
    }

    /* f_{c,i}=x*c_i+a_{c,i}, f_{out,i}^(j)=x*amt_{out,j}[i]+a_{out,i}^(j), with
     * inf norm <= B_r-p */
    for (i = 0; i < R - 1; i++) {
      for (j = 0; j < D; j++) {
#if M_SPENT == 1
        tmp = ((-c_bits[i + 1]) & out->x.poly[j]) + ac[i + 1].poly[j];

        rej |= ct_lt_s64(BRP, tmp) | ct_lt_s64(tmp, -BRP);

        out->fr[i].poly[j] = tmp;
#else
        tmp = ((-c_bits_in[i + 1]) & out->x.poly[j]) + ac_in[i + 1].poly[j];

        rej |= ct_lt_s64(BRP, tmp) | ct_lt_s64(tmp, -BRP);

        out->fr[i].poly[j] = tmp;

        tmp = ((-c_bits_out[i + 1]) & out->x.poly[j]) + ac_out[i + 1].poly[j];

        rej |= ct_lt_s64(BRP, tmp) | ct_lt_s64(tmp, -BRP);

        out->fr[i + R - 1].poly[j] = tmp;
#endif
      }
    }

    if (rej) {
      continue;
    }

    for (i = 0; i < R; i++) {
      for (j = 0; j < D; j++) {
        tmp = ((-amt_out_bin[0][i]) & out->x.poly[j]) + a_out[i].poly[j];

        rej |= ct_lt_s64(BRP, tmp) | ct_lt_s64(tmp, -BRP);

        out->fr[i + M_SPENT * (R - 1)].poly[j] = tmp;

        tmp = ((-amt_out_bin[1][i]) & out->x.poly[j]) + a_out[i + R].poly[j];

        rej |= ct_lt_s64(BRP, tmp) | ct_lt_s64(tmp, -BRP);

        out->fr[i + M_SPENT * (R - 1) + R].poly[j] = tmp;
      }
    }

    if (rej) {
      continue;
    }

    /* compute g and check ||g||<=T_g */
    rej = check_gnorm(g_norm, &f00, out->f0, out->fr, &(out->x));
    if (rej) {
      continue;
    }

    /* z_b=x*r_b+r_a with inf norm <= \hat{B_{big}}-Bpw */
    for (i = 0; i < M_HAT; i++) {
      mult_r(out->zb + i, &(out->x), rb + i);

      for (j = 0; j < D; j++) {
        tmp = out->zb[i].poly[j] + ra[i].poly[j];

        rej |= ct_lt_s64(ZB_NORM, tmp) | ct_lt_s64(tmp, -ZB_NORM);

        out->zb[i].poly[j] = tmp;
      }
    }

    if (rej) {
      continue;
    }

    /* z_c=x*r_c+r_d with inf norm <= B_{big}-Bpw */
    for (i = 0; i < M; i++) {
      mult_r(out->zc + i, &(out->x), rc + i);

      for (j = 0; j < D; j++) {
        tmp = out->zc[i].poly[j] + rd[i].poly[j];

        rej |= ct_lt_s64(ZC_NORM, tmp) | ct_lt_s64(tmp, -ZC_NORM);

        out->zc[i].poly[j] = tmp;
      }
    }

    if (rej) {
      continue;
    }

    /* z_{out,i}=x*ck_{out,i}+r_g^(i) with inf norm <= B_{big}-Bpw */
    for (i = 0; i < M; i++) {
      mult_r(out->z_out[0] + i, &(out->x), ck_out[0] + i);

      for (j = 0; j < D; j++) {
        tmp = out->z_out[0][i].poly[j] + r_g[0][i].poly[j];

        rej |= ct_lt_s64(ZC_NORM, tmp) | ct_lt_s64(tmp, -ZC_NORM);

        out->z_out[0][i].poly[j] = tmp;
      }
    }

    if (rej) {
      continue;
    }

    for (i = 0; i < M; i++) {
      mult_r(out->z_out[1] + i, &(out->x), ck_out[1] + i);

      for (j = 0; j < D; j++) {
        tmp = out->z_out[1][i].poly[j] + r_g[1][i].poly[j];

        rej |= ct_lt_s64(ZC_NORM, tmp) | ct_lt_s64(tmp, -ZC_NORM);

        out->z_out[1][i].poly[j] = tmp;
      }
    }

    if (rej) {
      continue;
    }

    /* z^(i)=x*r_{0,l}-rho_0^(0) with inf norm <= B_{big,k}-Bpw for i<M */
    for (i = 0; i < M; i++) {
      mult_r(out->z[0] + i, &(out->x), ask[0].r + i);

      for (j = 0; j < D; j++) {
        tmp = out->z[0][i].poly[j] - rho[0][i].poly[j];

        rej |= ct_lt_s64(Z0_NORM, tmp) | ct_lt_s64(tmp, -Z0_NORM);

        out->z[0][i].poly[j] = tmp;
      }
    }

    if (rej) {
      continue;
    }

#if M_SPENT == 2
    for (i = 0; i < M; i++) {
      mult_r(out->z[1] + i, &(out->x), ask[1].r + i);

      for (j = 0; j < D; j++) {
        tmp = out->z[1][i].poly[j] - rho[1][i].poly[j];

        rej |= ct_lt_s64(Z0_NORM, tmp) | ct_lt_s64(tmp, -Z0_NORM);

        out->z[1][i].poly[j] = tmp;
      }
    }

    if (rej) {
      continue;
    }
#endif

    /* z^(M)=x*r_{M,l}-rho_0^(M) with inf norm <= B'_{big,k}-(M+S+1)*Bpw */
    for (i = 0; i < M; i++) {
      for (j = 0; j < D; j++) {
        rm.poly[j] = rm_c[i].poly[j] + rc[i].poly[j];
      }

      mult_r(out->z[M_SPENT] + i, &(out->x), &rm);

      for (j = 0; j < D; j++) {
        tmp = out->z[M_SPENT][i].poly[j] - rho[M_SPENT][i].poly[j];

        rej |= ct_lt_s64(ZM_NORM, tmp) | ct_lt_s64(tmp, -ZM_NORM);

        out->z[M_SPENT][i].poly[j] = tmp;
      }
    }

    if (rej) {
      continue;
    }
  } while (rej);
}
