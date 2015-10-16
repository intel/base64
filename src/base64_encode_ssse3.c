/****************************************************************************
**
** Copyright (C) 2015 Intel Corporation
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
**
****************************************************************************/

#include "base64_encode_p.h"
#include "base64_byteorder_p.h"
#include <tmmintrin.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifdef __SSE4_1__
#  include <smmintrin.h>
#endif

static inline void do_encode_3bytes(const char (*alphabet)[2], char *out, uint_least32_t val)
{
    // write 4 chars x 6 bits = 24 bits
    memcpy(out, alphabet[(val >> 12) & 4095], 2);
    memcpy(out + 2, alphabet[val & 4095], 2);
}

static inline void do_encode_12bytes(const char (*alphabet)[2], char *out, __m128i chunk)
{
    // this shuffle mask converts the input data from 4x24 bits (12 bytes) to
    // 4x32 bits (16 bytes), while at the same time converting from big to
    // little endian
    const __m128i shufflemask = _mm_set_epi8(
        -1, 9, 10, 11,
        -1, 6, 7, 8,
        -1, 3, 4, 5,
        -1, 0, 1, 2
    );

    chunk = _mm_shuffle_epi8(chunk, shufflemask);

    // extract the 4x32-bit onto regular registers
    uint32_t v0, v1, v2, v3;
    {
#ifdef __SSE4_1__
        v0 = _mm_extract_epi32(chunk, 0);
        v1 = _mm_extract_epi32(chunk, 1);
        v2 = _mm_extract_epi32(chunk, 2);
        v3 = _mm_extract_epi32(chunk, 3);
#elif defined(__x86_64__) || defined(_M_X64)
        uint64_t half = _mm_cvtsi128_si64(chunk);
        v0 = half;
        v1 = half >> 32;
        chunk = _mm_srli_si128(chunk, 8);
        half = _mm_cvtsi128_si64(chunk);
        v2 = half;
        v3 = half >> 32;
#else
        v0 = _mm_cvtsi128_si32(chunk);
        v1 = _mm_cvtsi128_si32(_mm_shuffle_epi32(chunk, 1));
        v2 = _mm_cvtsi128_si32(_mm_shuffle_epi32(chunk, 2));
        v3 = _mm_cvtsi128_si32(_mm_shuffle_epi32(chunk, 3));
#endif
    }


    do_encode_3bytes(alphabet, out, v0);
    do_encode_3bytes(alphabet, out + 4, v1);
    do_encode_3bytes(alphabet, out + 8, v2);
    do_encode_3bytes(alphabet, out + 12, v3);
}

char *_base64_encode_ssse3(char *out, const unsigned char *in, size_t n, int options)
{
    size_t i;
    size_t o = 0;

    const char (*alphabet)[2] = _base64_alphabet_precombined;
    if (options & Base64UseUrlAlphabet)
        alphabet = _base64url_alphabet_precombined;

    for (i = 0; n - i >= 48; i += 48) {
        // read 48 bytes
        __m128i chunk1 = _mm_loadu_si128((const __m128i*)&in[i+0]);
        __m128i chunk2 = _mm_loadu_si128((const __m128i*)&in[i+16]);
        __m128i chunk3 = _mm_loadu_si128((const __m128i*)&in[i+32]);

        // first chunk of 12 bytes
        do_encode_12bytes(alphabet, out + o, chunk1);
        o += 16;

        // second chunk: 4 bytes left in chunk1
        do_encode_12bytes(alphabet, out + o, _mm_alignr_epi8(chunk2, chunk1, 12));
        o += 16;

        // third chunk: 8 bytes left in chunk2
        do_encode_12bytes(alphabet, out + o, _mm_alignr_epi8(chunk3, chunk2, 8));
        o += 16;

        // fourth chunk: 12 final bytes in chunk3
        do_encode_12bytes(alphabet, out + o, _mm_srli_si128(chunk3, 4));
        o += 16;

        if (options & Base64InsertLineBreaks)
            out[o++] = '\n';
    }

    return _base64_encode_tail(out, o, in, n, options);
}
