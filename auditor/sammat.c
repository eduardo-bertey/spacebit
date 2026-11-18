/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * SamMat                         *
 * ****************************** */

#include <stdint.h>

#include "fastrandombytes.h"
#include "littleendian.h"
#include "param.h"
#include "poly_q.h"
#include "poly_qbig.h"
#include "poly_red.h"
#include "sammat.h"

/* sample G matrix */
static inline void sample_mat_g(POLY_Q out[][N], const uint64_t m,
                                const uint64_t rej_len) {
  static unsigned char r[REJ_UNIF_Q_GR_LEN * REJ_UNIF_Q_BYTES];
  uint64_t i, j, k, x;

  unsigned char *r_head = r;

  fastrandombytes_setiv_g();

  fastrandombytes_pub(r, REJ_UNIF_Q_BYTES * rej_len);

  for (i = 0; i < m; i++) {
    for (j = 0; j < N; j++) {
      for (k = 0; k < D; k++) {
        do {
          x = load_32(r_head);
          r_head += REJ_UNIF_Q_BYTES;
        } while (x >= REJ_UNIF_Q_BOUND);

        out[i][j].poly[k] = con_sub(x, Q);
      }
    }
  }
}

void sample_mat_g0(POLY_Q out[][N]) { sample_mat_g(out, M, REJ_UNIF_Q_G0_LEN); }

void sample_mat_gr(POLY_Q out[][N]) {
  sample_mat_g(out, M + R, REJ_UNIF_Q_GR_LEN);
}

/* sample H matrix */
void sample_mat_gh(POLY_Q *out) {
  static unsigned char r[REJ_UNIF_Q_H_LEN * REJ_UNIF_Q_BYTES];
  uint64_t i, j, x;

  unsigned char *r_head = r;

  fastrandombytes_setiv_h();

  fastrandombytes_pub(r, REJ_UNIF_Q_BYTES * REJ_UNIF_Q_H_LEN);

  for (i = 0; i < M; i++) {
    for (j = 0; j < D; j++) {
      do {
        x = load_32(r_head);
        r_head += REJ_UNIF_Q_BYTES;
      } while (x >= REJ_UNIF_Q_BOUND);

      out[i].poly[j] = con_sub(x, Q);
    }
  }
}

/* sample G_hat matrix */
void sample_mat_gbig(POLY_QBIG out[][N_HAT]) {
  static unsigned char r[REJ_UNIF_QBIG_LEN * REJ_UNIF_QBIG_BYTES];
  uint64_t i, j, k, x;

  unsigned char *r_head = r;

  fastrandombytes_setiv_gbig();

  fastrandombytes_pub(r, REJ_UNIF_QBIG_LEN * REJ_UNIF_QBIG_BYTES);

  for (i = 0; i < M_HAT + V_HAT; i++) {
    for (j = 0; j < N_HAT; j++) {
      for (k = 0; k < D; k++) {
        do {
          x = load_56(r_head);
          r_head += REJ_UNIF_QBIG_BYTES;
        } while (x >= REJ_UNIF_QBIG_BOUND);

        out[i][j].poly1[k] = x % QBIG1;
        out[i][j].poly2[k] = x % QBIG2;
      }
    }
  }
}
