/* Hooks de prueba para la app Rust: ejercitan el parche imagen-cero y dos
 * intentos de falsificacion sobre una ronda real de MatRiCT-Au.
 * No se usan en la app de produccion; build.rs los enlaza igualmente. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "audit.h"
#include "fastrandombytes.h"
#include "gentdrow.h"
#include "keygen.h"
#include "mint.h"
#include "param.h"
#include "poly_q.h"
#include "poly_qbig.h"
#include "randombytes.h"
#include "sammat.h"
#include "spend.h"
#include "verify.h"

static int prueba_round(int modo, uint64_t *v_ok, uint64_t *v_manipulado) {
  static unsigned char seed[CRYPTO_BYTES];
  static POLY_Q g[M + R][N];
  static POLY_Q h[M];
  static POLY_QBIG g_hat[M_HAT + V_HAT][N_HAT];
  static ACT a_in[N_SPENT];
  static ASK ask;
  static POLY_Q sk[N_SPENT][M];
  static POLY_Q ck[N_SPENT][M];
  static POLY_Q pk_out[S][N];
  static SPEND_OUT out;
  static POLY_Q cn_out[S][N], s;
  static POLY_QBIG t0[M_HAT], t1[V_1], t2[V_2], td[N_HAT - 1];
  static int seeded = 0;

  uint64_t amt_out[S], l, i, amt;

  if (!seeded) {
    srand(20261118);
    seeded = 1;
  }

  randombytes(seed, CRYPTO_BYTES);
  fastrandombytes_setseed_prv(seed);
  randombytes(seed, CRYPTO_BYTES);
  fastrandombytes_setseed_pub(seed);

  sample_mat_gr(g);
  sample_mat_gh(h);
  sample_mat_gbig(g_hat);
  gentdrow(t0, t1, t2, td, g_hat);

  l = rand() % N_SPENT;

  for (i = 0; i < N_SPENT; i++) {
    keygen(a_in[i].pk[0], sk[i], g);
    amt = (uint64_t)rand();
    if (amt == 0)
      amt = 1;
    mint(a_in[i].cn[0], ck[i], amt, g);
    if (i == l) {
      ask.amt = amt;
      amt_out[0] = (uint64_t)rand() % amt;
      amt_out[1] = amt - amt_out[0];
    }
  }

  for (i = 0; i < M; i++) {
    memcpy(ask.r + i, sk[l] + i, sizeof(POLY_Q));
    memcpy(ask.ck + i, ck[l] + i, sizeof(POLY_Q));
  }
  for (i = 0; i < N; i++) {
    memcpy(pk_out[0] + i, a_in[0].pk[0] + i, sizeof(POLY_Q));
    memcpy(pk_out[1] + i, a_in[1].pk[0] + i, sizeof(POLY_Q));
  }

  spend(cn_out, &s, &out, a_in, l, &ask, pk_out, amt_out, g, h, g_hat, t0, t1,
        t2);
  *v_ok = verify(a_in, pk_out, cn_out, &out, &s, g, h, g_hat, t0, t1, t2);

  switch (modo) {
  case 0: /* serial/imagen todo cero: debe rechazarlo el parche */
    memset(&s, 0, sizeof(s));
    break;
  case 1: /* prueba alterada despues de firmar */
    out.zc[0].poly[0] += 1;
    break;
  case 2: /* el firmante ya no esta en el anillo presentado */
    memcpy(&a_in[l], &a_in[(l + 1) % N_SPENT], sizeof(ACT));
    break;
  default:
    break;
  }

  *v_manipulado =
      verify(a_in, pk_out, cn_out, &out, &s, g, h, g_hat, t0, t1, t2);
  return 0;
}

int matrict_test_serial_cero(uint64_t *v_ok, uint64_t *v_cero) {
  return prueba_round(0, v_ok, v_cero);
}

int matrict_test_firma_falsa(uint64_t *v_ok, uint64_t *v_falso) {
  return prueba_round(1, v_ok, v_falso);
}

int matrict_test_fuera_rango(uint64_t *v_ok, uint64_t *v_falso) {
  return prueba_round(2, v_ok, v_falso);
}
