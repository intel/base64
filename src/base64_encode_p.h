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

#include "base64.h"

#if defined(__APPLE__)
/* nothing */
#elif defined(__GNUC__) && !defined(_WIN32)
#  define __private_extern__ __attribute__((visibility("hidden")))
#else
#  define __private_extern__
#endif

#define CPUID_SSSE3           1
#define CPUID_AVX2            2
#ifdef __SSSE3__
#  define cpu_supports_SSSE3    1
#elif defined(HAVE_BUILTIN_CPU_SUPPORTS)
#  define cpu_supports_SSSE3    __builtin_cpu_supports("ssse3")
#else
#  define cpu_supports_SSSE3    _base64_cpuid() & CPUID_SSSE3
#endif

#ifdef __AVX2__
#  define cpu_supports_AVX2     1
#elif defined(HAVE_BUILTIN_CPU_SUPPORTS)
#  define cpu_supports_AVX2     __builtin_cpu_supports("avx2")
#else
#  define cpu_supports_AVX2     _base64_cpuid() & CPUID_AVX2
#endif

__private_extern__ extern const char _base64_alphabet[64];
__private_extern__ extern const char _base64url_alphabet[64];
__private_extern__ extern const char _base64_alphabet_precombined[64*64][2];
__private_extern__ extern const char _base64url_alphabet_precombined[64*64][2];

__private_extern__ extern int _base64_cpuid();

__private_extern__ char *
_base64_encode_tail(char *out, size_t offset, const unsigned char * in, size_t len, int options);

typedef char *(encode_function)(char *, const unsigned char *, size_t, int);
__private_extern__ extern encode_function _base64_encode_plain;
__private_extern__ extern encode_function _base64_encode_ssse3;
__private_extern__ extern encode_function _base64_encode_avx2;

#define ALIGN_PROLOGUE(out, in, len, N)     \
    TBD
