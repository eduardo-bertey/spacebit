/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * sample B                       *
 * ****************************** */

#include "sampleb.h"
#include "comp.h"
#include "fastrandombytes.h"
#include "littleendian.h"
#include "param.h"
#include "poly_q.h"
#include "poly_red.h"
#include <stdint.h>

/* out<--{-B...B}^{m*d} */
static inline void sample_b(POLY_Q *out, const uint64_t m, const uint64_t len) {
  unsigned char r[SAMPLE_B_MAX_LEN];

  uint64_t i, j, x;
  unsigned char *r_head = r;

  fastrandombytes_prv(r, len);

  for (i = 0; i < m; i++) {
    for (j = 0; j < D; j++) {
      do {
        x = *(r_head++);
      } while (ct_ge_u64(x, SAMPLE_B_BOUND));

      out[i].poly[j] =
          barrett(x, 2 * B + 1, BARRETT_FACTOR_B, BARRETT_SHIFT_B) - B;
    }
  }
}

/* out<--{-B_{big}...B_{big}}^{m*d} */
static inline void sample_bbig(POLY_Q *out, const uint64_t bbig,
                               const uint64_t m, const uint64_t len,
                               const uint64_t bound,
                               const uint64_t barrett_factor) {
  unsigned char r[SAMPLE_BBIG_MAX_LEN * SAMPLE_BBIG_BYTE];

  uint64_t i, j, x;
  unsigned char *r_head = r;

  fastrandombytes_prv(r, len * SAMPLE_BBIG_BYTE);

  for (i = 0; i < m; i++) {
    for (j = 0; j < D; j++) {
      do {
        x = load_32(r_head);
        r_head += SAMPLE_BBIG_BYTE;
      } while (ct_ge_u64(x, bound));

      out[i].poly[j] =
          barrett(x, 2 * bbig + 1, barrett_factor, BARRETT_SHIFT_BBIG) - bbig;
    }
  }
}

void sample_b_md(POLY_Q *out) { sample_b(out, M, SAMPLE_B_MD_LEN); }

void sample_bbig_md(POLY_Q *out) {
  sample_bbig(out, BBIG, M, SAMPLE_BBIG_MD_LEN, SAMPLE_BBIG_BOUND,
              BARRETT_FACTOR_BBIG);
}

void sample_bbig_k_prime_md(POLY_Q *out) {
  sample_bbig(out, BBIG_K_PRIME, M, SAMPLE_BBIG_K_PRIME_MD_LEN,
              SAMPLE_BBIG_K_PRIME_BOUND, BARRETT_FACTOR_BBIG_K_PRIME);
}

void sample_b_mhatd(POLY_Q *out) { sample_b(out, M_HAT, SAMPLE_B_MHATD_LEN); }

void sample_bbig_hat_mhatd(POLY_Q *out) {
  sample_bbig(out, BBIG_HAT, M_HAT, SAMPLE_BBIG_HAT_MHATD_LEN,
              SAMPLE_BBIG_HAT_BOUND, BARRETT_FACTOR_BBIG_HAT);
}

/* out<--{-Be...Be}^{m*d} */
static inline void sample_be(POLY_QBIG *out, const uint64_t m,
                             const uint64_t len) {
  unsigned char r[SAMPLE_BE_MAX_LEN];

  uint64_t i, j, x;
  unsigned char *r_head = r;

  fastrandombytes_prv(r, len);

  for (i = 0; i < m; i++) {
    for (j = 0; j < D; j++) {
      do {
        x = *(r_head++);
      } while (ct_ge_u64(x, SAMPLE_BE_BOUND));

      x = barrett(x, 2 * BE + 1, BARRETT_FACTOR_BE, BARRETT_SHIFT_BE) - BE;
      out[i].poly1[j] = QBIG1 + x;
      out[i].poly2[j] = QBIG2 + x;
    }
  }
}

void sample_be_mhatd(POLY_QBIG *out) {
  sample_be(out, M_HAT, SAMPLE_BE_MHATD_LEN);
}

void sample_be_v1d(POLY_QBIG *out) { sample_be(out, V_1, SAMPLE_BE_V1D_LEN); }

void sample_be_v2d(POLY_QBIG *out) { sample_be(out, V_2, SAMPLE_BE_V2D_LEN); }

/* out<--{-B_a,...,B_a}^d or out<--{-B_r,...,B_r}^d */
static inline void sample_bstar_d(POLY_Q *out, const uint64_t b,
                                  const uint64_t len, const uint64_t bound,
                                  const uint64_t barrett_factor) {
  unsigned char r[SAMPLE_BSTAR_MAX_LEN * SAMPLE_BSTAR_BYTE];

  uint64_t i, x;
  unsigned char *r_head = r;

  fastrandombytes_prv(r, len * SAMPLE_BSTAR_BYTE);

  for (i = 0; i < D; i++) {
    do {
      x = load_24(r_head);
      r_head += SAMPLE_BSTAR_BYTE;
    } while (ct_ge_u64(x, bound));

    out->poly[i] =
        barrett(x, 2 * b + 1, barrett_factor, BARRETT_SHIFT_BSTAR) - b;
  }
}

void sample_ba(POLY_Q *out) {
  sample_bstar_d(out, BA, SAMPLE_BA_LEN, SAMPLE_BA_BOUND, BARRETT_FACTOR_BA);
}

void sample_br(POLY_Q *out) {
  sample_bstar_d(out, BR, SAMPLE_BR_LEN, SAMPLE_BR_BOUND, BARRETT_FACTOR_BR);
}

void sample_bap(POLY_Q *out) {
  sample_bstar_d(out, BAP, SAMPLE_BAP_LEN, SAMPLE_BAP_BOUND,
                 BARRETT_FACTOR_BAP);
}

/* sample {1,...,beta-1} */
uint64_t sample_beta() {
  unsigned char r[SAMPLE_BETA_LEN * SAMPLE_BETA_BYTE];

  uint64_t x;
  unsigned char *r_head = r;

  fastrandombytes_prv(r, SAMPLE_BETA_LEN * SAMPLE_BETA_BYTE);

  do {
    x = load_16(r_head);
    r_head += SAMPLE_BETA_BYTE;
  } while (ct_ge_u64(x, SAMPLE_BETA_BOUND));

  return barrett(x, BETA - 1, BARRETT_FACTOR_BETA, BARRETT_SHIFT_BETA) + 1;
}

void sample_s(POLY_QBIG *out) {
  unsigned char r_qbig1[REJ_UNIF_QBIG1_LEN * REJ_UNIF_QBIG1_BYTES];
  unsigned char r_qbig2[REJ_UNIF_QBIG2_LEN * REJ_UNIF_QBIG2_BYTES];

  uint64_t i, j, x, y;

  unsigned char *r_qbig1_head = r_qbig1;
  unsigned char *r_qbig2_head = r_qbig2;

  fastrandombytes_prv(r_qbig1, REJ_UNIF_QBIG1_LEN * REJ_UNIF_QBIG1_BYTES);
  fastrandombytes_prv(r_qbig2, REJ_UNIF_QBIG2_LEN * REJ_UNIF_QBIG2_BYTES);

  for (i = 0; i < N_HAT - 1; i++) {
    for (j = 0; j < D; j++) {
      do {
        x = load_32(r_qbig1_head);
        r_qbig1_head += REJ_UNIF_QBIG1_BYTES;
      } while (ct_ge_u64(x, REJ_UNIF_QBIG1_BOUND));

      do {
        y = load_32(r_qbig2_head);
        r_qbig2_head += REJ_UNIF_QBIG2_BYTES;
      } while (ct_ge_u64(y, REJ_UNIF_QBIG2_BOUND));

      out[i].poly1[j] = red_short_qbig1(x);
      out[i].poly2[j] = red_short_qbig2(y);
    }
  }
}
