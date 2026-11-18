/* Puente C para la app Rust del esquema Plus.
 * Corre UNA ronda completa (setup, keygen, mint, spend, verify) con montos
 * elegidos por el llamador y devuelve, ademas de la verificacion honesta,
 * dos intentos de falsificacion: serial revelado a cero y prueba alterada. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cpucycles.h"
#include "fastrandombytes.h"
#include "keygen.h"
#include "mint.h"
#include "param.h"
#include "poly_q.h"
#include "randombytes.h"
#include "sammat.h"
#include "setup.h"
#include "spend.h"
#include "verify.h"

static unsigned char seed[CRYPTO_BYTES];
static POLY_Q mat1[K + 3][N], mat2[K][3];
static POLY_QHAT mat3[K_HAT + 2 * N_SPENT + 4][N_HAT];

int matrict_plus_demo(uint64_t amt0, uint64_t amt1, uint64_t pago,
                      uint64_t *v_ok, uint64_t *v_sn_cero, uint64_t *v_falso,
                      uint64_t *l_out, uint64_t *pago_out, uint64_t *vuelto_out,
                      uint64_t *ciclos) {
  static ACT a_in[N_SPENT][M];
  static ASK ask[M];
  static PK_OUT pk_out[S];
  static uint64_t amt_out[S];
  static POLY_R sk[N_SPENT][M][N_BAR + K];
  static unsigned char sn[N_SPENT][M][CRYPTO_BYTES];
  static POLY_R ck[N_SPENT][M][N + K + 1];
  static uint64_t a_hat[N_SPENT][M][R];
  static SPEND_OUT spend_out, out_falso;
  static CN_OUT cn_out[S];
  static CK_OUT ck_out[S];
  static SN_OUT sn_out[M], sn_guardado[M];
  static unsigned char pp[CRYPTO_BYTES];
  static int seeded = 0;

  uint64_t i, l, a0, a1;
  long long c1, c2, c3, c4, c5;

  if (pago > amt0 + amt1)
    return -1;

  if (!seeded) {
    srand(20261118);
    seeded = 1;
  }

  randombytes(seed, CRYPTO_BYTES);
  fastrandombytes_setseed_prv(seed);
  randombytes(seed, CRYPTO_BYTES);
  fastrandombytes_setseed_pub(seed);
  setup(pp, seed);

  c1 = cpucycles();
  sample_mat1(mat1);
  sample_mat2(mat2);
  sample_mat3(mat3);
  c2 = cpucycles();

  l = rand() % N_SPENT;

  for (i = 0; i < N_SPENT; i++) {
    keygen(a_in[i][0].pk, sk[i][0], sn[i][0], mat1);
    keygen(a_in[i][1].pk, sk[i][1], sn[i][1], mat1);

    a0 = (i == l) ? amt0 : (((uint64_t)rand() << 32) ^ (uint64_t)rand());
    a1 = (i == l) ? amt1 : (((uint64_t)rand() << 32) ^ (uint64_t)rand());
    mint(a_in[i][0].cn, ck[i][0], a_hat[i][0], a0, mat1, mat2);
    mint(a_in[i][1].cn, ck[i][1], a_hat[i][1], a1, mat1, mat2);

    if (i == l) {
      ask[0].amt_in = a0;
      ask[1].amt_in = a1;
      amt_out[0] = pago;
      amt_out[1] = a0 + a1 - pago;
    }
  }

  for (i = 0; i < N_BAR + K; i++) {
    memcpy(ask[0].sk + i, sk[l][0] + i, sizeof(POLY_R));
    memcpy(ask[1].sk + i, sk[l][1] + i, sizeof(POLY_R));
  }
  memcpy(ask[0].sn, sn[l][0], CRYPTO_BYTES);
  memcpy(ask[1].sn, sn[l][1], CRYPTO_BYTES);
  for (i = 0; i < N + K + 1; i++) {
    memcpy(ask[0].ck + i, ck[l][0] + i, sizeof(POLY_R));
    memcpy(ask[1].ck + i, ck[l][1] + i, sizeof(POLY_R));
  }
  for (i = 0; i < N_BAR; i++) {
    memcpy(pk_out[0].pk + i, a_in[0][0].pk + i, sizeof(POLY_Q));
    memcpy(pk_out[1].pk + i, a_in[0][1].pk + i, sizeof(POLY_Q));
  }

  c3 = cpucycles();
  spend(&spend_out, cn_out, ck_out, sn_out, a_in, l, ask, pk_out, amt_out,
        mat1, mat2, mat3, pp);
  c4 = cpucycles();
  *v_ok = verify(a_in, pk_out, cn_out, sn_out, &spend_out, mat1, mat2, mat3,
                 pp);
  c5 = cpucycles();

  /* ataque 1: revelar el serial a cero despues de firmar */
  memcpy(sn_guardado, sn_out, sizeof(sn_guardado));
  memset(sn_out, 0, sizeof(sn_out));
  *v_sn_cero = verify(a_in, pk_out, cn_out, sn_out, &spend_out, mat1, mat2,
                      mat3, pp);
  memcpy(sn_out, sn_guardado, sizeof(sn_out));

  /* ataque 2: alterar la prueba despues de firmar */
  memcpy(&out_falso, &spend_out, sizeof(SPEND_OUT));
  out_falso.zc[0].poly[0] += 1;
  *v_falso = verify(a_in, pk_out, cn_out, sn_out, &out_falso, mat1, mat2, mat3,
                    pp);

  *l_out = l;
  *pago_out = amt_out[0];
  *vuelto_out = amt_out[1];
  ciclos[0] = (uint64_t)(c2 - c1);
  ciclos[1] = (uint64_t)(c4 - c3);
  ciclos[2] = (uint64_t)(c5 - c4);
  return 0;
}
