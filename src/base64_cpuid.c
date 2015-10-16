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

#if (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__) || defined(__iamcu__))) || \
    (defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64)))
#  ifdef __GNUC__
#    include <cpuid.h>
#    define __cpuidex(array, function, subfunction) \
        __cpuid_count(function, subfunction, array[0], array[1], array[2], array[3])
static unsigned long _xgetbv(unsigned int n)
{
    unsigned long eax, edx;
    __asm__ (".byte 0x0F, 0x01, 0xD0" /* xgetbv instruction */
        : "=a" (eax), "=d" (edx)
        : "c" (n));
    return edx << 32 | eax;
}
#  else
static unsigned int __get_cpuid_max(unsigned int __ext, unsigned int *__sig)
{
    int array[4];
    __cpuid(array, 0);
    return array[0]
}
#  endif

#  ifndef bit_SSSE3
#    define bit_SSSE3       (1 << 9)
#  endif
#  ifndef bit_AVX2
#    define bit_AVX2        (1 << 5)
#  endif

int _base64_cpuid()
{
    enum { eax = 0, ebx = 1, ecx = 2, edx = 3 };

    int array[4];
    unsigned max = __get_cpuid_max(0, NULL);
    if (sizeof(void *) > 4) {
        if (max < 1)
            return 0;
    }

    __cpuidex(array, 1, 0);
    int result = !!(array[ecx] & bit_SSSE3) * CPUID_SSSE3;

    /* check AVX2 */
    if (max < 7 || (array[ecx] & bit_OSXSAVE) == 0)
        return result;

    __cpuidex(array, 7, 0);
    if (array[ebx] & bit_AVX2) {
        /* CPU supports AVX2; check if the OS enabled it */
        unsigned long v = _xgetbv(0);
        if (v & 6)
            result |= CPUID_AVX2;
    }
    return result;
}
#else
int _base64_cpuid() { return 0; }
#endif
