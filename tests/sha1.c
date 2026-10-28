/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "sha1.h"
#include <stdio.h>
#include <string.h>

#include "c_implementation.h"

static int check(SHA1_CTX* context, char const* expected) {
    unsigned char digest[20];
    char actual[42];
    SHA1Final(digest, context);
    memset(actual, '?', sizeof(actual));
    SHA1ToHexString(digest, actual);
    if (memcmp(actual, expected, 40) || actual[40] || actual[41] != '?') {
        fprintf(stderr, "SHA-1 mismatch: expected %s, got %.40s\n", expected, actual);
        return 1;
    }
    return 0;
}

int main(void) {
    static struct { char const* text; char const* digest; } const vectors[] = {
        {"", "da39a3ee5e6b4b0d3255bfef95601890afd80709"},
        {"abc", "a9993e364706816aba3e25717850c26c9cd0d89d"},
        {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
         "84983e441c3bd26ebaae4aa1f95129e5e54670f1"}
    };
    static struct { unsigned length; char const* digest; } const boundaries[] = {
        {55, "c4622048cfef59b72875839ee7ae1cbcf55e7658"},
        {56, "ddc12942656468475970fa4fa49161f52ed138e4"},
        {63, "7f8c3fa49f1297bd8b9feb964b6b419987f9f0d1"},
        {64, "a334b47180c61fd522f99905ec02c36f9e848211"},
        {65, "dd27d9eb923d39687e10872c3e8133ba2f0a68a1"},
        {127, "b2b4bfd7b2112a167b77a600cca227593523c406"},
        {128, "3b1953091899492377f686c266b81d84b5d40f70"},
        {129, "4fd6558b2a93925fb7129447e1d1fac8cff56287"},
    };
    unsigned char storage[131], repeated[1000];
    SHA1_CTX context;
    int failed = 0;
    for (unsigned i = 0; i < sizeof(vectors) / sizeof(*vectors); ++i) {
        size_t length = strlen(vectors[i].text);
        SHA1Init(&context);
        SHA1Update(&context, (unsigned char const*)vectors[i].text, (Card32)length);
        failed |= check(&context, vectors[i].digest);
    }
    for (unsigned i = 0; i < sizeof(storage); ++i) {
        storage[i] = (unsigned char)((i * 37 + 11) & 255);
    }
    /* An unaligned, nonterminated input exercises direct block loading. */
    for (unsigned i = 0; i < sizeof(boundaries) / sizeof(*boundaries); ++i) {
        unsigned length = boundaries[i].length;
        unsigned char unaligned[131];
        memcpy(unaligned + 1, storage, length);
        SHA1Init(&context);
        SHA1Update(&context, unaligned + 1, length);
        failed |= check(&context, boundaries[i].digest);
        SHA1Init(&context);
        for (unsigned pos = 0; pos < length;) {
            unsigned chunk = length - pos < 7 ? length - pos : 7;
            SHA1Update(&context, storage + pos, chunk);
            pos += chunk;
        }
        failed |= check(&context, boundaries[i].digest);
    }
    memset(repeated, 'a', sizeof(repeated));
    SHA1Init(&context);
    for (unsigned i = 0; i < 1000; ++i) {
        SHA1Update(&context, repeated, sizeof(repeated));
    }
    failed |= check(&context, "34aa973cd4c4daa4f61eeb2bdbad27316534016f");
    return failed;
}
