#include <stdio.h>
#include <string.h>
#include "gentdrow.c" /* para leer la tabla estatica tg_0 */

static void build_entry(uint64_t p, POLY_QBIG *out) {
  const uint64_t c1 = (uint64_t)(((unsigned __int128)T_BAR_QBIG1 * ((1ULL << 32) % QBIG1)) % QBIG1);
  const uint64_t c2 = (uint64_t)(((unsigned __int128)T_BAR_QBIG2 * ((1ULL << 32) % QBIG2)) % QBIG2);
  uint64_t j;
  for (j = 0; j < D; j++) {
    uint64_t bit = (p >> j) & 1ULL;
    out->poly1[j] = bit ? c1 : 0;
    out->poly2[j] = bit ? c2 : 0;
  }
  ntt_qbig(out);
}

int main(void) {
  POLY_QBIG e;
  uint64_t p, j;
  int bad;
  /* posiciones 1..99 contra la tabla */
  for (p = 1; p <= 99; p++) {
    build_entry(p, &e);
    bad = 0;
    for (j = 0; j < D; j++) {
      if (e.poly1[j] != tg_0[p - 1].poly1[j] || e.poly2[j] != tg_0[p - 1].poly2[j]) bad = 1;
    }
    if (bad) { printf("pos %llu: DIFIERE\n", (unsigned long long)p); return 1; }
  }
  printf("posiciones 1..99: TODAS COINCIDEN con la tabla\n");
  /* la entrada 99 (posicion 100) debe ser cero en el build de 500 */
  bad = 0;
  for (j = 0; j < D; j++) if (tg_0[99].poly1[j] || tg_0[99].poly2[j]) bad = 1;
  printf("tg_0[99] (posicion 100) es cero: %s\n", bad ? "NO" : "SI");
  return 0;
}
