/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * parse or sample C^d_{w,p}      *
 * ****************************** */

#ifndef _SAMPLEX_H
#define _SAMPLEX_H

#include "param.h"
#include "poly_q.h"

void parse_hash(POLY_Q *x, const unsigned char *hash_output);
void sample_x(POLY_Q *x);

#endif
