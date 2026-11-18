/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * 128-bit integer arithmetic     *
 * ****************************** */

#ifndef _BIGNUM_H
#define _BIGNUM_H

#include <stdint.h>

typedef struct {
  uint64_t coeff[4];
} NUM_128;

void num128_add_square(NUM_128 *out, uint64_t x);
uint64_t num128_lt(const NUM_128 *a, const NUM_128 *b);

#endif
