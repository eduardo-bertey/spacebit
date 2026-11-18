/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * generate g and compare ||g||   *
 * ****************************** */

#include "gnorm.h"
#include "bignum.h"
#include "param.h"
#include "poly_mult.h"
#include "poly_q.h"
#include <stdint.h>

uint64_t check_gnorm(POLY_Q *g_norm, const POLY_Q *f00, const POLY_Q *f0,
                     const POLY_Q *fr, const POLY_Q *x) {
  NUM_128 tg;
  NUM_128 norm;

  int64_t i, j;

  /* Escalado automatico: TG base (100 llaves) por N_SPENT/100 */
  {
    unsigned __int128 acarreo = 0;
    uint64_t base[4] = {TG_COEFF_0, TG_COEFF_1, TG_COEFF_2, TG_COEFF_3};
    uint64_t factor = (uint64_t)N_SPENT / 100ULL;
    int q;
    if (!factor)
      factor = 1;
    for (q = 0; q < 4; q++) {
      unsigned __int128 p = (unsigned __int128)base[q] * factor + acarreo;
      tg.coeff[q] = (uint64_t)p;
      acarreo = p >> 64;
    }
  }

  norm.coeff[0] = 0;
  norm.coeff[1] = 0;
  norm.coeff[2] = 0;
  norm.coeff[3] = 0;

  /* f_{c,1}(x-f_{c,1}},...,f_{out,r-1}^(S-1)(x-f_{out,r-1}^(S-1)) */
  for (i = 0; i < M_SPENT * (R - 1) + S * R; i++) {
    mult_fxf(g_norm + BETA + i, fr + i, x);

    for (j = 0; j < D; j++) {
      num128_add_square(&norm, g_norm[BETA + i].poly[j]);
    }
  }

  /* f_{0,0}(x-f_{0,0}),...,f_{k-1,beta-1}(x-f_{k-1,beta-1}) */
  mult_fxf(g_norm, f00, x);

  for (j = 0; j < D; j++) {
    num128_add_square(&norm, g_norm[0].poly[j]);
  }

  for (i = 1; i < BETA; i++) {
    mult_fxf(g_norm + i, f0 + i - 1, x);

    for (j = 0; j < D; j++) {
      num128_add_square(&norm, g_norm[i].poly[j]);
    }
  }

  if (num128_lt(&tg, &norm)) {
    return 1;
  }

  return 0;
}
