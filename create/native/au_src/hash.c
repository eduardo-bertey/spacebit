/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * hash function                  *
 * ****************************** */

#include "hash.h"
#include "littleendian.h"
#include "param.h"
#include "poly_q.h"
#include "poly_qbig.h"
#include "poly_red.h"
#include "samplex.h"
#include "spend.h"
#include <stdint.h>

#include <libXKCP.a.headers/SimpleFIPS202.h>

#include <stdlib.h>
#include <string.h>

/* --- extra text mixed into the challenge (patch for the demo) ---
 * Whatever bytes are set here are appended to the hashed input, so the
 * ring signature is bound to that text: spend and verify must present
 * the same text or the challenge differs and verification fails. */
static unsigned char extra_buf[8192];
static size_t extra_len = 0;

void matrict_set_extra(const unsigned char *data, size_t len) {
  if (len > sizeof(extra_buf)) len = sizeof(extra_buf);
  memcpy(extra_buf, data, len);
  extra_len = len;
}

void hash(POLY_Q *x, const POLY_QBIG *a, const POLY_QBIG *b, const POLY_Q *c,
          const POLY_Q *d, const POLY_Q e_0[][N], const POLY_Q *f_0,
          const POLY_Q g_out[][N], const POLY_Q *s, const ACT *a_in,
          const POLY_Q pk_out[][N], const POLY_Q cn_out[][N]) {
  uint64_t i, j, k;

  static unsigned char hash_input[HASH_INPUT_LEN * HASH_INPUT_BYTES];
  unsigned char *hash_input_head = hash_input;

  unsigned char hash_output[HASH_OUTPUT_LEN];

  /* we convert each coordinates to a 32-bit integer within [0,q-1], where q can
   * be Q, QBIG1, or QBIG2 */
  for (i = 0; i < N_HAT; i++) {
    for (j = 0; j < D; j++) {
      store_32(hash_input_head, a[i].poly1[j]);
      store_32(hash_input_head + HASH_INPUT_BYTES, a[i].poly2[j]);

      hash_input_head += HASH_INPUT_BYTES * 2;
    }
  }

  for (i = 0; i < N_HAT; i++) {
    for (j = 0; j < D; j++) {
      store_32(hash_input_head, b[i].poly1[j]);
      store_32(hash_input_head + HASH_INPUT_BYTES, b[i].poly2[j]);

      hash_input_head += HASH_INPUT_BYTES * 2;
    }
  }

  for (i = 0; i < N; i++) {
    for (j = 0; j < D; j++) {
      store_32(hash_input_head, c[i].poly[j]);

      hash_input_head += HASH_INPUT_BYTES;
    }
  }

  for (i = 0; i < N; i++) {
    for (j = 0; j < D; j++) {
      store_32(hash_input_head, d[i].poly[j]);

      hash_input_head += HASH_INPUT_BYTES;
    }
  }

  for (i = 0; i < N; i++) {
    for (j = 0; j < D; j++) {
      store_32(hash_input_head, e_0[0][i].poly[j]);

      hash_input_head += HASH_INPUT_BYTES;
    }
  }

  for (i = 0; i < N; i++) {
    for (j = 0; j < D; j++) {
      store_32(hash_input_head, e_0[1][i].poly[j]);

      hash_input_head += HASH_INPUT_BYTES;
    }
  }

#if M_SPENT == 2
  for (i = 0; i < N; i++) {
    for (j = 0; j < D; j++) {
      store_32(hash_input_head, e_0[2][i].poly[j]);

      hash_input_head += HASH_INPUT_BYTES;
    }
  }
#endif

  for (i = 0; i < D; i++) {
    store_32(hash_input_head, f_0[0].poly[i]);

    hash_input_head += HASH_INPUT_BYTES;
  }

#if M_SPENT == 2
  for (i = 0; i < D; i++) {
    store_32(hash_input_head, f_0[1].poly[i]);

    hash_input_head += HASH_INPUT_BYTES;
  }
#endif

  for (i = 0; i < N; i++) {
    for (j = 0; j < D; j++) {
      store_32(hash_input_head, g_out[0][i].poly[j]);

      hash_input_head += HASH_INPUT_BYTES;
    }
  }

  for (i = 0; i < N; i++) {
    for (j = 0; j < D; j++) {
      store_32(hash_input_head, g_out[1][i].poly[j]);

      hash_input_head += HASH_INPUT_BYTES;
    }
  }

  for (i = 0; i < D; i++) {
    store_32(hash_input_head, s[0].poly[i]);

    hash_input_head += HASH_INPUT_BYTES;
  }

#if M_SPENT == 2
  for (i = 0; i < D; i++) {
    store_32(hash_input_head, s[1].poly[i]);

    hash_input_head += HASH_INPUT_BYTES;
  }
#endif

  for (i = 0; i < N_SPENT; i++) {
    for (j = 0; j < N; j++) {
      for (k = 0; k < D; k++) {
        store_32(hash_input_head, a_in[i].pk[0][j].poly[k]);

        hash_input_head += HASH_INPUT_BYTES;
      }
    }

    for (j = 0; j < N; j++) {
      for (k = 0; k < D; k++) {
        store_32(hash_input_head, a_in[i].cn[0][j].poly[k]);

        hash_input_head += HASH_INPUT_BYTES;
      }
    }
  }

#if M_SPENT == 2
  for (i = 0; i < N_SPENT; i++) {
    for (j = 0; j < N; j++) {
      for (k = 0; k < D; k++) {
        store_32(hash_input_head, a_in[i].pk[1][j].poly[k]);

        hash_input_head += HASH_INPUT_BYTES;
      }
    }

    for (j = 0; j < N; j++) {
      for (k = 0; k < D; k++) {
        store_32(hash_input_head, a_in[i].cn[1][j].poly[k]);

        hash_input_head += HASH_INPUT_BYTES;
      }
    }
  }
#endif

  for (i = 0; i < N; i++) {
    for (j = 0; j < D; j++) {
      store_32(hash_input_head, pk_out[0][i].poly[j]);

      hash_input_head += HASH_INPUT_BYTES;
    }
  }

  for (i = 0; i < N; i++) {
    for (j = 0; j < D; j++) {
      store_32(hash_input_head, pk_out[1][i].poly[j]);

      hash_input_head += HASH_INPUT_BYTES;
    }
  }

  for (i = 0; i < N; i++) {
    for (j = 0; j < D; j++) {
      store_32(hash_input_head, cn_out[0][i].poly[j]);

      hash_input_head += HASH_INPUT_BYTES;
    }
  }

  for (i = 0; i < N; i++) {
    for (j = 0; j < D; j++) {
      store_32(hash_input_head, cn_out[1][i].poly[j]);

      hash_input_head += HASH_INPUT_BYTES;
    }
  }

  /* we use SHAKE-256 as the hash function; the extra text (if set) is
   * appended to the hashed input, binding the signature to that text */
  if (extra_len > 0) {
    size_t base_len = HASH_INPUT_LEN * HASH_INPUT_BYTES;
    unsigned char *combined = malloc(base_len + extra_len);
    memcpy(combined, hash_input, base_len);
    memcpy(combined + base_len, extra_buf, extra_len);
    SHAKE256(hash_output, HASH_OUTPUT_LEN, combined, base_len + extra_len);
    free(combined);
  } else {
    SHAKE256(hash_output, HASH_OUTPUT_LEN, hash_input,
             HASH_INPUT_LEN * HASH_INPUT_BYTES);
  }

  parse_hash(x, hash_output);
}
