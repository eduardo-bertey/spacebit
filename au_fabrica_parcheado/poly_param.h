/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * poly ring constants            *
 * ****************************** */

#ifndef _POLY_PARAM_H
#define _POLY_PARAM_H

#define D 64LL /* ring dimension */

/* modulus */
#define Q 2147221513LL
#define QBIG1 134215681LL
#define QBIG2 536870657LL
#define QBIG 72056460838172417LL

#define T_DEC 13
#define T_BAR 5542804679859416LL
#define T_BAR_QBIG1 113567114LL
#define T_BAR_QBIG2 82595485LL

/* QBIG1^{-1} % QBIG2 (containing the Montgomery factor) */
#define QBIG1_INVMOD_QBIG2 368290698LL

#endif
