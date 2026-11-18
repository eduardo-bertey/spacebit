/* Un solo ejecutable con TODAS las pruebas del auditor (Au):
 *   1. ronda honesta + auditoria
 *   2. firma falsa (pago, anillo, clave, monto)
 *   3. falsificador con valores altos al azar
 *   4. observador: ¿ve o no los montos reales?
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

#define RONDAS_HONESTAS 5
#define INTENTOS 100

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

static void inicializar(void) {
  srand(20261118);
  randombytes(seed, CRYPTO_BYTES);
  fastrandombytes_setseed_prv(seed);
  randombytes(seed, CRYPTO_BYTES);
  fastrandombytes_setseed_pub(seed);
  sample_mat_gr(g);
  sample_mat_gh(h);
  sample_mat_gbig(g_hat);
  gentdrow(t0, t1, t2, td, g_hat);
}

/* Arma una ronda completa. lsk != l usa la clave de otro (firma falsa). */
static void armar(uint64_t l, uint64_t billete, uint64_t pago, uint64_t lsk) {
  uint64_t i, amt, amt_out[S];

  for (i = 0; i < N_SPENT; i++) {
    keygen(a_in[i].pk[0], sk[i], g);
    amt = (i == l) ? billete : (((uint64_t)rand() << 32) ^ (uint64_t)rand());
    if (!amt)
      amt = 1;
    mint(a_in[i].cn[0], ck[i], amt, g);
    if (i == l) {
      ask.amt = amt;
      amt_out[0] = pago;
      amt_out[1] = amt - pago;
    }
  }
  for (i = 0; i < M; i++) {
    memcpy(ask.r + i, sk[lsk] + i, sizeof(POLY_Q));
    memcpy(ask.ck + i, ck[lsk] + i, sizeof(POLY_Q));
  }
  for (i = 0; i < N; i++) {
    memcpy(pk_out[0] + i, a_in[0].pk[0] + i, sizeof(POLY_Q));
    memcpy(pk_out[1] + i, a_in[1].pk[0] + i, sizeof(POLY_Q));
  }
  spend(cn_out, &s, &out, a_in, l, &ask, pk_out, amt_out, g, h, g_hat, t0, t1,
        t2);
}

static uint64_t verificar(SPEND_OUT *prueba, POLY_Q *serial) {
  return verify(a_in, pk_out, cn_out, prueba, serial, g, h, g_hat, t0, t1, t2);
}

static void prueba_honesta(void) {
  uint64_t t, l, l_au, amt_au[S], ok = 0;

  printf("\n=== 1. RONDA HONESTA + AUDITORIA ===\n");
  for (t = 0; t < RONDAS_HONESTAS; t++) {
    l = rand() % N_SPENT;
    armar(l, 1000000ULL, 456ULL, l);
    v = verificar(&out, &s);
    if (v)
      ok++;
    audit(&l_au, amt_au, &out, g_hat, td);
    printf("ronda %llu: v=%llu  firmante=%llu/%llu  pago=%llu/%llu\n",
           (unsigned long long)(t + 1), (unsigned long long)v,
           (unsigned long long)l, (unsigned long long)l_au, 456ULL,
           (unsigned long long)amt_au[0]);
  }
  printf("resultado: %llu/%d OK\n", (unsigned long long)ok, RONDAS_HONESTAS);
}

static void prueba_firma_falsa(void) {
  uint64_t l, v, i;
  static const char *nombres[4] = {"pago alterado", "firmante fuera del anillo",
                                   "clave ajena", "monto alterado"};

  printf("\n=== 2. FIRMA FALSA ===\n");
  l = rand() % N_SPENT;
  armar(l, 1000000ULL, 456ULL, l);
  v = verificar(&out, &s);
  printf("base honesta: v=%llu\n", (unsigned long long)v);

  for (i = 0; i < 4; i++) {
    /* cada caso arranca de una ronda limpia; el caso 2 ya firma con clave ajena */
    armar(l, 1000000ULL, 456ULL, (i == 2) ? (l + 1) % N_SPENT : l);
    memcpy(&out_try, &out, sizeof(SPEND_OUT));
    switch (i) {
    case 0:
      out_try.zc[0].poly[0] += 1;
      break;
    case 1:
      memcpy(&a_in[l], &a_in[(l + 1) % N_SPENT], sizeof(ACT));
      break;
    case 3:
      out_try.c[0].poly[0] += 1;
      break;
    default:
      break;
    }
    v = verificar(&out_try, &s);
    printf("%-26s v=%llu %s\n", nombres[i], (unsigned long long)v,
           v ? "ACEPTO (mal)" : "rechazado (bien)");
  }
}

static uint64_t alto(uint64_t tope) {
  uint64_t v = ((uint64_t)rand() << 31) ^ (uint64_t)rand();
  v %= tope;
  return v ? v : 1;
}

static void prueba_falsificador(void) {
  uint64_t estr, n, aceptadas = 0, j, v;
  static const char *nombres[3] = {"serial alto", "zc cerca del tope",
                                   "x aleatorio"};

  printf("\n=== 3. FALSIFICADOR (%llu intentos por estrategia) ===\n",
         (unsigned long long)INTENTOS);
  {
    uint64_t l = rand() % N_SPENT;
    armar(l, 1000000ULL, 456ULL, l);
  }
  for (estr = 0; estr < 3; estr++) {
    aceptadas = 0;
    for (n = 0; n < INTENTOS; n++) {
      memcpy(&out_try, &out, sizeof(SPEND_OUT));
      memcpy(&s_try, &s, sizeof(s));
      switch (estr) {
      case 0:
        for (j = 0; j < D; j++)
          s_try.poly[j] = alto((uint64_t)Q - 1);
        break;
      case 1:
        for (j = 0; j < M; j++)
          out_try.zc[j].poly[0] = Q - alto((uint64_t)ZC_NORM);
        break;
      case 2:
        for (j = 0; j < D; j++)
          out_try.x.poly[j] = Q - alto((uint64_t)Q - 1);
        break;
      default:
        break;
      }
      v = verificar(&out_try, &s_try);
      if (v)
        aceptadas++;
    }
    printf("%-22s aceptadas=%llu/%llu\n", nombres[estr],
           (unsigned long long)aceptadas, (unsigned long long)INTENTOS);
  }
}

static void prueba_observador(void) {
  uint64_t l, l_au, amt_au[S], v;
  const uint64_t pagos[2] = {123456ULL, 999999ULL};
  int k;

  printf("\n=== 4. OBSERVADOR: ¿VE LOS MONTOS REALES? ===\n");
  printf("verify() no recibe ningun parametro de monto\n");
  for (k = 0; k < 2; k++) {
    l = rand() % N_SPENT;
    armar(l, 1000000ULL, pagos[k], l);
    v = verificar(&out, &s);
    audit(&l_au, amt_au, &out, g_hat, td);
    printf("pago real=%llu  vuelto real=%llu | observador: pago=%llu vuelto=%llu "
           "| ve los reales: %s (v=%llu)\n",
           pagos[k], 1000000ULL - pagos[k], (unsigned long long)amt_au[0],
           (unsigned long long)amt_au[1],
           (amt_au[0] == pagos[k] && amt_au[1] == 1000000ULL - pagos[k]) ? "SI"
                                                                        : "NO",
           (unsigned long long)v);
  }
}

int main(void) {
  printf("=== AUDITOR / Au: todas las pruebas (anillo=%lld) ===\n",
         (long long)N_SPENT);
  inicializar();
  prueba_honesta();
  prueba_firma_falsa();
  prueba_falsificador();
  prueba_observador();

  printf("\nFin. Presione Enter para salir...\n");
  fflush(stdout);
  (void)getchar();
  return 0;
}
