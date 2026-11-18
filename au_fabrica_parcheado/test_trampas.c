/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * test and benchmarks            *
 * ****************************** */

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

static unsigned char seed[CRYPTO_BYTES];
static POLY_Q g[M + R][N];
static POLY_Q h[M];
static POLY_QBIG g_hat[M_HAT + V_HAT][N_HAT];

#define NUM_ROUNDS 1

int main(int argc, char **argv) {
  static ACT a_in[N_SPENT];
  static ASK ask;

  static POLY_Q sk[N_SPENT][M];
  static POLY_Q ck[N_SPENT][M];
  static POLY_Q pk_out[S][N];
  uint64_t amt_out[S], l, l_au, amt_out_au[S];

  static SPEND_OUT out;
  static POLY_Q cn_out[S][N], s;

  uint64_t i, amt, t, v;

  long long cycle1, cycle2, cycle3, cycle4, cycle5, cycle6, cycle7;

  POLY_QBIG t0[M_HAT], t1[V_1], t2[V_2], td[N_HAT - 1];


  /* trampas: posicion, billete, pago (argv[1] elige; argv[2] = semilla) */
  static const struct { uint64_t l, bill, pago; } casos[6] = {
    {10, 1000ULL, 1001ULL},
    {20, 200ULL, 300ULL},
    {30, 5000ULL, 2000ULL},
    {40, 7777ULL, 1111ULL},
    {50, 500ULL, 300ULL},
    {60, 500ULL, 300ULL}
  };
  int caso = (argc > 1) ? atoi(argv[1]) : 0;
  srand(1000 + caso * 131 + ((argc > 2) ? atoi(argv[2]) : 0) * 17);

  for (t = caso; t < caso + 1; t++) {
    randombytes(seed, CRYPTO_BYTES);
    fastrandombytes_setseed_prv(seed);

    randombytes(seed, CRYPTO_BYTES);
    fastrandombytes_setseed_pub(seed);
    cycle1 = cpucycles();
    sample_mat_gr(g);
    sample_mat_gh(h);
    sample_mat_gbig(g_hat);
    cycle2 = cpucycles();

    /* auditability */
    gentdrow(t0, t1, t2, td, g_hat);

    cycle3 = cpucycles();
    l = casos[t].l;
    if (caso == 4) l = N_SPENT - 1;
    if (caso == 5) { l = rand() % N_SPENT; }

    for (i = 0; i < N_SPENT; i++) {
      keygen(a_in[i].pk[0], sk[i], g);

      amt = (i == l) ? ((caso == 5) ? (uint64_t)(rand() % 1000000 + 1000) : casos[t].bill) : (uint64_t)rand();

      mint(a_in[i].cn[0], ck[i], amt, g);

      if (i == l) {
        ask.amt = amt;
        amt_out[0] = casos[t].pago;
        amt_out[1] = (caso == 4) ? 300ULL : amt - amt_out[0];
      }
    }

    {
      uint64_t lsk = (caso == 3) ? (l + 1) % N_SPENT : l;
      for (i = 0; i < M; i++) {
        memcpy(ask.r + i, sk[lsk] + i, sizeof(POLY_Q));
      }
      for (i = 0; i < M; i++) {
        memcpy(ask.ck + i, ck[lsk] + i, sizeof(POLY_Q));
      }
    }

    for (i = 0; i < N; i++) {
      memcpy(pk_out[0] + i, a_in[0].pk[0] + i, sizeof(POLY_Q));
    }
    for (i = 0; i < N; i++) {
      memcpy(pk_out[1] + i, a_in[1].pk[0] + i, sizeof(POLY_Q));
    }

    cycle4 = cpucycles();
    spend(cn_out, &s, &out, a_in, l, &ask, pk_out, amt_out, g, h, g_hat, t0, t1,
          t2);
    cycle5 = cpucycles();
    if (caso == 2) { out.fr[0].poly[0] += 1; }
    v = verify(a_in, pk_out, cn_out, &out, &s, g, h, g_hat, t0, t1, t2);
    cycle6 = cpucycles();
    audit(&l_au, amt_out_au, &out, g_hat, td);
    cycle7 = cpucycles();

    printf("TRAMPA %d v=%llu l=%llu l_au=%llu pago=%llu pago_au=%llu cambio=%llu cambio_au=%llu\n",
           caso, v, l, l_au, amt_out[0], amt_out_au[0], amt_out[1], amt_out_au[1]);
  }
}
