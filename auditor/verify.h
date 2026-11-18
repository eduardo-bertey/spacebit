/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * Verify                         *
 * ****************************** */

#ifndef _VERIFY_H
#define _VERIFY_H

#include "param.h"
#include "poly_q.h"
#include "poly_qbig.h"
#include "spend.h"
#include <stdint.h>

uint64_t verify(const ACT *a_in, const POLY_Q pk_out[][N],
                const POLY_Q cn_out[][N], const SPEND_OUT *in, const POLY_Q *s,
                const POLY_Q g[][N], const POLY_Q *h, POLY_QBIG g_hat[][N_HAT],
                const POLY_QBIG *t0, const POLY_QBIG *t1, const POLY_QBIG *t2);

#endif
