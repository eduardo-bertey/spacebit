/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * poly multiplications           *
 * ****************************** */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "poly_mult.h"
#include "poly_param.h"
#include "poly_q.h"
#include "poly_qbig.h"

/* c=c+a*b on R_q */
void mult_plus_rq(POLY_Q *c, const POLY_Q *a, const POLY_Q *b) {
  POLY_Q tmp;
  uint64_t i;

  mult_rq(&tmp, a, b);

  for (i = 0; i < D; i++) {
    c->poly[i] += tmp.poly[i];
  }
}

/* c=c-a*b on R_q */
void mult_minus_rq(POLY_Q *c, const POLY_Q *a, const POLY_Q *b) {
  POLY_Q tmp;
  uint64_t i;

  mult_rq(&tmp, a, b);

  for (i = 0; i < D; i++) {
    c->poly[i] = c->poly[i] - tmp.poly[i] + Q;
  }
}

/* c=c+a*b on R_{\hat{q}} */
void mult_plus_rqbig(POLY_QBIG *c, const POLY_QBIG *a, const POLY_QBIG *b) {
  POLY_QBIG tmp;
  uint64_t i;

  mult_rqbig(&tmp, a, b);

  for (i = 0; i < D; i++) {
    c->poly1[i] += tmp.poly1[i];
    c->poly2[i] += tmp.poly2[i];
  }
}

/* c=c+a*b when pm=0 or c=c-a*b when pm=1 on R_{\hat{q}} */
void mult_plus_rqbig_pm(POLY_QBIG *c, const POLY_QBIG *a, const POLY_QBIG *b,
                        const uint64_t pm) {
  POLY_QBIG tmp;
  uint64_t i;

  mult_rqbig(&tmp, a, b);

  for (i = 0; i < D; i++) {
    c->poly1[i] += (1 - 2 * pm) * tmp.poly1[i] + QBIG1;
    c->poly2[i] += (1 - 2 * pm) * tmp.poly2[i] + QBIG2;
  }
}

/* out=a*b on R (no modulo reduction) */
void mult_r(POLY_Q *out, const POLY_Q *a, const POLY_Q *b) {
  uint64_t tmp[D << 1];

  uint64_t i, j;

  memset(tmp, 0, sizeof(tmp));
  for (i = 0; i < D; i++) {
    for (j = 0; j < D; j++) {
      tmp[i + j] += ((int64_t)a->poly[i]) * ((int64_t)b->poly[j]);
    }
  }

  for (i = 0; i < D; i++) {
    out->poly[i] = tmp[i] - tmp[i + D];
  }
}

/* out=f*(x-f) on R (no modulo reduction) */
void mult_fxf(POLY_Q *out, const POLY_Q *f, const POLY_Q *x) {
  POLY_Q fx;

  uint64_t i;

  for (i = 0; i < D; i++) {
    fx.poly[i] = x->poly[i] - f->poly[i];
  }

  mult_r(out, f, &fx);
}
