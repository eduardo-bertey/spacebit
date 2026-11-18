/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * Audit                          *
 * ****************************** */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "audit.h"
#include "comp.h"
#include "param.h"
#include "poly_mult.h"
#include "poly_qbig.h"
#include "poly_red.h"
#include "samplex.h"
#include "spend.h"

static const uint64_t rnd_t[T_DEC + 1] = {0,
                                          5542804679859416LL,
                                          11085609359718832LL,
                                          16628414039578248LL,
                                          22171218719437664LL,
                                          27714023399297080LL,
                                          33256828079156496LL,
                                          38799632759015912LL,
                                          44342437438875328LL,
                                          49885242118734744LL,
                                          55428046798594160LL,
                                          60970851478453576LL,
                                          66513656158312992LL,
                                          72056460838172408LL};

void audit(uint64_t *l, uint64_t *amt_out, const SPEND_OUT *in,
           const POLY_QBIG g_hat[][N_HAT], const POLY_QBIG *td) {
  POLY_QBIG c_prime;
  POLY_Q c_pp, c_rnd;
  POLY_Q m_prime;

  uint64_t i, j;
  uint64_t check;
  uint64_t tmp;

  /* c'=<td,B> */
  memcpy(&c_prime, (in->b) + N_HAT - 1, sizeof(POLY_QBIG));

  for (i = 0; i < N_HAT - 1; i++) {
    mult_plus_rqbig(&c_prime, td + i, (in->b) + i);
  }
  for (i = 0; i < D; i++) {
    c_prime.poly1[i] = red_short_qbig1(c_prime.poly1[i]);
    c_prime.poly2[i] = red_short_qbig2(c_prime.poly2[i]);
  }
  intt_qbig(&c_prime);

  check = 0;
  for (i = 0; i < D; i++) {
    /* CRT inverse */
    c_pp.poly[i] = c_prime.poly1[i] +
                   QBIG1 * montgomery_qbig(
                               c_prime.poly2[i] - c_prime.poly1[i] + QBIG2,
                               QBIG1_INVMOD_QBIG2, QBIG2,
                               MONTGOMERY_FACTOR_QBIG2, MONTGOMERY_SHIFT_QBIG2);

    /* m'=\bar{t}^{-1}*c'' */
    m_prime.poly[i] = 0;
    for (j = 1; j <= T_DEC; j++) {
      m_prime.poly[i] += ct_ge_u64(c_pp.poly[i] + (T_BAR >> 1), rnd_t[j]);
    }

    /* c''=Rnd_{\bar{t}}(c') */
    c_rnd.poly[i] = m_prime.poly[i] * T_BAR;

    /* mod \bar{t} */
    m_prime.poly[i] = con_sub(m_prime.poly[i], T_DEC);

    /* e'=C'-C'' */
    tmp = c_pp.poly[i] - c_rnd.poly[i];

    /* ||e'||_{\inf} ?< ||e||_{bnd,\inf}, m'' ?\in [0...2^{\tau}-1]^d */
    tmp = tmp * (1 - 2 * (tmp >> 63));
    check |=
        ct_ge_u64(tmp, E_BOUND) | ct_lt_s64((1 << TD_T) - 1, m_prime.poly[i]);
  }

  if (!check) {
    /* decomposition */
    *l = 0;
    amt_out[0] = 0;
    amt_out[1] = 0;

    for (i = 0; i < D; i++) {
      *l += (m_prime.poly[i] & 0x1) << i;
      amt_out[0] += ((m_prime.poly[i] >> 1) & 0x1) << i;
      amt_out[1] += ((m_prime.poly[i] >> 2) & 0x1) << i;
    }
  }
}
