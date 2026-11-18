/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * ntt on R_q                     *
 * ****************************** */

#include "poly_q.h"
#include "poly_param.h"
#include "poly_red.h"
#include <stdint.h>

#define INV_NTT 1610416135

static const uint64_t omega[4] = {0, 1274573788, 1273977873, 871730293};
static const uint64_t omega_inv[4] = {0, 1275491220, 655115411, 872647725};

/* two levels splitting into 4 subrings */
static inline void ntt(uint64_t *a) {
  uint64_t i;
  uint64_t u, v;

  /* implicitly convert to Montgomery form and perform the butterfly operations
   */
  /* butterfly: (u,v)-->(u+v*omega,u-v*omega) */
  for (i = 0; i < D >> 1; i++) {
    u = montgomery_q(a[i], MONTGOMERY_CONVERT_FACTOR_Q);
    v = montgomery_q(a[i + (D >> 1)], omega[2]);

    a[i] = con_sub(u + v, Q);
    a[i + (D >> 1)] = con_add(u - v, Q);
  }

  for (i = 0; i < D >> 2; i++) {
    u = a[i];
    v = montgomery_q(a[i + (D >> 2)], omega[1]);

    a[i] = con_sub(u + v, Q);
    a[i + (D >> 2)] = con_add(u - v, Q);
  }

  for (i = 0; i < D >> 2; i++) {
    u = a[i + (D >> 1)];
    v = montgomery_q(a[i + (D >> 1) + (D >> 2)], omega[3]);

    a[i + (D >> 1)] = con_sub(u + v, Q);
    a[i + (D >> 1) + (D >> 2)] = con_add(u - v, Q);
  }
}

static inline void intt(uint64_t *a) {
  uint64_t i;
  uint64_t u, v;

  /* butterfly: (u,v)-->(u+v, (u-v)*omega) */
  for (i = 0; i < D >> 2; i++) {
    u = a[i];
    v = a[i + (D >> 2)];

    a[i] = con_sub(u + v, Q);
    a[i + (D >> 2)] = montgomery_q(Q + u - v, omega_inv[1]);
  }

  for (i = 0; i < D >> 2; i++) {
    u = a[i + (D >> 1)];
    v = a[i + (D >> 1) + (D >> 2)];

    a[i + (D >> 1)] = con_sub(u + v, Q);
    a[i + (D >> 1) + (D >> 2)] = montgomery_q(Q + u - v, omega_inv[3]);
  }

  /* implicitly get rid of the Montgomery factor */
  for (i = 0; i < D >> 1; i++) {
    u = a[i];
    v = a[i + (D >> 1)];

    a[i] = montgomery_q(u + v, INV_NTT);
    a[i + (D >> 1)] = montgomery_q(Q + u - v, omega_inv[2]);
  }
}

/* schoolbook out=a*b on subring x^16 +/- omega */
static inline void mult_16_plus(uint64_t *out, const uint64_t *a,
                                const uint64_t *b, const uint64_t omega) {
  out[0] = red_short_q(
      montgomery_q(a[0], b[0]) +
      montgomery_q(
          red_short_q(montgomery_q(a[1], b[15]) + montgomery_q(a[2], b[14]) +
                      montgomery_q(a[3], b[13]) + montgomery_q(a[4], b[12]) +
                      montgomery_q(a[5], b[11]) + montgomery_q(a[6], b[10]) +
                      montgomery_q(a[7], b[9]) + montgomery_q(a[8], b[8]) +
                      montgomery_q(a[9], b[7]) + montgomery_q(a[10], b[6]) +
                      montgomery_q(a[11], b[5]) + montgomery_q(a[12], b[4]) +
                      montgomery_q(a[13], b[3]) + montgomery_q(a[14], b[2]) +
                      montgomery_q(a[15], b[1])),
          omega));
  out[1] = red_short_q(
      montgomery_q(a[0], b[1]) + montgomery_q(a[1], b[0]) +
      montgomery_q(
          red_short_q(montgomery_q(a[2], b[15]) + montgomery_q(a[3], b[14]) +
                      montgomery_q(a[4], b[13]) + montgomery_q(a[5], b[12]) +
                      montgomery_q(a[6], b[11]) + montgomery_q(a[7], b[10]) +
                      montgomery_q(a[8], b[9]) + montgomery_q(a[9], b[8]) +
                      montgomery_q(a[10], b[7]) + montgomery_q(a[11], b[6]) +
                      montgomery_q(a[12], b[5]) + montgomery_q(a[13], b[4]) +
                      montgomery_q(a[14], b[3]) + montgomery_q(a[15], b[2])),
          omega));
  out[2] = red_short_q(
      montgomery_q(a[0], b[2]) + montgomery_q(a[1], b[1]) +
      montgomery_q(a[2], b[0]) +
      montgomery_q(
          red_short_q(montgomery_q(a[3], b[15]) + montgomery_q(a[4], b[14]) +
                      montgomery_q(a[5], b[13]) + montgomery_q(a[6], b[12]) +
                      montgomery_q(a[7], b[11]) + montgomery_q(a[8], b[10]) +
                      montgomery_q(a[9], b[9]) + montgomery_q(a[10], b[8]) +
                      montgomery_q(a[11], b[7]) + montgomery_q(a[12], b[6]) +
                      montgomery_q(a[13], b[5]) + montgomery_q(a[14], b[4]) +
                      montgomery_q(a[15], b[3])),
          omega));
  out[3] = red_short_q(
      montgomery_q(a[0], b[3]) + montgomery_q(a[1], b[2]) +
      montgomery_q(a[2], b[1]) + montgomery_q(a[3], b[0]) +
      montgomery_q(
          red_short_q(montgomery_q(a[4], b[15]) + montgomery_q(a[5], b[14]) +
                      montgomery_q(a[6], b[13]) + montgomery_q(a[7], b[12]) +
                      montgomery_q(a[8], b[11]) + montgomery_q(a[9], b[10]) +
                      montgomery_q(a[10], b[9]) + montgomery_q(a[11], b[8]) +
                      montgomery_q(a[12], b[7]) + montgomery_q(a[13], b[6]) +
                      montgomery_q(a[14], b[5]) + montgomery_q(a[15], b[4])),
          omega));
  out[4] = red_short_q(
      montgomery_q(a[0], b[4]) + montgomery_q(a[1], b[3]) +
      montgomery_q(a[2], b[2]) + montgomery_q(a[3], b[1]) +
      montgomery_q(a[4], b[0]) +
      montgomery_q(
          red_short_q(montgomery_q(a[5], b[15]) + montgomery_q(a[6], b[14]) +
                      montgomery_q(a[7], b[13]) + montgomery_q(a[8], b[12]) +
                      montgomery_q(a[9], b[11]) + montgomery_q(a[10], b[10]) +
                      montgomery_q(a[11], b[9]) + montgomery_q(a[12], b[8]) +
                      montgomery_q(a[13], b[7]) + montgomery_q(a[14], b[6]) +
                      montgomery_q(a[15], b[5])),
          omega));
  out[5] = red_short_q(
      montgomery_q(a[0], b[5]) + montgomery_q(a[1], b[4]) +
      montgomery_q(a[2], b[3]) + montgomery_q(a[3], b[2]) +
      montgomery_q(a[4], b[1]) + montgomery_q(a[5], b[0]) +
      montgomery_q(
          red_short_q(montgomery_q(a[6], b[15]) + montgomery_q(a[7], b[14]) +
                      montgomery_q(a[8], b[13]) + montgomery_q(a[9], b[12]) +
                      montgomery_q(a[10], b[11]) + montgomery_q(a[11], b[10]) +
                      montgomery_q(a[12], b[9]) + montgomery_q(a[13], b[8]) +
                      montgomery_q(a[14], b[7]) + montgomery_q(a[15], b[6])),
          omega));
  out[6] = red_short_q(
      montgomery_q(a[0], b[6]) + montgomery_q(a[1], b[5]) +
      montgomery_q(a[2], b[4]) + montgomery_q(a[3], b[3]) +
      montgomery_q(a[4], b[2]) + montgomery_q(a[5], b[1]) +
      montgomery_q(a[6], b[0]) +
      montgomery_q(
          red_short_q(montgomery_q(a[7], b[15]) + montgomery_q(a[8], b[14]) +
                      montgomery_q(a[9], b[13]) + montgomery_q(a[10], b[12]) +
                      montgomery_q(a[11], b[11]) + montgomery_q(a[12], b[10]) +
                      montgomery_q(a[13], b[9]) + montgomery_q(a[14], b[8]) +
                      montgomery_q(a[15], b[7])),
          omega));
  out[7] = red_short_q(
      montgomery_q(a[0], b[7]) + montgomery_q(a[1], b[6]) +
      montgomery_q(a[2], b[5]) + montgomery_q(a[3], b[4]) +
      montgomery_q(a[4], b[3]) + montgomery_q(a[5], b[2]) +
      montgomery_q(a[6], b[1]) + montgomery_q(a[7], b[0]) +
      montgomery_q(
          red_short_q(montgomery_q(a[8], b[15]) + montgomery_q(a[9], b[14]) +
                      montgomery_q(a[10], b[13]) + montgomery_q(a[11], b[12]) +
                      montgomery_q(a[12], b[11]) + montgomery_q(a[13], b[10]) +
                      montgomery_q(a[14], b[9]) + montgomery_q(a[15], b[8])),
          omega));
  out[8] = red_short_q(
      montgomery_q(a[0], b[8]) + montgomery_q(a[1], b[7]) +
      montgomery_q(a[2], b[6]) + montgomery_q(a[3], b[5]) +
      montgomery_q(a[4], b[4]) + montgomery_q(a[5], b[3]) +
      montgomery_q(a[6], b[2]) + montgomery_q(a[7], b[1]) +
      montgomery_q(a[8], b[0]) +
      montgomery_q(
          red_short_q(montgomery_q(a[9], b[15]) + montgomery_q(a[10], b[14]) +
                      montgomery_q(a[11], b[13]) + montgomery_q(a[12], b[12]) +
                      montgomery_q(a[13], b[11]) + montgomery_q(a[14], b[10]) +
                      montgomery_q(a[15], b[9])),
          omega));
  out[9] = red_short_q(
      montgomery_q(a[0], b[9]) + montgomery_q(a[1], b[8]) +
      montgomery_q(a[2], b[7]) + montgomery_q(a[3], b[6]) +
      montgomery_q(a[4], b[5]) + montgomery_q(a[5], b[4]) +
      montgomery_q(a[6], b[3]) + montgomery_q(a[7], b[2]) +
      montgomery_q(a[8], b[1]) + montgomery_q(a[9], b[0]) +
      montgomery_q(
          red_short_q(montgomery_q(a[10], b[15]) + montgomery_q(a[11], b[14]) +
                      montgomery_q(a[12], b[13]) + montgomery_q(a[13], b[12]) +
                      montgomery_q(a[14], b[11]) + montgomery_q(a[15], b[10])),
          omega));
  out[10] = red_short_q(
      montgomery_q(a[0], b[10]) + montgomery_q(a[1], b[9]) +
      montgomery_q(a[2], b[8]) + montgomery_q(a[3], b[7]) +
      montgomery_q(a[4], b[6]) + montgomery_q(a[5], b[5]) +
      montgomery_q(a[6], b[4]) + montgomery_q(a[7], b[3]) +
      montgomery_q(a[8], b[2]) + montgomery_q(a[9], b[1]) +
      montgomery_q(a[10], b[0]) +
      montgomery_q(
          red_short_q(montgomery_q(a[11], b[15]) + montgomery_q(a[12], b[14]) +
                      montgomery_q(a[13], b[13]) + montgomery_q(a[14], b[12]) +
                      montgomery_q(a[15], b[11])),
          omega));
  out[11] = red_short_q(
      montgomery_q(a[0], b[11]) + montgomery_q(a[1], b[10]) +
      montgomery_q(a[2], b[9]) + montgomery_q(a[3], b[8]) +
      montgomery_q(a[4], b[7]) + montgomery_q(a[5], b[6]) +
      montgomery_q(a[6], b[5]) + montgomery_q(a[7], b[4]) +
      montgomery_q(a[8], b[3]) + montgomery_q(a[9], b[2]) +
      montgomery_q(a[10], b[1]) + montgomery_q(a[11], b[0]) +
      montgomery_q(
          red_short_q(montgomery_q(a[12], b[15]) + montgomery_q(a[13], b[14]) +
                      montgomery_q(a[14], b[13]) + montgomery_q(a[15], b[12])),
          omega));
  out[12] = red_short_q(montgomery_q(a[0], b[12]) + montgomery_q(a[1], b[11]) +
                        montgomery_q(a[2], b[10]) + montgomery_q(a[3], b[9]) +
                        montgomery_q(a[4], b[8]) + montgomery_q(a[5], b[7]) +
                        montgomery_q(a[6], b[6]) + montgomery_q(a[7], b[5]) +
                        montgomery_q(a[8], b[4]) + montgomery_q(a[9], b[3]) +
                        montgomery_q(a[10], b[2]) + montgomery_q(a[11], b[1]) +
                        montgomery_q(a[12], b[0]) +
                        montgomery_q(red_short_q(montgomery_q(a[13], b[15]) +
                                                 montgomery_q(a[14], b[14]) +
                                                 montgomery_q(a[15], b[13])),
                                     omega));
  out[13] = red_short_q(
      montgomery_q(a[0], b[13]) + montgomery_q(a[1], b[12]) +
      montgomery_q(a[2], b[11]) + montgomery_q(a[3], b[10]) +
      montgomery_q(a[4], b[9]) + montgomery_q(a[5], b[8]) +
      montgomery_q(a[6], b[7]) + montgomery_q(a[7], b[6]) +
      montgomery_q(a[8], b[5]) + montgomery_q(a[9], b[4]) +
      montgomery_q(a[10], b[3]) + montgomery_q(a[11], b[2]) +
      montgomery_q(a[12], b[1]) + montgomery_q(a[13], b[0]) +
      montgomery_q(montgomery_q(a[14], b[15]) + montgomery_q(a[15], b[14]),
                   omega));
  out[14] = red_short_q(montgomery_q(a[0], b[14]) + montgomery_q(a[1], b[13]) +
                        montgomery_q(a[2], b[12]) + montgomery_q(a[3], b[11]) +
                        montgomery_q(a[4], b[10]) + montgomery_q(a[5], b[9]) +
                        montgomery_q(a[6], b[8]) + montgomery_q(a[7], b[7]) +
                        montgomery_q(a[8], b[6]) + montgomery_q(a[9], b[5]) +
                        montgomery_q(a[10], b[4]) + montgomery_q(a[11], b[3]) +
                        montgomery_q(a[12], b[2]) + montgomery_q(a[13], b[1]) +
                        montgomery_q(a[14], b[0]) +
                        montgomery_q(montgomery_q(a[15], b[15]), omega));
  out[15] = red_short_q(montgomery_q(a[0], b[15]) + montgomery_q(a[1], b[14]) +
                        montgomery_q(a[2], b[13]) + montgomery_q(a[3], b[12]) +
                        montgomery_q(a[4], b[11]) + montgomery_q(a[5], b[10]) +
                        montgomery_q(a[6], b[9]) + montgomery_q(a[7], b[8]) +
                        montgomery_q(a[8], b[7]) + montgomery_q(a[9], b[6]) +
                        montgomery_q(a[10], b[5]) + montgomery_q(a[11], b[4]) +
                        montgomery_q(a[12], b[3]) + montgomery_q(a[13], b[2]) +
                        montgomery_q(a[14], b[1]) + montgomery_q(a[15], b[0]));
}

static inline void mult_16_minus(uint64_t *out, const uint64_t *a,
                                 const uint64_t *b, const uint64_t omega) {
  out[0] = red_short_q(
      montgomery_q(a[0], b[0]) + Q -
      montgomery_q(
          red_short_q(montgomery_q(a[1], b[15]) + montgomery_q(a[2], b[14]) +
                      montgomery_q(a[3], b[13]) + montgomery_q(a[4], b[12]) +
                      montgomery_q(a[5], b[11]) + montgomery_q(a[6], b[10]) +
                      montgomery_q(a[7], b[9]) + montgomery_q(a[8], b[8]) +
                      montgomery_q(a[9], b[7]) + montgomery_q(a[10], b[6]) +
                      montgomery_q(a[11], b[5]) + montgomery_q(a[12], b[4]) +
                      montgomery_q(a[13], b[3]) + montgomery_q(a[14], b[2]) +
                      montgomery_q(a[15], b[1])),
          omega));
  out[1] = red_short_q(
      montgomery_q(a[0], b[1]) + montgomery_q(a[1], b[0]) + Q -
      montgomery_q(
          red_short_q(montgomery_q(a[2], b[15]) + montgomery_q(a[3], b[14]) +
                      montgomery_q(a[4], b[13]) + montgomery_q(a[5], b[12]) +
                      montgomery_q(a[6], b[11]) + montgomery_q(a[7], b[10]) +
                      montgomery_q(a[8], b[9]) + montgomery_q(a[9], b[8]) +
                      montgomery_q(a[10], b[7]) + montgomery_q(a[11], b[6]) +
                      montgomery_q(a[12], b[5]) + montgomery_q(a[13], b[4]) +
                      montgomery_q(a[14], b[3]) + montgomery_q(a[15], b[2])),
          omega));
  out[2] = red_short_q(
      montgomery_q(a[0], b[2]) + montgomery_q(a[1], b[1]) +
      montgomery_q(a[2], b[0]) + Q -
      montgomery_q(
          red_short_q(montgomery_q(a[3], b[15]) + montgomery_q(a[4], b[14]) +
                      montgomery_q(a[5], b[13]) + montgomery_q(a[6], b[12]) +
                      montgomery_q(a[7], b[11]) + montgomery_q(a[8], b[10]) +
                      montgomery_q(a[9], b[9]) + montgomery_q(a[10], b[8]) +
                      montgomery_q(a[11], b[7]) + montgomery_q(a[12], b[6]) +
                      montgomery_q(a[13], b[5]) + montgomery_q(a[14], b[4]) +
                      montgomery_q(a[15], b[3])),
          omega));
  out[3] = red_short_q(
      montgomery_q(a[0], b[3]) + montgomery_q(a[1], b[2]) +
      montgomery_q(a[2], b[1]) + montgomery_q(a[3], b[0]) + Q -
      montgomery_q(
          red_short_q(montgomery_q(a[4], b[15]) + montgomery_q(a[5], b[14]) +
                      montgomery_q(a[6], b[13]) + montgomery_q(a[7], b[12]) +
                      montgomery_q(a[8], b[11]) + montgomery_q(a[9], b[10]) +
                      montgomery_q(a[10], b[9]) + montgomery_q(a[11], b[8]) +
                      montgomery_q(a[12], b[7]) + montgomery_q(a[13], b[6]) +
                      montgomery_q(a[14], b[5]) + montgomery_q(a[15], b[4])),
          omega));
  out[4] = red_short_q(
      montgomery_q(a[0], b[4]) + montgomery_q(a[1], b[3]) +
      montgomery_q(a[2], b[2]) + montgomery_q(a[3], b[1]) +
      montgomery_q(a[4], b[0]) + Q -
      montgomery_q(
          red_short_q(montgomery_q(a[5], b[15]) + montgomery_q(a[6], b[14]) +
                      montgomery_q(a[7], b[13]) + montgomery_q(a[8], b[12]) +
                      montgomery_q(a[9], b[11]) + montgomery_q(a[10], b[10]) +
                      montgomery_q(a[11], b[9]) + montgomery_q(a[12], b[8]) +
                      montgomery_q(a[13], b[7]) + montgomery_q(a[14], b[6]) +
                      montgomery_q(a[15], b[5])),
          omega));
  out[5] = red_short_q(
      montgomery_q(a[0], b[5]) + montgomery_q(a[1], b[4]) +
      montgomery_q(a[2], b[3]) + montgomery_q(a[3], b[2]) +
      montgomery_q(a[4], b[1]) + montgomery_q(a[5], b[0]) + Q -
      montgomery_q(
          red_short_q(montgomery_q(a[6], b[15]) + montgomery_q(a[7], b[14]) +
                      montgomery_q(a[8], b[13]) + montgomery_q(a[9], b[12]) +
                      montgomery_q(a[10], b[11]) + montgomery_q(a[11], b[10]) +
                      montgomery_q(a[12], b[9]) + montgomery_q(a[13], b[8]) +
                      montgomery_q(a[14], b[7]) + montgomery_q(a[15], b[6])),
          omega));
  out[6] = red_short_q(
      montgomery_q(a[0], b[6]) + montgomery_q(a[1], b[5]) +
      montgomery_q(a[2], b[4]) + montgomery_q(a[3], b[3]) +
      montgomery_q(a[4], b[2]) + montgomery_q(a[5], b[1]) +
      montgomery_q(a[6], b[0]) + Q -
      montgomery_q(
          red_short_q(montgomery_q(a[7], b[15]) + montgomery_q(a[8], b[14]) +
                      montgomery_q(a[9], b[13]) + montgomery_q(a[10], b[12]) +
                      montgomery_q(a[11], b[11]) + montgomery_q(a[12], b[10]) +
                      montgomery_q(a[13], b[9]) + montgomery_q(a[14], b[8]) +
                      montgomery_q(a[15], b[7])),
          omega));
  out[7] = red_short_q(
      montgomery_q(a[0], b[7]) + montgomery_q(a[1], b[6]) +
      montgomery_q(a[2], b[5]) + montgomery_q(a[3], b[4]) +
      montgomery_q(a[4], b[3]) + montgomery_q(a[5], b[2]) +
      montgomery_q(a[6], b[1]) + montgomery_q(a[7], b[0]) + Q -
      montgomery_q(
          red_short_q(montgomery_q(a[8], b[15]) + montgomery_q(a[9], b[14]) +
                      montgomery_q(a[10], b[13]) + montgomery_q(a[11], b[12]) +
                      montgomery_q(a[12], b[11]) + montgomery_q(a[13], b[10]) +
                      montgomery_q(a[14], b[9]) + montgomery_q(a[15], b[8])),
          omega));
  out[8] = red_short_q(
      montgomery_q(a[0], b[8]) + montgomery_q(a[1], b[7]) +
      montgomery_q(a[2], b[6]) + montgomery_q(a[3], b[5]) +
      montgomery_q(a[4], b[4]) + montgomery_q(a[5], b[3]) +
      montgomery_q(a[6], b[2]) + montgomery_q(a[7], b[1]) +
      montgomery_q(a[8], b[0]) + Q -
      montgomery_q(
          red_short_q(montgomery_q(a[9], b[15]) + montgomery_q(a[10], b[14]) +
                      montgomery_q(a[11], b[13]) + montgomery_q(a[12], b[12]) +
                      montgomery_q(a[13], b[11]) + montgomery_q(a[14], b[10]) +
                      montgomery_q(a[15], b[9])),
          omega));
  out[9] = red_short_q(
      montgomery_q(a[0], b[9]) + montgomery_q(a[1], b[8]) +
      montgomery_q(a[2], b[7]) + montgomery_q(a[3], b[6]) +
      montgomery_q(a[4], b[5]) + montgomery_q(a[5], b[4]) +
      montgomery_q(a[6], b[3]) + montgomery_q(a[7], b[2]) +
      montgomery_q(a[8], b[1]) + montgomery_q(a[9], b[0]) + Q -
      montgomery_q(
          red_short_q(montgomery_q(a[10], b[15]) + montgomery_q(a[11], b[14]) +
                      montgomery_q(a[12], b[13]) + montgomery_q(a[13], b[12]) +
                      montgomery_q(a[14], b[11]) + montgomery_q(a[15], b[10])),
          omega));
  out[10] = red_short_q(
      montgomery_q(a[0], b[10]) + montgomery_q(a[1], b[9]) +
      montgomery_q(a[2], b[8]) + montgomery_q(a[3], b[7]) +
      montgomery_q(a[4], b[6]) + montgomery_q(a[5], b[5]) +
      montgomery_q(a[6], b[4]) + montgomery_q(a[7], b[3]) +
      montgomery_q(a[8], b[2]) + montgomery_q(a[9], b[1]) +
      montgomery_q(a[10], b[0]) + Q -
      montgomery_q(
          red_short_q(montgomery_q(a[11], b[15]) + montgomery_q(a[12], b[14]) +
                      montgomery_q(a[13], b[13]) + montgomery_q(a[14], b[12]) +
                      montgomery_q(a[15], b[11])),
          omega));
  out[11] = red_short_q(
      montgomery_q(a[0], b[11]) + montgomery_q(a[1], b[10]) +
      montgomery_q(a[2], b[9]) + montgomery_q(a[3], b[8]) +
      montgomery_q(a[4], b[7]) + montgomery_q(a[5], b[6]) +
      montgomery_q(a[6], b[5]) + montgomery_q(a[7], b[4]) +
      montgomery_q(a[8], b[3]) + montgomery_q(a[9], b[2]) +
      montgomery_q(a[10], b[1]) + montgomery_q(a[11], b[0]) + Q -
      montgomery_q(
          red_short_q(montgomery_q(a[12], b[15]) + montgomery_q(a[13], b[14]) +
                      montgomery_q(a[14], b[13]) + montgomery_q(a[15], b[12])),
          omega));
  out[12] = red_short_q(montgomery_q(a[0], b[12]) + montgomery_q(a[1], b[11]) +
                        montgomery_q(a[2], b[10]) + montgomery_q(a[3], b[9]) +
                        montgomery_q(a[4], b[8]) + montgomery_q(a[5], b[7]) +
                        montgomery_q(a[6], b[6]) + montgomery_q(a[7], b[5]) +
                        montgomery_q(a[8], b[4]) + montgomery_q(a[9], b[3]) +
                        montgomery_q(a[10], b[2]) + montgomery_q(a[11], b[1]) +
                        montgomery_q(a[12], b[0]) + Q -
                        montgomery_q(red_short_q(montgomery_q(a[13], b[15]) +
                                                 montgomery_q(a[14], b[14]) +
                                                 montgomery_q(a[15], b[13])),
                                     omega));
  out[13] = red_short_q(
      montgomery_q(a[0], b[13]) + montgomery_q(a[1], b[12]) +
      montgomery_q(a[2], b[11]) + montgomery_q(a[3], b[10]) +
      montgomery_q(a[4], b[9]) + montgomery_q(a[5], b[8]) +
      montgomery_q(a[6], b[7]) + montgomery_q(a[7], b[6]) +
      montgomery_q(a[8], b[5]) + montgomery_q(a[9], b[4]) +
      montgomery_q(a[10], b[3]) + montgomery_q(a[11], b[2]) +
      montgomery_q(a[12], b[1]) + montgomery_q(a[13], b[0]) + Q -
      montgomery_q(montgomery_q(a[14], b[15]) + montgomery_q(a[15], b[14]),
                   omega));
  out[14] = red_short_q(montgomery_q(a[0], b[14]) + montgomery_q(a[1], b[13]) +
                        montgomery_q(a[2], b[12]) + montgomery_q(a[3], b[11]) +
                        montgomery_q(a[4], b[10]) + montgomery_q(a[5], b[9]) +
                        montgomery_q(a[6], b[8]) + montgomery_q(a[7], b[7]) +
                        montgomery_q(a[8], b[6]) + montgomery_q(a[9], b[5]) +
                        montgomery_q(a[10], b[4]) + montgomery_q(a[11], b[3]) +
                        montgomery_q(a[12], b[2]) + montgomery_q(a[13], b[1]) +
                        montgomery_q(a[14], b[0]) + Q -
                        montgomery_q(montgomery_q(a[15], b[15]), omega));
  out[15] = red_short_q(montgomery_q(a[0], b[15]) + montgomery_q(a[1], b[14]) +
                        montgomery_q(a[2], b[13]) + montgomery_q(a[3], b[12]) +
                        montgomery_q(a[4], b[11]) + montgomery_q(a[5], b[10]) +
                        montgomery_q(a[6], b[9]) + montgomery_q(a[7], b[8]) +
                        montgomery_q(a[8], b[7]) + montgomery_q(a[9], b[6]) +
                        montgomery_q(a[10], b[5]) + montgomery_q(a[11], b[4]) +
                        montgomery_q(a[12], b[3]) + montgomery_q(a[13], b[2]) +
                        montgomery_q(a[14], b[1]) + montgomery_q(a[15], b[0]));
}

/* out=a*b on 4 ntt subrings */
static inline void mult(uint64_t *out, const uint64_t *a, const uint64_t *b) {
  mult_16_plus(out, a, b, omega[1]);
  mult_16_minus(out + (D >> 2), a + (D >> 2), b + (D >> 2), omega[1]);
  mult_16_plus(out + (D >> 1), a + (D >> 1), b + (D >> 1), omega[3]);
  mult_16_minus(out + (D >> 1) + (D >> 2), a + (D >> 1) + (D >> 2),
                b + (D >> 1) + (D >> 2), omega[3]);
}

void ntt_q(POLY_Q *a) { ntt(a->poly); }

void intt_q(POLY_Q *a) { intt(a->poly); }

void mult_rq(POLY_Q *out, const POLY_Q *a, const POLY_Q *b) {
  mult(out->poly, a->poly, b->poly);
}
