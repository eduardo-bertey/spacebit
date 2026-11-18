#include <stdio.h>
#include <string.h>
#include "gentdrow.c"

static void build_entry(uint64_t p, POLY_QBIG *out, int mont) {
  uint64_t c1 = T_BAR_QBIG1, c2 = T_BAR_QBIG2, j;
  if (mont) {
    c1 = (uint64_t)(((unsigned __int128)c1 * ((1ULL << 32) % QBIG1)) % QBIG1);
    c2 = (uint64_t)(((unsigned __int128)c2 * ((1ULL << 32) % QBIG2)) % QBIG2);
  }
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
  int mont, bad;
  for (mont = 0; mont <= 1; mont++) {
    bad = 0;
    for (p = 1; p <= 99 && !bad; p++) {
      build_entry(p, &e, mont);
      for (j = 0; j < D; j++)
        if (e.poly1[j] != tg_0[p - 1].poly1[j] || e.poly2[j] != tg_0[p - 1].poly2[j]) { bad = 1; break; }
    }
    printf("variante mont=%d: %s\n", mont, bad ? "DIFIERE" : "COINCIDE 1..99");
    if (!bad) break;
  }
  /* diagnostico pos 1 */
  build_entry(1, &e, 0);
  printf("sin mont, pos1 poly1[0..2]: %llu %llu %llu | tabla: %llu\n",
         (unsigned long long)e.poly1[0], (unsigned long long)e.poly1[1],
         (unsigned long long)e.poly1[2], (unsigned long long)tg_0[0].poly1[0]);
  return 0;
}
