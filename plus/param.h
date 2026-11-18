#ifndef _PARAM_H
#define _PARAM_H

#include "poly_param.h"

#define CRYPTO_BYTES 32LL

#define M 2LL
#define S 2LL
#define N_SPENT 50LL
#define W 44LL
#define WA 39LL
#define N 4LL
#define K 4LL
#define N_BAR 4LL
#define N_HAT 5LL
#define K_HAT 5LL

#define B_K 7525LL
#define BA_10_K 111LL
#define BETA 40LL
#define BRS_13_K 1219127LL
/* BG_0 escala automaticamente con el anillo (formula de main/):
 * 37162531 es el tope de los autores para 50 llaves; por llave = 743250.
 * Con N_SPENT=50 da el valor original; con 1000 da 20x. */
#define BG_0 (37162531LL * N_SPENT / 50LL)
#define BG_1 1137628LL

#define B_6 38346LL
#define BA_10_6 565LL
#define BRS_13_6 6212586LL

#endif
