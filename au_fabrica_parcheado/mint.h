/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * Mint                           *
 * ****************************** */

#ifndef _MINT_H
#define _MINT_H

#include "param.h"
#include "poly_q.h"
#include <stdint.h>

void mint(POLY_Q *cn, POLY_Q *ck, uint64_t amt, const POLY_Q g[][N]);

#endif
