/* Intento de levantar la dimension del anillo Au de 64 a 256.
 * Se compila con -DD=256 y se ve si el esquema firma/verifica.
 * Los tamanos de matrices, NTT y cotas estan calculados para D=64,
 * asi que se espera que falle (no es un cambio de un solo numero). */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "fastrandombytes.h"
#include "keygen.h"
#include "mint.h"
#include "param.h"
#include "poly_param.h"
#include "poly_q.h"
#include "randombytes.h"
#include "sammat.h"
#include "spend.h"
#include "verify.h"

static unsigned char seed[CRYPTO_BYTES];
static POLY_Q g[M + R][N], h[M];

int main(void) {
  POLY_Q pk, sk[M], cn, ck;
  uint64_t i, j;

  printf("=== INTENTO Au con D=%lld (el codigo usa D=64) ===\n", (long long)D);
  printf("q=%lld  M=%lld  BETA=%lld\n", (long long)Q, (long long)M, (long long)BETA);

  srand(20261121);
  randombytes(seed, CRYPTO_BYTES);
  fastrandombytes_setseed_prv(seed);
  randombytes(seed, CRYPTO_BYTES);
  fastrandombytes_setseed_pub(seed);
  sample_mat_gr(g);
  sample_mat_gh(h);

  keygen(&pk, sk, g);
  mint(&cn, &ck, 1000ULL, g);

  /* prueba rapida: los polinomios deben quedar en [0,q) */
  for (j = 0; j < D; j++) {
    if (pk.poly[j] >= (uint64_t)Q) {
      printf("FALLA: pk[%lld]=%llu fuera de rango (q=%lld)\n", (long long)j,
             (unsigned long long)pk.poly[j], (long long)Q);
      goto fin;
    }
  }
  (void)i;
  printf("keygen/mint terminaron sin desbordar\n");

fin:
  printf("Fin. Presione Enter para salir...\n");
  fflush(stdout);
  (void)getchar();
  return 0;
}
