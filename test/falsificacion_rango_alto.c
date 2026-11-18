/* Prueba independiente: falsificacion con valores FUERA del rango seguro.
 *
 * Concepto: el tope "recomendable" se calcula como firmas/100 (Au).
 * El firmante honesto se mantiene ahi para poder firmar. Un atacante puede
 * usar numeros MUCHO mas grandes que ese recomendado e intentar hacer pasar
 * la firma como valida. Este test arma una firma honesta y despues, sin
 * volver a firmar, le mete valores altos y pregunta al verificador.
 *
 * Anillo fijo de 100 llaves. Compilar con build.sh (anillo=100).
 */
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
static SPEND_OUT out, out_try;
static POLY_Q cn_out[S][N], s, s_try;
static POLY_QBIG t0[M_HAT], t1[V_1], t2[V_2], td[N_HAT - 1];

static uint64_t recomendado(void) {
  uint64_t v = (uint64_t)N_SPENT / 100ULL;
  return v ? v : 1;
}

static void armar(uint64_t l, uint64_t monto, uint64_t pago) {
  uint64_t i, amt, amt_out[S];
  for (i = 0; i < N_SPENT; i++) {
    keygen(a_in[i].pk[0], sk[i], g);
    amt = (i == l) ? monto : (((uint64_t)rand() << 32) ^ (uint64_t)rand());
    if (!amt) amt = 1;
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
}

int main(void) {
  uint64_t l, v, j, cap, mult, n, aceptadas;
  static const uint64_t factores[4] = {10ULL, 50ULL, 100ULL, 1000ULL};

  srand(20261118);
  randombytes(seed, CRYPTO_BYTES);
  fastrandombytes_setseed_prv(seed);
  randombytes(seed, CRYPTO_BYTES);
  fastrandombytes_setseed_pub(seed);
  sample_mat_gr(g);
  sample_mat_gh(h);
  sample_mat_gbig(g_hat);
  gentdrow(t0, t1, t2, td, g_hat);

  cap = recomendado();
  l = rand() % N_SPENT;
  armar(l, 1000000ULL, 456ULL);

  v = verify(a_in, pk_out, cn_out, &out, &s, g, h, g_hat, t0, t1, t2);
  printf("=== FALSIFICACION FUERA DE RANGO (anillo=%lld) ===\n",
         (long long)N_SPENT);
  printf("tope recomendado (firmas/100) = %llu\n", (unsigned long long)cap);
  printf("firma honesta: v=%llu\n", (unsigned long long)v);

  for (mult = 0; mult < 4; mult++) {
    uint64_t valor = cap * factores[mult];
    aceptadas = 0;
    for (n = 0; n < 100; n++) {
      memcpy(&out_try, &out, sizeof(SPEND_OUT));
      memcpy(&s_try, &s, sizeof(s));
      /* mete valores altos, fuera del rango del honesto, en zc y serial */
      for (j = 0; j < M; j++)
        out_try.zc[j].poly[0] = Q - (valor % Q ? valor % Q : 1);
      for (j = 0; j < D; j++)
        s_try.poly[j] = (valor + j + 1) % (Q - 1) + 1;
      if (verify(a_in, pk_out, cn_out, &out_try, &s_try, g, h, g_hat, t0, t1,
                 t2))
        aceptadas++;
    }
    printf("valor=%llu (x%llu del recomendado): aceptadas=%llu/100\n",
           (unsigned long long)valor, (unsigned long long)factores[mult],
           (unsigned long long)aceptadas);
  }

  printf("Fin. Presione Enter para salir...\n");
  fflush(stdout);
  (void)getchar();
  return 0;
}
