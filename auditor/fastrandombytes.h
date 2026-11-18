/* ****************************** *
 * Implemented by Raymond K. ZHAO *
 *                                *
 * AES-256 CTR PRG                *
 * ****************************** */

#ifndef FASTRANDOMBYTES_H
#define FASTRANDOMBYTES_H

void fastrandombytes_setseed_pub(const unsigned char *randomness);
void fastrandombytes_setseed_prv(const unsigned char *randomness);
void fastrandombytes_pub(unsigned char *r, unsigned long long rlen);
void fastrandombytes_prv(unsigned char *r, unsigned long long rlen);
void fastrandombytes_setiv_g();
void fastrandombytes_setiv_h();
void fastrandombytes_setiv_gbig();

#endif
