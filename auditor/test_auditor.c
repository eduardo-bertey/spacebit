/* Prueba del observador (audit) en el esquema Au:
 * ¿ve o no los montos reales de la transaccion?
 *
 * Se arman DOS gastos del mismo billete con repartos distintos. En cada
 * caso se compara lo que dice el que gasta con lo que recupera el auditor
 * usando la trampilla. El verificador, en cambio, no recibe ningun monto:
 * verify() solo ve cajas selladas y pruebas de bits. */
#include <stdint.h>
#include <stdio.h>
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
static ACT a_in[N_SPENT];
static ASK ask;
static POLY_Q sk[N_SPENT][M], ck[N_SPENT][M], pk_out[S][N];
static SPEND_OUT out;
static POLY_Q cn_out[S][N], s;
static POLY_QBIG t0[M_HAT], t1[V_1], t2[V_2], td[N_HAT - 1];

static void correr(uint64_t billete, uint64_t pago) {
  uint64_t amt_out[S], amt_au[S], l, l_au, i, amt, v;

  l = rand() % N_SPENT;
  for (i = 0; i < N_SPENT; i++) {
    keygen(a_in[i].pk[0], sk[i], g);
    amt = (i == l) ? billete : ((uint64_t)rand() << 32) ^ (uint64_t)rand();
    mint(a_in[i].cn[0], ck[i], amt, g);
    if (i == l) {
      ask.amt = amt;
      amt_out[0] = pago;
      amt_out[1] = amt - pago;
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
  v = verify(a_in, pk_out, cn_out, &out, &s, g, h, g_hat, t0, t1, t2);
  audit(&l_au, amt_au, &out, g_hat, td);

  printf("\n--- gasto con pago=%llu ---\n", (unsigned long long)pago);
  printf("verificacion        : %s (v=%llu)  <- verificador NO ve montos\n",
         v ? "OK" : "FALLA", (unsigned long long)v);
  printf("quien gasta dice    : cuenta %llu, pago %llu, vuelto %llu\n",
         (unsigned long long)l, (unsigned long long)amt_out[0],
         (unsigned long long)amt_out[1]);
  printf("auditor (trampilla): cuenta %llu, pago %llu, vuelto %llu\n",
         (unsigned long long)l_au, (unsigned long long)amt_au[0],
         (unsigned long long)amt_au[1]);
  printf("el observador ve los montos reales: %s\n",
         (l == l_au && amt_out[0] == amt_au[0] && amt_out[1] == amt_au[1])
             ? "SI"
             : "NO");
}

int main(void) {
  srand(20261118);

  randombytes(seed, CRYPTO_BYTES);
  fastrandombytes_setseed_prv(seed);
  randombytes(seed, CRYPTO_BYTES);
  fastrandombytes_setseed_pub(seed);

  sample_mat_gr(g);
  sample_mat_gh(h);
  sample_mat_gbig(g_hat);
  gentdrow(t0, t1, t2, td, g_hat);

  printf("=== PRUEBA DEL OBSERVADOR (Au) ===\n");
  printf("anillo=%lld  verify() no recibe ningun parametro de monto\n",
         (long long)N_SPENT);

  correr(1000000ULL, 123456ULL);
  correr(1000000ULL, 999999ULL);

  printf("\nFin. Presione Enter para salir...\n");
  fflush(stdout);
  (void)getchar();
  return 0;
}
