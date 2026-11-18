/* Falsificador empirico: ¿puede un atacante usar valores "altos" al azar
 * y pasar verify() en un anillo grande?
 *
 * Se arma UNA ronda honesta y despues se lanzan N intentos por estrategia,
 * siempre sobre la misma prueba base. Cada intento se verifica y se cuenta
 * cuantas veces el comprobador devuelve 1.
 *
 * Uso: falsificador.exe [intentos_por_estrategia] [semilla]
 */
#include <stdint.h>
#include <stdio.h>
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

static const char *NOMBRES[6] = {
    "serial_aleatorio_alto", "zc_random_cerca_del_tope",
    "zb_random_cerca_del_tope", "x_random", "clave_ajena", "anillo_sustituido"};

/* valor uniforme en [0, tope) representado en el anillo Z_q */
static uint64_t valor_alto(uint64_t tope) {
  uint64_t v = ((uint64_t)rand() << 31) ^ (uint64_t)rand();
  v %= tope;
  return v ? v : 1;
}

int main(int argc, char **argv) {
  uint64_t intentos = (argc > 1) ? strtoull(argv[1], NULL, 10) : 200;
  unsigned semilla = (argc > 2) ? (unsigned)strtoul(argv[2], NULL, 10) : 424242U;
  uint64_t aceptadas[6] = {0, 0, 0, 0, 0, 0};
  uint64_t i, j, t, v_base, estr, n;
  uint64_t amt_out[S], amt, l;

  srand(semilla);
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
    if (!amt)
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
  v_base = verify(a_in, pk_out, cn_out, &out, &s, g, h, g_hat, t0, t1, t2);

  printf("# ANILLO=%lld INTENTOS=%llu BASE_V=%llu TOPES zc=%lld zb=%lld\n",
         (long long)N_SPENT, (unsigned long long)intentos,
         (unsigned long long)v_base, (long long)ZC_NORM, (long long)ZB_NORM);

  for (estr = 0; estr < 6; estr++) {
    for (n = 0; n < intentos; n++) {
      memcpy(&out_try, &out, sizeof(SPEND_OUT));
      memcpy(&s_try, &s, sizeof(s));

      switch (estr) {
      case 0: /* serial "alto" pero distinto de cero: el parche no lo toca */
        for (i = 0; i < M_SPENT; i++)
          for (j = 0; j < D; j++)
            s_try[i].poly[j] = valor_alto((uint64_t)Q - 1);
        break;
      case 1: /* respuestas zc cerca del tope de norma */
        for (i = 0; i < M; i++)
          for (j = 0; j < D; j++)
            out_try.zc[i].poly[j] = Q - valor_alto((uint64_t)ZC_NORM);
        break;
      case 2: /* respuestas zb cerca del tope de norma */
        for (i = 0; i < M_HAT; i++)
          for (j = 0; j < D; j++)
            out_try.zb[i].poly[j] = Q - valor_alto((uint64_t)ZB_NORM);
        break;
      case 3: /* desafio x aleatorio */
        for (j = 0; j < D; j++)
          out_try.x.poly[j] = Q - valor_alto((uint64_t)Q - 1);
        break;
      case 4: /* clave secreta ajena: hay que re-firmar */
        for (i = 0; i < M; i++) {
          memcpy(ask.r + i, sk[(l + 1) % N_SPENT] + i, sizeof(POLY_Q));
          memcpy(ask.ck + i, ck[(l + 1) % N_SPENT] + i, sizeof(POLY_Q));
        }
        spend(cn_out, &s_try, &out_try, a_in, l, &ask, pk_out, amt_out, g, h,
              g_hat, t0, t1, t2);
        break;
      case 5: /* el firmante real se sustituye por otra cuenta del anillo */
        memcpy(&a_in[l], &a_in[(l + 1) % N_SPENT], sizeof(ACT));
        for (i = 0; i < N; i++) {
          memcpy(pk_out[0] + i, a_in[0].pk[0] + i, sizeof(POLY_Q));
          memcpy(pk_out[1] + i, a_in[1].pk[0] + i, sizeof(POLY_Q));
        }
        break;
      default:
        break;
      }

      if (verify(a_in, pk_out, cn_out, &out_try, &s_try, g, h, g_hat, t0, t1,
                 t2))
        aceptadas[estr]++;
    }
    printf("ESTRATEGIA=%d %-28s ACEPTADAS=%llu/%llu\n", (int)estr,
           NOMBRES[estr], (unsigned long long)aceptadas[estr],
           (unsigned long long)intentos);
  }

  (void)t;
  return 0;
}
