/* Shim: runs ONE round of the MatRiCT-Au (n10m1) test.c flow —
 * keygen, mint, spend, verify, audit — and reports the results so a
 * Rust host can call it over FFI as a single function. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

static unsigned char seed[CRYPTO_BYTES];
static POLY_Q g[M + R][N];
static POLY_Q h[M];
static POLY_QBIG g_hat[M_HAT + V_HAT][N_HAT];

int matrict_demo(uint64_t *verify_out, uint64_t *l_real, uint64_t *l_audit,
                 uint64_t *amt0_real, uint64_t *amt0_audit,
                 uint64_t *amt1_real, uint64_t *amt1_audit) {
  static ACT a_in[N_SPENT];
  static ASK ask;
  static POLY_Q sk[N_SPENT][M];
  static POLY_Q ck[N_SPENT][M];
  static POLY_Q pk_out[S][N];
  static SPEND_OUT out;
  static POLY_Q cn_out[S][N], s;
  static POLY_QBIG t0[M_HAT], t1[V_1], t2[V_2], td[N_HAT - 1];

  uint64_t amt_out[S], l, l_au, amt_out_au[S];
  uint64_t i, amt, v;

  static int seeded = 0;
  if (!seeded) {
    srand(20261117);
    seeded = 1;
  }

  randombytes(seed, CRYPTO_BYTES);
  fastrandombytes_setseed_prv(seed);
  randombytes(seed, CRYPTO_BYTES);
  fastrandombytes_setseed_pub(seed);

  sample_mat_gr(g);
  sample_mat_gh(h);
  sample_mat_gbig(g_hat);

  /* the trapdoor of the designated observer */
  gentdrow(t0, t1, t2, td, g_hat);

  l = rand() % N_SPENT;

  for (i = 0; i < N_SPENT; i++) {
    keygen(a_in[i].pk[0], sk[i], g);
    amt = rand();
    mint(a_in[i].cn[0], ck[i], amt, g);
    if (i == l) {
      ask.amt = amt;
      amt_out[0] = rand() % amt;
      amt_out[1] = amt - amt_out[0];
    }
  }

  for (i = 0; i < M; i++) memcpy(ask.r + i, sk[l] + i, sizeof(POLY_Q));
  for (i = 0; i < M; i++) memcpy(ask.ck + i, ck[l] + i, sizeof(POLY_Q));

  for (i = 0; i < N; i++)
    memcpy(pk_out[0] + i, a_in[0].pk[0] + i, sizeof(POLY_Q));
  for (i = 0; i < N; i++)
    memcpy(pk_out[1] + i, a_in[1].pk[0] + i, sizeof(POLY_Q));

  spend(cn_out, &s, &out, a_in, l, &ask, pk_out, amt_out, g, h, g_hat, t0, t1,
        t2);
  v = verify(a_in, pk_out, cn_out, &out, &s, g, h, g_hat, t0, t1, t2);
  audit(&l_au, amt_out_au, &out, g_hat, td);

  *verify_out = v;
  *l_real = l;
  *l_audit = l_au;
  *amt0_real = amt_out[0];
  *amt0_audit = amt_out_au[0];
  *amt1_real = amt_out[1];
  *amt1_audit = amt_out_au[1];
  return 0;
}

/* Same round, but the transaction also signs an extra text (mixed into
 * the challenge hash). After a successful verify + audit, the text is
 * swapped for a tampered one and verify is run again: it must fail. */
void matrict_set_extra(const unsigned char *data, size_t len);

int matrict_demo_texto(const char *texto, const char *texto_alterado,
                       uint64_t *verify_ok, uint64_t *verify_alterado,
                       uint64_t *l_real, uint64_t *l_audit,
                       uint64_t *amt0_real, uint64_t *amt0_audit,
                       uint64_t *amt1_real, uint64_t *amt1_audit) {
  static ACT a_in[N_SPENT];
  static ASK ask;
  static POLY_Q sk[N_SPENT][M];
  static POLY_Q ck[N_SPENT][M];
  static POLY_Q pk_out[S][N];
  static SPEND_OUT out;
  static POLY_Q cn_out[S][N], s;
  static POLY_QBIG t0[M_HAT], t1[V_1], t2[V_2], td[N_HAT - 1];

  uint64_t amt_out[S], l, l_au, amt_out_au[S];
  uint64_t i, amt, v;

  static int seeded = 0;
  if (!seeded) {
    srand(20261117);
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
    amt = rand();
    mint(a_in[i].cn[0], ck[i], amt, g);
    if (i == l) {
      ask.amt = amt;
      amt_out[0] = rand() % amt;
      amt_out[1] = amt - amt_out[0];
    }
  }

  for (i = 0; i < M; i++) memcpy(ask.r + i, sk[l] + i, sizeof(POLY_Q));
  for (i = 0; i < M; i++) memcpy(ask.ck + i, ck[l] + i, sizeof(POLY_Q));

  for (i = 0; i < N; i++)
    memcpy(pk_out[0] + i, a_in[0].pk[0] + i, sizeof(POLY_Q));
  for (i = 0; i < N; i++)
    memcpy(pk_out[1] + i, a_in[1].pk[0] + i, sizeof(POLY_Q));

  /* spend + verify with the signed text in the challenge */
  matrict_set_extra((const unsigned char *)texto, strlen(texto));
  spend(cn_out, &s, &out, a_in, l, &ask, pk_out, amt_out, g, h, g_hat, t0, t1,
        t2);
  v = verify(a_in, pk_out, cn_out, &out, &s, g, h, g_hat, t0, t1, t2);
  audit(&l_au, amt_out_au, &out, g_hat, td);

  *verify_ok = v;
  *l_real = l;
  *l_audit = l_au;
  *amt0_real = amt_out[0];
  *amt0_audit = amt_out_au[0];
  *amt1_real = amt_out[1];
  *amt1_audit = amt_out_au[1];

  /* now present the same transaction with a tampered text */
  matrict_set_extra((const unsigned char *)texto_alterado,
                    strlen(texto_alterado));
  *verify_alterado =
      verify(a_in, pk_out, cn_out, &out, &s, g, h, g_hat, t0, t1, t2);

  /* leave the correct text set */
  matrict_set_extra((const unsigned char *)texto, strlen(texto));
  return 0;
}

/* Same as matrict_demo_texto, but the amounts are chosen by the caller:
 * the spent bill holds amt_in, amt_pago goes out as payment and the
 * change (amt_in - amt_pago) as the second output. */
int matrict_demo_pago(const char *texto, const char *texto_alterado,
                      uint64_t amt_in, uint64_t amt_pago,
                      uint64_t *verify_ok, uint64_t *verify_alterado,
                      uint64_t *l_real, uint64_t *l_audit,
                      uint64_t *amt0_real, uint64_t *amt0_audit,
                      uint64_t *amt1_real, uint64_t *amt1_audit) {
  static ACT a_in[N_SPENT];
  static ASK ask;
  static POLY_Q sk[N_SPENT][M];
  static POLY_Q ck[N_SPENT][M];
  static POLY_Q pk_out[S][N];
  static SPEND_OUT out;
  static POLY_Q cn_out[S][N], s;
  static POLY_QBIG t0[M_HAT], t1[V_1], t2[V_2], td[N_HAT - 1];

  uint64_t amt_out[S], l, l_au, amt_out_au[S];
  uint64_t i, amt, v;

  if (amt_pago > amt_in) return -1;

  static int seeded = 0;
  if (!seeded) {
    srand(20261117);
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
    amt = rand();
    if (i == l) amt = amt_in;
    mint(a_in[i].cn[0], ck[i], amt, g);
    if (i == l) {
      ask.amt = amt;
      amt_out[0] = amt_pago;
      amt_out[1] = amt_in - amt_pago;
    }
  }

  for (i = 0; i < M; i++) memcpy(ask.r + i, sk[l] + i, sizeof(POLY_Q));
  for (i = 0; i < M; i++) memcpy(ask.ck + i, ck[l] + i, sizeof(POLY_Q));

  for (i = 0; i < N; i++)
    memcpy(pk_out[0] + i, a_in[0].pk[0] + i, sizeof(POLY_Q));
  for (i = 0; i < N; i++)
    memcpy(pk_out[1] + i, a_in[1].pk[0] + i, sizeof(POLY_Q));

  matrict_set_extra((const unsigned char *)texto, strlen(texto));
  spend(cn_out, &s, &out, a_in, l, &ask, pk_out, amt_out, g, h, g_hat, t0, t1,
        t2);
  v = verify(a_in, pk_out, cn_out, &out, &s, g, h, g_hat, t0, t1, t2);
  audit(&l_au, amt_out_au, &out, g_hat, td);

  *verify_ok = v;
  *l_real = l;
  *l_audit = l_au;
  *amt0_real = amt_out[0];
  *amt0_audit = amt_out_au[0];
  *amt1_real = amt_out[1];
  *amt1_audit = amt_out_au[1];

  matrict_set_extra((const unsigned char *)texto_alterado,
                    strlen(texto_alterado));
  *verify_alterado =
      verify(a_in, pk_out, cn_out, &out, &s, g, h, g_hat, t0, t1, t2);

  matrict_set_extra((const unsigned char *)texto, strlen(texto));
  return 0;
}
