/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * modulo reductions              *
 * ****************************** */

#ifndef _POLY_RED_H
#define _POLY_RED_H

#include "poly_param.h"
#include <stdint.h>

#define RED_SHORT_SHIFT_1_Q 31
#define RED_SHORT_SHIFT_2_Q 18
#define RED_SHORT_SHIFT_3_Q 3
#define RED_SHORT_MASK_Q ((1LL << RED_SHORT_SHIFT_1_Q) - 1)

#define RED_SHORT_SHIFT_1_QBIG1 27
#define RED_SHORT_SHIFT_2_QBIG1 11
#define RED_SHORT_MASK_QBIG1 ((1LL << RED_SHORT_SHIFT_1_QBIG1) - 1)

#define RED_SHORT_SHIFT_1_QBIG2 29
#define RED_SHORT_SHIFT_2_QBIG2 8
#define RED_SHORT_MASK_QBIG2 ((1LL << RED_SHORT_SHIFT_1_QBIG2) - 1)

static inline uint64_t con_sub(const uint64_t x, const uint64_t q) {
  return x - ((-(1 ^ ((x - q) >> 63))) & q);
}

static inline uint64_t con_add(const uint64_t x, const uint64_t q) {
  return x + ((-(x >> 63)) & q);
}

/* low hamming weight reduction
 * for q=2^(k_1) +/- 2^(k_2) + 1, k_1>k_2, and input x
 * x=u*2^(k_1)+v mod q=u*(-/+ 2^(k_2)-1)+v mod q */
static inline uint64_t red_short_q(uint64_t t) {
  uint64_t x, y;

  x = t >> RED_SHORT_SHIFT_1_Q;
  y = t & RED_SHORT_MASK_Q;

  return con_sub(
      y + (x << RED_SHORT_SHIFT_2_Q) - (x << RED_SHORT_SHIFT_3_Q) - x, Q);
}

static inline uint64_t red_short_qbig(uint64_t t,
                                      const uint64_t red_short_shift_1,
                                      const uint64_t red_short_shift_2,
                                      const uint64_t red_short_mask,
                                      const uint64_t q) {
  uint64_t x, y;

  x = t >> red_short_shift_1;
  y = t & red_short_mask;

  return con_sub((x << red_short_shift_2) + y - x, q);
}

static inline uint64_t red_short_qbig1(uint64_t t) {
  return red_short_qbig(t, RED_SHORT_SHIFT_1_QBIG1, RED_SHORT_SHIFT_2_QBIG1,
                        RED_SHORT_MASK_QBIG1, QBIG1);
}

static inline uint64_t red_short_qbig2(uint64_t t) {
  return red_short_qbig(t, RED_SHORT_SHIFT_1_QBIG2, RED_SHORT_SHIFT_2_QBIG2,
                        RED_SHORT_MASK_QBIG2, QBIG2);
}

#define MONTGOMERY_FACTOR_Q 609776071
#define MONTGOMERY_SHIFT_Q 32
#define MONTGOMERY_CONVERT_FACTOR_Q 14679236

#define MONTGOMERY_FACTOR_QBIG1 130021375
#define MONTGOMERY_SHIFT_QBIG1 32
#define MONTGOMERY_CONVERT_FACTOR_QBIG1 130087905

#define MONTGOMERY_FACTOR_QBIG2 520027903
#define MONTGOMERY_SHIFT_QBIG2 32
#define MONTGOMERY_CONVERT_FACTOR_QBIG2 4161600

/* Montgomery reduction
 * Input: x < Q*R, where R=2^k and Q<R
 * Output: m = x*R^{-1} % Q
 *
 * b = -Q^{-1} % R
 * t = ((x % R)*b) % R
 * m = (x + t * Q) / R */
static inline uint64_t montgomery_qbig(uint64_t a, uint64_t b, const uint64_t q,
                                       const uint64_t montgomery_factor,
                                       const uint64_t montgomery_shift) {
  uint64_t t;
  uint32_t x, y;

  t = a * b;
  x = t;
  y = ((uint64_t)x) * montgomery_factor;

  return con_sub((t + ((uint64_t)y) * q) >> montgomery_shift, q);
}

static inline uint64_t montgomery_q(uint64_t a, uint64_t b) {
  return montgomery_qbig(a, b, Q, MONTGOMERY_FACTOR_Q, MONTGOMERY_SHIFT_Q);
}

/* Barrett reduction
 * Input: x < 2^k
 * Output m = x % Q in [0, 2Q)
 *
 * b = floor(2^k/Q)
 * t = floor((x * b) / 2^k), where t is an estimation of x / Q
 * m = x - t * Q */
static inline uint64_t barrett(const uint64_t x, const uint64_t q,
                               const uint64_t barrett_factor,
                               const uint64_t barrett_shift) {
  return con_sub(x - ((x * barrett_factor) >> barrett_shift) * q, q);
}

#endif
