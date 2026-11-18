/* RNG portable SOLO PARA TESTS.
 *
 * El randombytes.c original abre /dev/urandom y se cuelga en Windows si no
 * existe. Esta version usa xorshift64 alimentado por rand(), asi que los .exe
 * compilados con MSYS2/MinGW pueden ejecutarse en Windows sin cambios.
 * No es criptograficamente seguro: solo sirve para sembrar el PRG AES-CTR
 * del esquema de pruebas.
 */
#include "randombytes.h"

#include <stdlib.h>

static unsigned long long st = 0x9e3779b97f4a7c15ULL;
static int inicializado = 0;

int randombytes(unsigned char *x, unsigned long long xlen) {
  if (!inicializado) {
    st ^= (unsigned long long)rand();
    st ^= (unsigned long long)rand() << 16;
    st ^= (unsigned long long)rand() << 32;
    st ^= (unsigned long long)rand() << 48;
    if (!st)
      st = 0x9e3779b97f4a7c15ULL;
    inicializado = 1;
  }

  while (xlen-- > 0) {
    st ^= st << 13;
    st ^= st >> 7;
    st ^= st << 17;
    *x++ = (unsigned char)(st >> 33);
  }
  return 0;
}
