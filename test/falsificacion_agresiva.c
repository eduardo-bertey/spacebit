/* Prueba agresiva de falsificacion (anillo 100, concepto).
 *
 * Diferencia con falsificacion_rango_alto.c: aca NO solo se altera una firma
 * ya hecha. Se prueban metodos mas fuertes:
 *   0. zc en el borde de la norma (dentro del tope, pero cambiado)
 *   1. zb en el borde de la norma
 *   2. f0/fr en el borde de sus topes
 *   3. x (desafio) cambiado
 *   4. serial de OTRO usuario del anillo
 *   5. anillo sustituido (replay contra otra cuenta)
 *   6. prueba ARMADA DESDE CERO (sin spend), con coeficientes dentro de norma
 *
 * Cada estrategia reporta cuantas firmas el verificador acepto.
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
#include "serialgen.h"
#include "spend.h"
#include "verify.h"

#define INTENTOS 50

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

/* valor pequeno en Z_q: 0..lim o su negativo Q-v */
static uint64_t pequeno(uint64_t lim, int neg) {
  uint64_t v = ((uint64_t)rand() * 1103515245u + 12345u) % lim;
  if (!v) v = 1;
  return neg ? Q - v : v;
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

static void relleno_aleatorio(SPEND_OUT *o, POLY_Q *s) {
  uint64_t i, j;
  for (i = 0; i < N_HAT; i++)
    for (j = 0; j < D; j++) {
      o->b[i].poly1[j] = (uint64_t)rand() % QBIG1;
      o->b[i].poly2[j] = (uint64_t)rand() % QBIG2;
    }
  for (i = 0; i < N; i++)
    for (j = 0; j < D; j++) o->c[i].poly[j] = pequeno(BA, rand() & 1);
  for (j = 0; j < D; j++) o->x.poly[j] = pequeno(BA, rand() & 1);
  for (i = 0; i < BETA - 1; i++)
    for (j = 0; j < D; j++) o->f0[i].poly[j] = pequeno(BAP, rand() & 1);
  for (i = 0; i < M_SPENT * (R - 1) + S * R; i++)
    for (j = 0; j < D; j++) o->fr[i].poly[j] = pequeno(BRP, rand() & 1);
  for (i = 0; i < M_HAT; i++)
    for (j = 0; j < D; j++) o->zb[i].poly[j] = pequeno(ZB_NORM - 1, rand() & 1);
  for (i = 0; i < M; i++)
    for (j = 0; j < D; j++) o->zc[i].poly[j] = pequeno(ZC_NORM - 1, rand() & 1);
  for (i = 0; i < M_SPENT + 1; i++)
    for (j = 0; j < M; j++)
      for (uint64_t k = 0; k < D; k++)
        o->z[i][j].poly[k] = pequeno(Z0_NORM - 1, rand() & 1);
  for (i = 0; i < S; i++)
    for (j = 0; j < M; j++)
      for (uint64_t k = 0; k < D; k++)
        o->z_out[i][j].poly[k] = pequeno(ZC_NORM - 1, rand() & 1);
  for (j = 0; j < D; j++) s->poly[j] = pequeno(Q - 1, 0);
}

int main(void) {
  uint64_t l, v, i, j, k, n, aceptadas;
  static const char *nombres[7] = {
      "zc borde norma", "zb borde norma", "f0/fr borde", "x cambiado",
      "serial de otro", "anillo sustituido", "prueba desde cero"};

  srand(20261118);
  randombytes(seed, CRYPTO_BYTES);
  fastrandombytes_setseed_prv(seed);
  randombytes(seed, CRYPTO_BYTES);
  fastrandombytes_setseed_pub(seed);
  sample_mat_gr(g);
  sample_mat_gh(h);
  sample_mat_gbig(g_hat);
  gentdrow(t0, t1, t2, td, g_hat);

  l = rand() % N_SPENT;
  armar(l, 1000000ULL, 456ULL); /* spend() completa: el atacante PUEDE firmar */

  printf("=== FALSIFICACION AGRESIVA (anillo=%lld) ===\n", (long long)N_SPENT);
  v = verify(a_in, pk_out, cn_out, &out, &s, g, h, g_hat, t0, t1, t2);
  printf("spend completo = 1; firma honesta v=%llu\n", (unsigned long long)v);

  for (i = 0; i < 7; i++) {
    aceptadas = 0;
    for (n = 0; n < INTENTOS; n++) {
      memcpy(&out_try, &out, sizeof(SPEND_OUT));
      memcpy(&s_try, &s, sizeof(s));
      switch (i) {
      case 0:
        for (j = 0; j < M; j++)
          for (k = 0; k < D; k++)
            out_try.zc[j].poly[k] = Q - (ZC_NORM - 1 - (n % 3));
        break;
      case 1:
        for (j = 0; j < M_HAT; j++)
          for (k = 0; k < D; k++)
            out_try.zb[j].poly[k] = Q - (ZB_NORM - 1 - (n % 3));
        break;
      case 2:
        for (j = 0; j < BETA - 1; j++)
          for (k = 0; k < D; k++)
            out_try.f0[j].poly[k] = Q - (BAP - 1 - (n % 3));
        break;
      case 3:
        for (k = 0; k < D; k++)
          out_try.x.poly[k] = Q - (1 + n + k);
        break;
      case 4:
        serialgen(&s_try, sk[(l + 1) % N_SPENT], h);
        break;
      case 5:
        memcpy(&a_in[l], &a_in[(l + 1) % N_SPENT], sizeof(ACT));
        for (k = 0; k < N; k++) {
          memcpy(pk_out[0] + k, a_in[0].pk[0] + k, sizeof(POLY_Q));
          memcpy(pk_out[1] + k, a_in[1].pk[0] + k, sizeof(POLY_Q));
        }
        break;
      case 6:
        relleno_aleatorio(&out_try, &s_try);
        break;
      default:
        break;
      }
      if (verify(a_in, pk_out, cn_out, &out_try, &s_try, g, h, g_hat, t0, t1,
                 t2))
        aceptadas++;
      if (i == 5) { /* restaurar el anillo para las siguientes estrategias */
        keygen(a_in[l].pk[0], sk[l], g);
        mint(a_in[l].cn[0], ck[l], 1000000ULL, g);
        for (k = 0; k < N; k++) {
          memcpy(pk_out[0] + k, a_in[0].pk[0] + k, sizeof(POLY_Q));
          memcpy(pk_out[1] + k, a_in[1].pk[0] + k, sizeof(POLY_Q));
        }
      }
    }
    printf("%-22s aceptadas=%llu/%d\n", nombres[i],
           (unsigned long long)aceptadas, INTENTOS);
  }

  printf("Fin. Presione Enter para salir...\n");
  fflush(stdout);
  (void)getchar();
  return 0;
}
