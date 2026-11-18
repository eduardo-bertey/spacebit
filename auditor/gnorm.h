/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * generate g and compare ||g||   *
 * ****************************** */

#ifndef _GNORM_H
#define _GNORM_H

#include "poly_q.h"
#include <stdint.h>

uint64_t check_gnorm(POLY_Q *g_norm, const POLY_Q *f00, const POLY_Q *f0,
                     const POLY_Q *fr, const POLY_Q *x);

#endif
