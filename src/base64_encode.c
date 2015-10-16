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
#include "base64_encode_p.h"

#ifdef HAVE_ATOMICS
#  include <stdatomic.h>
#endif

#ifdef HAVE_IFUNC
/* Let the linker figure it out */
encode_function base64_encode __attribute__((ifunc("_base64_resolve_encode")));
#else
static encode_function *_base64_resolve_encode();
char *base64_encode(char *out, const unsigned char *in, size_t len, int options)
{
    typedef encode_function *encode_function_ptr;
#ifdef HAVE_ATOMICS
    static _Atomic encode_function_ptr f = ATOMIC_VAR_INIT((encode_function_ptr)0);
#else
    static encode_function_ptr f = (encode_function_ptr)0;
#endif
    if (!f)
        f = _base64_resolve_encode();
    return f(out, in, len, options);
}
#endif


__private_extern__ encode_function *_base64_resolve_encode()
{
#ifdef HAVE_AVX2
    if (cpu_supports_AVX2)
        return &_base64_encode_avx2;
#endif
#ifdef HAVE_SSSE3
    if (cpu_supports_SSSE3)
        return &_base64_encode_ssse3;
#endif
    return _base64_encode_plain;
}
