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
#include <immintrin.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static inline void do_encode_6bytes(const char (*alphabet)[2], char *out, __m128i chunk)
{
    uint32_t v0, v1, v2, v3;
    v0 = _mm_extract_epi32(chunk, 0);
    v1 = _mm_extract_epi32(chunk, 1);
    v2 = _mm_extract_epi32(chunk, 2);
    v3 = _mm_extract_epi32(chunk, 3);
    memcpy(out + 0, alphabet[v0], 2);
    memcpy(out + 2, alphabet[v1], 2);
    memcpy(out + 4, alphabet[v2], 2);
    memcpy(out + 6, alphabet[v3], 2);
}

static inline void do_encode_12bytes(const char (*alphabet)[2], char *out, __m256i chunk)
{
    const __m256i shufflemask = _mm256_set_epi8(
        -1, 9, 10, 11,
        -1, 9, 10, 11,
        -1, 6, 7, 8,
        -1, 6, 7, 8,
        -1, 3, 4, 5,
        -1, 3, 4, 5,
        -1, 0, 1, 2,
        -1, 0, 1, 2
    );
    const __m256i shifts = _mm256_set_epi32(0, 12, 0, 12, 0, 12, 0, 12);
    const __m256i masks = _mm256_set1_epi32(4095);

    // convert from big endian and rearrange the bytes
    chunk = _mm256_shuffle_epi8(chunk, shufflemask);
    chunk = _mm256_srlv_epi32(chunk, shifts);
    chunk = _mm256_and_si256(chunk, masks);

    // write the two halves to memory
    do_encode_6bytes(alphabet, out + 0, _mm256_extracti128_si256(chunk, 0));
    do_encode_6bytes(alphabet, out + 8, _mm256_extracti128_si256(chunk, 1));
}

char *_base64_encode_avx2(char *out, const unsigned char *in, size_t n, int options)
{
    size_t i;
    size_t o = 0;

    const char (*alphabet)[2] = _base64_alphabet_precombined;
    if (options & Base64UseUrlAlphabet)
        alphabet = _base64url_alphabet_precombined;

    for (i = 0; n - i >= 48; i += 48) {
        // read 48 bytes and duplicate each 16-byte chunk in the high part of the register
        __m256i chunk1 = _mm256_broadcastsi128_si256(* (const __m128i *)&in[i+0]);
        __m256i chunk2 = _mm256_broadcastsi128_si256(* (const __m128i *)&in[i+16]);
        __m256i chunk3 = _mm256_broadcastsi128_si256(* (const __m128i *)&in[i+32]);

        // first chunk of 12 bytes
        do_encode_12bytes(alphabet, out + o, chunk1);
        o += 16;

        // second chunk: 4 bytes left in chunk1
        do_encode_12bytes(alphabet, out + o, _mm256_alignr_epi8(chunk2, chunk1, 12));
        o += 16;

        // third chunk: 8 bytes left in chunk2
        do_encode_12bytes(alphabet, out + o, _mm256_alignr_epi8(chunk3, chunk2, 8));
        o += 16;

        // fourth chunk: 12 final bytes in chunk3
        do_encode_12bytes(alphabet, out + o, _mm256_srli_si256(chunk3, 4));
        o += 16;

        if (options & Base64InsertLineBreaks)
            out[o++] = '\n';
    }

    return _base64_encode_tail(out, o, in, n, options);
}
