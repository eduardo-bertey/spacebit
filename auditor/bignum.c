/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * 128-bit integer arithmetic     *
 * ****************************** */

#include "bignum.h"
#include <stdint.h>

/* compute out=out+x^2 */
void num128_add_square(NUM_128 *out, uint64_t x) {
  uint64_t x0, x1, a0, a1, a2, b0, b1, b2;

  x *= (1 - 2 * (x >> 63));

  x0 = x & 0xffffffff;
  x1 = x >> 32;

  a0 = x0 * x0;
  a1 = (x0 * x1) << 1;
  a2 = x1 * x1;

  b0 = out->coeff[0] + (a0 & 0xffffffff);
  b1 = out->coeff[1] + (a0 >> 32) + (a1 & 0xffffffff) + (b0 >> 32);
  b2 = out->coeff[2] + (a1 >> 32) + (a2 & 0xffffffff) + (b1 >> 32);

  out->coeff[0] = b0 & 0xffffffff;
  out->coeff[1] = b1 & 0xffffffff;
  out->coeff[2] = b2 & 0xffffffff;
  out->coeff[3] += (a2 >> 32) + (b2 >> 32);
}

/* return a < b */
uint64_t num128_lt(const NUM_128 *a, const NUM_128 *b) {
  uint64_t c0, c1, c2, c3;

  c3 = a->coeff[3] - b->coeff[3];
  c2 = a->coeff[2] - b->coeff[2];
  c1 = a->coeff[1] - b->coeff[1];
  c0 = a->coeff[0] - b->coeff[0];

  return (c3 | (((1LL << 63) ^ (c3 | -c3)) &
                (c2 | (((1LL << 63) ^ (c2 | -c2)) &
                       (c1 | (((1LL << 63) ^ (c1 | -c1)) & c0)))))) >>
         63;
}
