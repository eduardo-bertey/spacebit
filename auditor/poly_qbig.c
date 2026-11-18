/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * ntt on R_{\hat{q}}             *
 * ****************************** */

#include "poly_qbig.h"
#include "poly_param.h"
#include "poly_red.h"
#include <stdint.h>

#define INV_N_QBIG1 132118561

#define INV_N_QBIG2 528482053

static const uint64_t psi_qbig1[D] = {
    0,         109131128, 126251080, 70854583,  72905940,  54201686,  75635241,
    96768772,  14267631,  21044366,  40217166,  105274606, 23053474,  54045255,
    54582905,  29783661,  56200316,  28122743,  109840550, 69846535,  99178610,
    131400895, 51150663,  70895333,  12647626,  10540326,  128095456, 88933485,
    121439855, 126691202, 26609138,  3239786,   78525796,  32627689,  24942905,
    64138855,  54260605,  95679300,  124713926, 86707200,  13783529,  120296806,
    105393827, 26522247,  75908394,  27015574,  18679951,  98196911,  48351885,
    124710348, 64249090,  118452210, 73561021,  1779291,   118465480, 65142177,
    88548221,  36992911,  22886135,  42808757,  108877769, 80197304,  10075758,
    42444576};
static const uint64_t psi_inv_qbig1[D] = {
    0,         49747106,  63361098,  7964601,   37446909,  58580440,  80013995,
    61309741,  104432020, 79632776,  80170426,  111162207, 28941075,  93998515,
    113171315, 119948050, 130975895, 107606543, 7524479,   12775826,  45282196,
    6120225,   123675355, 121568055, 63320348,  83065018,  2814786,   35037071,
    64369146,  24375131,  106092938, 78015365,  91771105,  124139923, 54018377,
    25337912,  91406924,  111329546, 97222770,  45667460,  69073504,  15750201,
    132436390, 60654660,  15763471,  69966591,  9505333,   85863796,  36018770,
    115535730, 107200107, 58307287,  107693434, 28821854,  13918875,  120432152,
    47508481,  9501755,   38536381,  79955076,  70076826,  109272776, 101587992,
    55689885};

static const uint64_t psi_qbig2[D] = {
    0,         2113217,   338911751, 65455629,  207295251, 92809301,  148704782,
    202227742, 190104151, 322779325, 451118253, 146858624, 200062444, 375860655,
    3410503,   52309456,  256314258, 451159164, 431986373, 415677560, 455791138,
    535928373, 46290487,  340518686, 355259825, 446997448, 28594849,  271090957,
    6159945,   503264848, 92024304,  257805221, 169066528, 303822593, 307274790,
    402123737, 481886618, 477622432, 162681113, 250605223, 527106320, 218853597,
    366417638, 345942810, 259981944, 339887851, 277184203, 85324791,  361172290,
    466246085, 197636357, 293902929, 56755244,  42189368,  226349206, 23487336,
    205672306, 280300800, 247195115, 216560575, 103964122, 193113857, 468077956,
    352960060};
static const uint64_t psi_inv_qbig2[D] = {
    0,         25297400,  471415028, 197958906, 334642915, 388165875, 444061356,
    329575406, 484561201, 533460154, 161010002, 336808213, 390012033, 85752404,
    214091332, 346766506, 279065436, 444846353, 33605809,  530710712, 265779700,
    508275808, 89873209,  181610832, 196351971, 490580170, 942284,    81079519,
    121193097, 104884284, 85711493,  280556399, 183910597, 68792701,  343756800,
    432906535, 320310082, 289675542, 256569857, 331198351, 513383321, 310521451,
    494681289, 480115413, 242967728, 339234300, 70624572,  175698367, 451545866,
    259686454, 196982806, 276888713, 190927847, 170453019, 318017060, 9764337,
    286265434, 374189544, 59248225,  54984039,  134746920, 229595867, 233048064,
    367804129};

/* full ntt on sub qbig1 or qbig2, where \hat{q}=qbig1*qbig2 */
static inline void ntt(uint64_t *a, const uint64_t q, const uint64_t *psi,
                       const uint64_t montgomery_qbig_factor,
                       const uint64_t montgomery_qbig_shift,
                       const uint64_t montgomery_qbig_convert_factor) {
  uint64_t t = D >> 1, m, i, j, j_first, j_last;
  uint64_t u, v;

  /* implicitly convert to Montgomery form and perform the butterfly operations
   */
  /* butterfly: (u,v)-->(u+v*omega,u-v*omega) */
  /* we can also omit the reduction of u+/-v*omega on every odd level */
  for (j = 0; j < (D >> 1); j++) {
    u = montgomery_qbig(a[j], montgomery_qbig_convert_factor, q,
                        montgomery_qbig_factor, montgomery_qbig_shift);
    v = montgomery_qbig(a[j + (D >> 1)], psi[1], q, montgomery_qbig_factor,
                        montgomery_qbig_shift);

    a[j] = con_sub(u + v, q);
    a[j + (D >> 1)] = con_add(u - v, q);
  }

  for (m = 2; m < (D >> 1); m <<= 1) {
    t >>= 1;
    for (i = 0; i < m; i++) {
      j_first = 2 * i * t;
      j_last = j_first + t;
      for (j = j_first; j < j_last; j++) {
        u = a[j];
        v = montgomery_qbig(a[j + t], psi[m + i], q, montgomery_qbig_factor,
                            montgomery_qbig_shift);

        a[j] = con_sub(u + v, q);
        a[j + t] = con_add(u - v, q);
      }
    }
  }

  for (i = 0; i < (D >> 1); i++) {
    j = i << 1;

    u = a[j];
    v = montgomery_qbig(a[j + 1], psi[(D >> 1) + i], q, montgomery_qbig_factor,
                        montgomery_qbig_shift);

    a[j] = con_sub(u + v, q);
    a[j + 1] = con_add(u - v, q);
  }
}

static inline void intt(uint64_t *a, const uint64_t q, const uint64_t *psi_inv,
                        const uint64_t inv_n,
                        const uint64_t montgomery_qbig_factor,
                        const uint64_t montgomery_qbig_shift) {
  uint64_t t = 2, m, i, j, j_first, j_last, h;
  uint64_t u, v;

  /* butterfly: (u,v)-->(u+v, (u-v)*omega) */
  for (i = 0; i < (D >> 1); i++) {
    j = i << 1;

    u = a[j];
    v = a[j + 1];

    a[j] = con_sub(u + v, q);
    a[j + 1] = montgomery_qbig(q + u - v, psi_inv[(D >> 1) + i], q,
                               montgomery_qbig_factor, montgomery_qbig_shift);
  }

  for (m = (D >> 1); m > 2; m >>= 1) {
    j_first = 0;
    h = m >> 1;
    for (i = 0; i < h; i++) {
      j_last = j_first + t;
      for (j = j_first; j < j_last; j++) {
        u = a[j];
        v = a[j + t];

        a[j] = con_sub(u + v, q);
        a[j + t] =
            montgomery_qbig(q + u - v, psi_inv[h + i], q,
                            montgomery_qbig_factor, montgomery_qbig_shift);
      }
      j_first += (t << 1);
    }
    t <<= 1;
  }

  /* implicitly get rid of the Montgomery factor */
  for (j = 0; j < (D >> 1); j++) {
    u = a[j];
    v = a[j + (D >> 1)];

    a[j] = montgomery_qbig(u + v, inv_n, q, montgomery_qbig_factor,
                           montgomery_qbig_shift);
    a[j + (D >> 1)] =
        montgomery_qbig(q + u - v, psi_inv[1], q, montgomery_qbig_factor,
                        montgomery_qbig_shift);
  }
}

/* perform ntt on both sub qbig1 and qbig2 */
void ntt_qbig(POLY_QBIG *a) {
  ntt(a->poly1, QBIG1, psi_qbig1, MONTGOMERY_FACTOR_QBIG1,
      MONTGOMERY_SHIFT_QBIG1, MONTGOMERY_CONVERT_FACTOR_QBIG1);
  ntt(a->poly2, QBIG2, psi_qbig2, MONTGOMERY_FACTOR_QBIG2,
      MONTGOMERY_SHIFT_QBIG2, MONTGOMERY_CONVERT_FACTOR_QBIG2);
}

void intt_qbig(POLY_QBIG *a) {
  intt(a->poly1, QBIG1, psi_inv_qbig1, INV_N_QBIG1, MONTGOMERY_FACTOR_QBIG1,
       MONTGOMERY_SHIFT_QBIG1);
  intt(a->poly2, QBIG2, psi_inv_qbig2, INV_N_QBIG2, MONTGOMERY_FACTOR_QBIG2,
       MONTGOMERY_SHIFT_QBIG2);
}

/* pointwise multiplication on both sub qbig1 and qbig2 */
void mult_rqbig(POLY_QBIG *out, const POLY_QBIG *a, const POLY_QBIG *b) {
  uint64_t i;

  for (i = 0; i < D; i++) {
    out->poly1[i] =
        montgomery_qbig(a->poly1[i], b->poly1[i], QBIG1,
                        MONTGOMERY_FACTOR_QBIG1, MONTGOMERY_SHIFT_QBIG1);
    out->poly2[i] =
        montgomery_qbig(a->poly2[i], b->poly2[i], QBIG2,
                        MONTGOMERY_FACTOR_QBIG2, MONTGOMERY_SHIFT_QBIG2);
  }
}
