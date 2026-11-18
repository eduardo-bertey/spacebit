/* ************************************************************************
 * Prueba de "firma falsa" y de limites de rango para el esquema Au.
 *
 * Compilar con -DN_SPENT=<anillo> (100 / 500 / 1000). El tamano del anillo
 * es una constante de compilacion; por eso el workflow genera un .exe por
 * rango. El argumento <anillo> de la linea de comandos solo se usa para
 * avisar si no coincide con el binario.
 *
 * Uso:
 *   au_firma_falsa.exe <anillo> <l> <monto> <pago> <modo> [semilla] [delta] [pasos]
 *
 *   modo 0 honesto      - prueba limpia, no se toca nada.
 *   modo 1 pago_falso   - se firma bien y luego se altera zc[0] de la prueba.
 *   modo 2 fuera_rango  - se firma bien y luego se sustituye la entrada del
 *                         anillo donde estaba el firmante (el firmante ya no
 *                         esta en el anillo que ve el verificador).
 *   modo 3 clave_falsa  - se usa la clave secreta de otro miembro del anillo.
 *   modo 4 barrido      - barre pagos alrededor del limite "firmas/100" y
 *                         repite el modo 1 para cada uno.
 *   modo 5 monto_falso  - se altera c[0] de la prueba (monto sellado).
 *
 * Salida: una linea CSV con V_OK (verificacion de la prueba original) y
 * V_FALSO (verificacion de la prueba manipulada), mas lo que recupera audit.
 * ************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "audit.h"
#include "cpucycles.h"
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

/* Buffers estaticos: con anillo 1000 los arreglos locales desbordan la pila. */
static unsigned char seed[CRYPTO_BYTES];
static POLY_Q g[M + R][N];
static POLY_Q h[M];
static POLY_QBIG g_hat[M_HAT + V_HAT][N_HAT];

static ACT a_in[N_SPENT];
static ASK ask;
static POLY_Q sk[N_SPENT][M];
static POLY_Q ck[N_SPENT][M];
static POLY_Q pk_out[S][N];
static uint64_t amt_out[S], amt_out_au[S];
static SPEND_OUT out, out_falso;
static POLY_Q cn_out[S][N], s;
static POLY_QBIG t0[M_HAT], t1[V_1], t2[V_2], td[N_HAT - 1];

/* Regla pedida: valor maximo = cantidad de firmas / 100 (minimo 1). */
static uint64_t cap_seguro(void) {
  uint64_t c = (uint64_t)N_SPENT / 100ULL;
  return c ? c : 1ULL;
}

static void usar_clave(uint64_t lsk) {
  uint64_t i;
  for (i = 0; i < M; i++) {
    memcpy(ask.r + i, sk[lsk] + i, sizeof(POLY_Q));
    memcpy(ask.ck + i, ck[lsk] + i, sizeof(POLY_Q));
  }
  for (i = 0; i < N; i++) {
    memcpy(pk_out[0] + i, a_in[0].pk[0] + i, sizeof(POLY_Q));
    memcpy(pk_out[1] + i, a_in[1].pk[0] + i, sizeof(POLY_Q));
  }
}

static void run_round(uint64_t l_claim, uint64_t monto, uint64_t pago, int modo,
                      uint64_t delta, unsigned semilla) {
  uint64_t i, l, lsk, amt, v_ok, v_falso, l_au;
  int fuera;

  l = l_claim % (uint64_t)N_SPENT;
  fuera = (l_claim >= (uint64_t)N_SPENT);

  /* Evitar underflow al armar el cambio; la falsedad se introduce despues. */
  if (pago > monto)
    monto = pago;

  srand(semilla);
  randombytes(seed, CRYPTO_BYTES);
  fastrandombytes_setseed_prv(seed);
  randombytes(seed, CRYPTO_BYTES);
  fastrandombytes_setseed_pub(seed);

  sample_mat_gr(g);
  sample_mat_gh(h);
  sample_mat_gbig(g_hat);
  gentdrow(t0, t1, t2, td, g_hat);

  for (i = 0; i < N_SPENT; i++) {
    keygen(a_in[i].pk[0], sk[i], g);

    amt = (i == l) ? monto
                   : (((uint64_t)rand() << 32) ^ (uint64_t)rand());
    mint(a_in[i].cn[0], ck[i], amt, g);

    if (i == l) {
      ask.amt = amt;
      amt_out[0] = pago;
      amt_out[1] = amt - pago;
    }
  }

  lsk = (modo == 3) ? (l + 1) % (uint64_t)N_SPENT : l;
  usar_clave(lsk);

  spend(cn_out, &s, &out, a_in, l, &ask, pk_out, amt_out, g, h, g_hat, t0, t1,
        t2);
  v_ok = verify(a_in, pk_out, cn_out, &out, &s, g, h, g_hat, t0, t1, t2);

  memcpy(&out_falso, &out, sizeof(SPEND_OUT));

  switch (modo) {
  case 1: /* pago alterado dentro de la prueba */
  case 4:
    out_falso.zc[0].poly[0] += delta;
    break;
  case 2: /* el firmante original desaparece del anillo presentado */
    memcpy(&a_in[l], &a_in[(l + 1) % (uint64_t)N_SPENT], sizeof(ACT));
    usar_clave(lsk);
    break;
  case 5: /* monto sellado alterado */
    out_falso.c[0].poly[0] += delta;
    break;
  default:
    break;
  }

  v_falso = verify(a_in, pk_out, cn_out, &out_falso, &s, g, h, g_hat, t0, t1,
                   t2);
  audit(&l_au, amt_out_au, &out_falso, g_hat, td);

  printf("MODO=%d ANILLO=%lld L_CLAIM=%llu L_USADO=%llu FUERA=%d MONTO=%llu "
         "PAGO=%llu CAP=%llu DELTA=%llu V_OK=%llu V_FALSO=%llu L_AU=%llu "
         "PAGO_AU=%llu CAMBIO_AU=%llu\n",
         modo, (long long)N_SPENT, (unsigned long long)l_claim,
         (unsigned long long)l, fuera, (unsigned long long)monto,
         (unsigned long long)pago, (unsigned long long)cap_seguro(),
         (unsigned long long)delta, (unsigned long long)v_ok,
         (unsigned long long)v_falso, (unsigned long long)l_au,
         (unsigned long long)amt_out_au[0],
         (unsigned long long)amt_out_au[1]);
}

static void barrido(uint64_t monto, uint64_t delta, unsigned semilla, int pasos) {
  uint64_t cap = cap_seguro();
  int k;

  for (k = 0; k < pasos; k++) {
    /* montos por debajo y por encima del limite firmas/100 */
    uint64_t pago = (cap * (uint64_t)(k + 1)) / 4ULL + 1ULL;
    uint64_t l = (uint64_t)k % (uint64_t)N_SPENT;
    run_round(l, monto, pago, 1, delta, semilla + (unsigned)k);
  }
}

int main(int argc, char **argv) {
  uint64_t anillo, l, monto, pago, delta = 1ULL;
  unsigned semilla = 12345U;
  int modo = 0, pasos = 6;

  if (argc < 6) {
    fprintf(stderr,
            "Uso: %s <anillo> <l> <monto> <pago> <modo> [semilla] [delta] "
            "[pasos]\n"
            "modos: 0 honesto, 1 pago_falso, 2 fuera_rango, 3 clave_falsa, "
            "4 barrido, 5 monto_falso\n",
            argv[0]);
    return 2;
  }

  anillo = strtoull(argv[1], NULL, 10);
  l = strtoull(argv[2], NULL, 10);
  monto = strtoull(argv[3], NULL, 10);
  pago = strtoull(argv[4], NULL, 10);
  modo = atoi(argv[5]);
  if (argc > 6)
    semilla = (unsigned)strtoul(argv[6], NULL, 10);
  if (argc > 7)
    delta = strtoull(argv[7], NULL, 10);
  if (argc > 8)
    pasos = atoi(argv[8]);

  printf("# ANILLO_COMPILADO=%lld ANILLO_PEDIDO=%llu M_SPENT=%lld S=%lld "
         "M=%lld R=%lld BETA=%lld CAP=firmas/100=%llu\n",
         (long long)N_SPENT, (unsigned long long)anillo, (long long)M_SPENT,
         (long long)S, (long long)M, (long long)R, (long long)BETA,
         (unsigned long long)cap_seguro());

  if (anillo != (uint64_t)N_SPENT)
    printf("# AVISO: pediste anillo %llu pero este .exe fue compilado con "
           "%lld; se usa el compilado.\n",
           (unsigned long long)anillo, (long long)N_SPENT);
  if (modo < 0 || modo > 5) {
    fprintf(stderr, "modo desconocido: %d\n", modo);
    return 2;
  }

  if (modo == 4)
    barrido(monto, delta, semilla, pasos);
  else
    run_round(l, monto, pago, modo, delta, semilla);

  printf("\nFin. Presione Enter para salir...\n");
  fflush(stdout);
  (void)getchar();
  return 0;
}
