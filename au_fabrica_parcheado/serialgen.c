/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * SerialGen                      *
 * ****************************** */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "param.h"
#include "poly_mult.h"
#include "poly_q.h"
#include "poly_red.h"
#include "sampleb.h"
#include "serialgen.h"

void serialgen(POLY_Q *s, const POLY_Q *sk, const POLY_Q *h) {
  uint64_t i, j;
  POLY_Q r[M];

  /* ntt */
  for (i = 0; i < M; i++) {
    for (j = 0; j < D; j++) {
      r[i].poly[j] = sk[i].poly[j] + Q;
    }

    ntt_q(r + i);
  }

  /* c=H*r */
  memset(s, 0, sizeof(POLY_Q));

  for (i = 0; i < M; i++) {
    mult_plus_rq(s, h + i, r + i);
  }

  for (i = 0; i < D; i++) {
    s->poly[i] = red_short_q(s->poly[i]);
  }
}
