/*
SHA-1 in C
By Steve Reid <steve@edmweb.com>
100% Public Domain
*/

#ifndef SHA1_H
#define SHA1_H

/*
   SHA-1 in C
   By Steve Reid <steve@edmweb.com>
   100% Public Domain
 */

#include "sysdefs.h"

typedef struct {
    Card32        state[5];
    Card32        count[2];
    unsigned char buffer[64];
} SHA1_CTX;

void SHA1Transform(Card32 state[5], unsigned char const buffer[64]);

void SHA1Init(SHA1_CTX* context);

void SHA1Update(SHA1_CTX* context, unsigned char const* data, Card32 len);

void SHA1Final(unsigned char digest[20], SHA1_CTX* context);

void SHA1(char* hash_out, char const* str, int len);

void SHA1ToHexString(unsigned char digest[20], char* hexstring);

#endif /* SHA1_H */
