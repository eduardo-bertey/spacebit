/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * KeyGen                         *
 * ****************************** */

#ifndef _KEYGEN_H
#define _KEYGEN_H

#include "param.h"
#include "poly_q.h"

void keygen(POLY_Q *pk, POLY_Q *sk, const POLY_Q g[][N]);

#endif
