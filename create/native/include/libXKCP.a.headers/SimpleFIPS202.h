/* Minimal stand-in for XKCP's SimpleFIPS202.h: only SHAKE256 is used
 * by the MatRiCT-Au code (hash.c). Implemented in shake256.c (FIPS 202). */
#ifndef _SimpleFIPS202_h_
#define _SimpleFIPS202_h_

#include <stddef.h>

int SHAKE256(unsigned char *output, size_t outputByteLen,
             const unsigned char *input, size_t inputByteLen);

#endif
