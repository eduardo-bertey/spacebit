/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * constants                      *
 * ****************************** */

#ifndef _PARAM_H
#define _PARAM_H

#include "poly_param.h"

#define CRYPTO_BYTES 32

#define B 1LL
#ifndef BETA
#define BETA 100LL
#endif
#define R 64LL
#define N 18LL
#define M 38LL
#define N_HAT 35LL
#define M_HAT 69LL
#define W 56LL
#define P 8LL
#define BE 1LL

#define M_SPENT 1LL
#define S 2LL
#define N_SPENT BETA
#define N_SPENT_LOG 7

#define V_HAT (2 * (BETA + R - 1 + S * R))
#define T (S + 2)

#define V_1 (BETA + S * R + R - 1)
#define V_2 (BETA + S * R + R - 1)

#define TD_T 5
#define TD_D 58
#define TD_L 1

#define E_BOUND 3031281439163LL

#define BBIG 5229773LL
#define BBIG_K_PRIME 10459546LL
#define BA 10240LL
#define BR 98304LL
#define BAP (BA - P)
#define BRP (BR - P)

#define F00_NORM (BA * BA * D * (BETA - 1))

/* TG base para 100 llaves (valor de fabrica). gnorm.c lo escala por N/100. */
#define TG_COEFF_0 0
#define TG_COEFF_1 2365587456
#define TG_COEFF_2 1001492
#define TG_COEFF_3 0

#define BBIG_HAT 63307776LL
#define ZB_NORM (BBIG_HAT - B * P * W)
#define ZC_NORM (BBIG - B * P * W)
#define Z0_NORM (BBIG - B * P * W)
#define ZM_NORM (BBIG_K_PRIME - (M_SPENT + S + 1) * B * P * W)

#define REJ_UNIF_Q_BYTES 4
#define REJ_UNIF_Q_G0_LEN 43836
#define REJ_UNIF_Q_GR_LEN 117594
#define REJ_UNIF_Q_H_LEN 2458
#define REJ_UNIF_Q_BOUND 4294443026

#define REJ_UNIF_QBIG_BYTES 7
#define REJ_UNIF_QBIG_LEN 1458353
#define REJ_UNIF_QBIG_BOUND 72056460838172417LL

#define REJ_UNIF_QBIG1_BYTES 4
#define REJ_UNIF_QBIG1_LEN 2193
#define REJ_UNIF_QBIG1_BOUND 4294901792LL
#define REJ_UNIF_QBIG2_BYTES 4
#define REJ_UNIF_QBIG2_LEN 2187
#define REJ_UNIF_QBIG2_BOUND 4294965256LL

#define SAMPLE_B_MD_LEN 2508
#define SAMPLE_B_MHATD_LEN 4515
#define SAMPLE_B_MAX_LEN 4515
#define SAMPLE_B_BOUND 255

#define SAMPLE_BE_MHATD_LEN 4515
#define SAMPLE_BE_V1D_LEN 18839
#define SAMPLE_BE_V2D_LEN 18839
#define SAMPLE_BE_MAX_LEN 18839
#define SAMPLE_BE_BOUND 255

#define SAMPLE_BBIG_MD_LEN 2485
#define SAMPLE_BBIG_MAX_LEN 4719
#define SAMPLE_BBIG_BYTE 4
#define SAMPLE_BBIG_BOUND 4288414270

#define SAMPLE_BBIG_K_PRIME_MD_LEN 2485
#define SAMPLE_BBIG_K_PRIME_BOUND 4288414065

#define SAMPLE_BBIG_HAT_MHATD_LEN 4719
#define SAMPLE_BBIG_HAT_BOUND 4178313249

#define SAMPLE_BSTAR_BYTE 3
#define SAMPLE_BSTAR_MAX_LEN 90

#define SAMPLE_BA_LEN 79
#define SAMPLE_BA_BOUND 16773939

#define SAMPLE_BR_LEN 90
#define SAMPLE_BR_BOUND 16711765

#define SAMPLE_BAP_LEN 84
#define SAMPLE_BAP_BOUND 16760835

#define SAMPLE_BETA_LEN 16
#define SAMPLE_BETA_BYTE 2
#define SAMPLE_BETA_BOUND 65439

#define BARRETT_FACTOR_B 85
#define BARRETT_SHIFT_B 8

#define BARRETT_FACTOR_BE 85
#define BARRETT_SHIFT_BE 8

#define BARRETT_FACTOR_BBIG 410
#define BARRETT_FACTOR_BBIG_K_PRIME 205
#define BARRETT_FACTOR_BBIG_HAT 33
#define BARRETT_SHIFT_BBIG 32

#define BARRETT_SHIFT_BSTAR 24
#define BARRETT_FACTOR_BA 819
#define BARRETT_FACTOR_BR 85
#define BARRETT_FACTOR_BAP 819

#define BARRETT_FACTOR_BETA 661
#define BARRETT_SHIFT_BETA 16

#define HASH_INPUT_BYTES 4
#if M_SPENT == 1
#define HASH_INPUT_LEN                                                         \
  (2 * N_HAT * D + 2 * N_HAT * D + N * D + N * D + N * D + N * D + D + N * D + \
   N * D + D + N_SPENT * (N * D + N * D) + N * D + N * D + N * D + N * D)
#else
#define HASH_INPUT_LEN                                                         \
  (2 * N_HAT * D + 2 * N_HAT * D + N * D + N * D + N * D + N * D + D + N * D + \
   N * D + D + N_SPENT * (N * D + N * D) + N * D + N * D + N * D + N * D +     \
   N * D + D + D + N_SPENT * (N * D + N * D))
#endif
#define HASH_OUTPUT_LEN 86
#define HASH_COEFF_LEN 28

#endif
