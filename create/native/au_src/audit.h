/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * Audit                          *
 * ****************************** */

#ifndef _AUDIT_H
#define _AUDIT_H

#include "param.h"
#include "poly_qbig.h"
#include "spend.h"
#include <stdint.h>

void audit(uint64_t *l, uint64_t *amt_out, const SPEND_OUT *in,
           const POLY_QBIG g_hat[][N_HAT], const POLY_QBIG *td);

#endif
