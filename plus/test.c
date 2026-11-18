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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static unsigned char seed[CRYPTO_BYTES];
static POLY_Q mat1[K + 3][N], mat2[K][3];
static POLY_QHAT mat3[K_HAT + 2 * N_SPENT + 4][N_HAT];

#define NUM_ROUNDS 20

int main() {
  static ACT a_in[N_SPENT][M];
  uint64_t l;
  static ASK ask[M];
  static PK_OUT pk_out[S];
  static uint64_t amt_out[S];

  static POLY_R sk[N_SPENT][M][N_BAR + K];
  static unsigned char sn[N_SPENT][M][CRYPTO_BYTES];
  static POLY_R ck[N_SPENT][M][N + K + 1];
  static uint64_t a_hat[N_SPENT][M][R];

  static SPEND_OUT spend_out;
  static CN_OUT cn_out[S];
  static CK_OUT ck_out[S];

  static unsigned char pp[CRYPTO_BYTES];

  uint64_t i, amt0, amt1, t, v, ok = 0;

  static SN_OUT sn_out[M];

  long long cycle1, cycle2, cycle3, cycle4, cycle5;

  srand(time(NULL));

  printf("=== PLUS ===\n");
  printf("anillo=%lld  M=%lld  S=%lld  rondas=%d\n", (long long)N_SPENT,
         (long long)M, (long long)S, NUM_ROUNDS);

  for (t = 0; t < NUM_ROUNDS; t++) {
    randombytes(seed, CRYPTO_BYTES);
    fastrandombytes_setseed_prv(seed);
    randombytes(seed, CRYPTO_BYTES);
    fastrandombytes_setseed_pub(seed);

    setup(pp, seed);

    cycle1 = cpucycles();
    sample_mat1(mat1);
    sample_mat2(mat2);
    sample_mat3(mat3);
    cycle2 = cpucycles();

    l = rand() % N_SPENT;

    for (i = 0; i < N_SPENT; i++) {
      keygen(a_in[i][0].pk, sk[i][0], sn[i][0], mat1);
      keygen(a_in[i][1].pk, sk[i][1], sn[i][1], mat1);

      amt0 = ((((uint64_t)rand()) << 32) + rand()) % (1LL << 63);
      mint(a_in[i][0].cn, ck[i][0], a_hat[i][0], amt0, mat1, mat2);
      amt1 = ((((uint64_t)rand()) << 32) + rand()) % (1LL << 63);
      mint(a_in[i][1].cn, ck[i][1], a_hat[i][1], amt1, mat1, mat2);

      if (i == l) {
        ask[0].amt_in = amt0;
        ask[1].amt_in = amt1;
        amt_out[0] = rand() % (amt0 + amt1);
        amt_out[1] = amt0 + amt1 - amt_out[0];
      }
    }

    for (i = 0; i < N_BAR + K; i++) {
      memcpy((ask[0].sk) + i, sk[l][0] + i, sizeof(POLY_R));
      memcpy((ask[1].sk) + i, sk[l][1] + i, sizeof(POLY_R));
    }
    memcpy(ask[0].sn, sn[l][0], CRYPTO_BYTES);
    memcpy(ask[1].sn, sn[l][1], CRYPTO_BYTES);
    for (i = 0; i < N + K + 1; i++) {
      memcpy((ask[0].ck) + i, ck[l][0] + i, sizeof(POLY_R));
      memcpy((ask[1].ck) + i, ck[l][1] + i, sizeof(POLY_R));
    }

    for (i = 0; i < N_BAR; i++) {
      memcpy((pk_out[0].pk) + i, (a_in[0][0].pk) + i, sizeof(POLY_Q));
      memcpy((pk_out[1].pk) + i, (a_in[0][1].pk) + i, sizeof(POLY_Q));
    }

    cycle3 = cpucycles();
    spend(&spend_out, cn_out, ck_out, sn_out, a_in, l, ask, pk_out, amt_out,
          mat1, mat2, mat3, pp);
    cycle4 = cpucycles();
    v = verify(a_in, pk_out, cn_out, sn_out, &spend_out, mat1, mat2, mat3, pp);
    if (v)
      ok++;
    cycle5 = cpucycles();
    printf("\n--- ronda %llu ---\n", (unsigned long long)(t + 1));
    printf("verificacion : %s (v=%llu)\n", v ? "OK" : "FALLA",
           (unsigned long long)v);
    printf("firmante     : %llu\n", (unsigned long long)l);
    printf("billetes     : %llu + %llu = %llu\n", (unsigned long long)amt0,
           (unsigned long long)amt1, (unsigned long long)(amt0 + amt1));
    printf("pago/vuelto  : %llu / %llu\n", (unsigned long long)amt_out[0],
           (unsigned long long)amt_out[1]);
    printf("ciclos CPU   : setup=%lld  firmar=%lld  verificar=%lld\n",
           cycle2 - cycle1, cycle4 - cycle3, cycle5 - cycle4);
  }

  printf("\nRESUMEN: %llu/%d rondas OK\n", (unsigned long long)ok, NUM_ROUNDS);
  printf("Fin. Presione Enter para salir...\n");
  fflush(stdout);
  (void)getchar();
  return 0;
}
