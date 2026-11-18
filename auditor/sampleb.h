/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * sample B                       *
 * ****************************** */

#ifndef _SAMPLEB_H
#define _SAMPLEB_H

#include "poly_q.h"
#include "poly_qbig.h"
#include <stdint.h>

void sample_b_md(POLY_Q *out);
void sample_bbig_md(POLY_Q *out);
void sample_bbig_k_prime_md(POLY_Q *out);
void sample_b_mhatd(POLY_Q *out);
void sample_bbig_hat_mhatd(POLY_Q *out);

void sample_be_mhatd(POLY_QBIG *out);
void sample_be_v1d(POLY_QBIG *out);
void sample_be_v2d(POLY_QBIG *out);

void sample_ba(POLY_Q *out);
void sample_br(POLY_Q *out);
void sample_bap(POLY_Q *out);

uint64_t sample_beta();

void sample_s(POLY_QBIG *out);

#endif
