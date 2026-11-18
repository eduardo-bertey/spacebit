/* SHAKE-256 (FIPS 202) in plain C: Keccak-f[1600] sponge, rate 136 bytes.
 * Verified against Python hashlib.shake_256 (see shake_test.c).
 * Public-domain style code written for this project. */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static uint64_t rotl64(uint64_t x, int n) {
  return n == 0 ? x : ((x << n) | (x >> (64 - n)));
}

static const uint64_t RC[24] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
    0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
    0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
    0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
    0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
    0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
    0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
    0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL};

/* rotation offsets r[x][y], flattened as x + 5*y */
static const int RHO[25] = {0,  1,  62, 28, 27, 36, 44, 6,  55, 20,
                            3,  10, 43, 25, 39, 41, 45, 15, 21, 8,
                            18, 2,  61, 56, 14};

static void keccakf1600(uint64_t a[25]) {
  for (int round = 0; round < 24; round++) {
    /* theta */
    uint64_t c[5], d[5];
    for (int x = 0; x < 5; x++)
      c[x] = a[x] ^ a[x + 5] ^ a[x + 10] ^ a[x + 15] ^ a[x + 20];
    for (int x = 0; x < 5; x++)
      d[x] = c[(x + 4) % 5] ^ rotl64(c[(x + 1) % 5], 1);
    for (int x = 0; x < 5; x++)
      for (int y = 0; y < 5; y++) a[x + 5 * y] ^= d[x];
    /* rho + pi */
    uint64_t b[25];
    for (int x = 0; x < 5; x++)
      for (int y = 0; y < 5; y++)
        b[y + 5 * ((2 * x + 3 * y) % 5)] = rotl64(a[x + 5 * y], RHO[x + 5 * y]);
    /* chi */
    for (int x = 0; x < 5; x++)
      for (int y = 0; y < 5; y++)
        a[x + 5 * y] =
            b[x + 5 * y] ^ ((~b[(x + 1) % 5 + 5 * y]) & b[(x + 2) % 5 + 5 * y]);
    /* iota */
    a[0] ^= RC[round];
  }
}

#define RATE 136 /* bytes, for SHAKE-256 (capacity 512) */

static void xor_block(uint64_t a[25], const unsigned char *p, size_t len) {
  for (size_t i = 0; i < len; i++)
    a[i / 8] ^= (uint64_t)p[i] << (8 * (i % 8));
}

int SHAKE256(unsigned char *output, size_t outputByteLen,
             const unsigned char *input, size_t inputByteLen) {
  uint64_t a[25];
  memset(a, 0, sizeof(a));

  while (inputByteLen >= RATE) {
    xor_block(a, input, RATE);
    keccakf1600(a);
    input += RATE;
    inputByteLen -= RATE;
  }
  /* last block: delimited suffix 1111 (0x1F) + pad10*1 */
  unsigned char last[RATE];
  memset(last, 0, sizeof(last));
  memcpy(last, input, inputByteLen);
  last[inputByteLen] ^= 0x1F;
  last[RATE - 1] ^= 0x80;
  xor_block(a, last, RATE);
  keccakf1600(a);

  while (outputByteLen > 0) {
    size_t take = outputByteLen < RATE ? outputByteLen : RATE;
    for (size_t i = 0; i < take; i++)
      output[i] = (unsigned char)(a[i / 8] >> (8 * (i % 8)));
    output += take;
    outputByteLen -= take;
    if (outputByteLen > 0) keccakf1600(a);
  }
  return 0;
}
