/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * SamMat                         *
 * ****************************** */

#ifndef _SAMMAT_H
#define _SAMMAT_H

#include "param.h"
#include "poly_q.h"
#include "poly_qbig.h"

void sample_mat_g0(POLY_Q out[][N]);
void sample_mat_gr(POLY_Q out[][N]);
void sample_mat_gh(POLY_Q *out);
void sample_mat_gbig(POLY_QBIG out[][N_HAT]);

#endif
