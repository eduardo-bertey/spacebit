/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * parse or sample C^d_{w,p}      *
 * ****************************** */

#include "samplex.h"
#include "fastrandombytes.h"
#include "param.h"
#include "poly_q.h"
#include <stdint.h>

void parse_hash(POLY_Q *x, const unsigned char *hash_output) {
  uint64_t i, j, boo, count = 0, x_pos, coeff;

  unsigned char *hash_output_head = hash_output + HASH_COEFF_LEN;
  uint64_t zero_pos[D - W];

  /* sample the positions of zeroes from the hash output
   * since it may generates two same positions, we need to do a rejection
   * sampling here */
  zero_pos[0] = (*(hash_output_head++)) & 63;
  x->poly[zero_pos[0]] = 0;

  for (i = 1; i < D - W; i++) {
    do {
      boo = 0;
      x_pos = (*(hash_output_head++)) & 63;

      for (j = 0; j < i; j++) {
        if (x_pos == zero_pos[j]) {
          boo = 1;
          break;
        }
      }
    } while (boo);

    zero_pos[i] = x_pos;
    x->poly[x_pos] = 0;
  }

  for (i = 0; i < D; i++) {
    boo = 1;

    for (j = 0; j < D - W; j++) {
      if (i == zero_pos[j]) {
        boo = 0;
        break;
      }
    }

    /* generate the non zero coordinates from the hash output */
    if (boo) {
      coeff = ((hash_output[count >> 1] >> ((count & 0x1) << 2)) & 0xf) - 8;

      if (coeff == 0) {
        coeff = 8;
      }

      x->poly[i] = coeff;
      count++;
    }
  }
}

void sample_x(POLY_Q *x) {
  unsigned char hash_output[HASH_OUTPUT_LEN];

  fastrandombytes_prv(hash_output, HASH_OUTPUT_LEN);
  parse_hash(x, hash_output);
}
