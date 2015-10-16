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

#ifndef LIBBASE64_BASE64_H
#define LIBBASE64_BASE64_H

#include <stddef.h>

#ifndef LIBBASE64_API
#  ifdef base64_EXPORTS
#    ifdef _WIN32
#      define LIBBASE64_API     __declspec(dllexport)
#    elif defined(__ELF__)
#      define LIBBASE64_API     __attribute__((visibility("protected")))
#    else
#      define LIBBASE64_API     __attribute__((visibility("default")))
#    endif
#  else
#    ifdef _WIN32
#      define LIBBASE64_API     __declspec(dllimport)
#    else
#      define LIBBASE64_API     __attribute__((visibility("default")))
#    endif
#  endif
#endif

#ifdef __cplusplus
#  define LIBBASE64_INLINE_API inline
extern "C" {
#else
#  define LIBBASE64_INLINE_API static inline
#endif

typedef enum Base64Options {
    /* used in both encoding and decoding: */
    Base64UseOriginalAlphabet       = 0,
    Base64UseUrlAlphabet            = 1,
    Base64EmitPadding               = 0,
    Base64OmitPadding               = 2,

    /* used in encoding */
    Base64InsertLineBreaks          = 0x10,

    /* used in decoding: */
    Base64SkipLineBreaks            = 0x10,
    Base64SkipBlanks                = 0x20 | Base64SkipLineBreaks,
    Base64SkipUnknown               = 0x40,

    /* variant selection */
    Base64Variant                   = Base64UseOriginalAlphabet | Base64EmitPadding,
    Base64UrlVariant                = Base64UseUrlAlphabet | Base64OmitPadding
} Base64Options;

/*!
 * Returns the minimum size of the buffer that can hold the encoded version of
 * an input data of size \a len, plus the terminating NUL.
 */
#ifdef __GNUC__
__attribute__((pure))
#endif
LIBBASE64_INLINE_API size_t
base64_encode_buffer_size(size_t len, int options)
{
    len = (len + 5) / 3 * 4;
    if (options & Base64InsertLineBreaks) {
        /* libbase64 inserts one line break for every 64 output characters */
        len += (len + 63) / 64;
    }
    return len;
}

/*!
 * Encodes \a len bytes from the \a in buffer to the \a out buffer, using the
 * encoding options specified in \a options. The \a out buffer must be big
 * enough to accommodate the entire encoded data, plus a terminating NUL (see
 * base64_encode_buffer_size()).
 *
 * This function returns \a out.
 *
 * The \a in and \a out buffers may overlap, provided that \a out starts no
 * earlier than \c{base64_encode_buffer_size(len, options) - len} bytes from \a
 * in.
 */
LIBBASE64_API char *
base64_encode(char *out, const unsigned char *in, size_t len, int options);

/*!
 * Allocates a buffer and encodes \a len bytes from the \a in buffer to it,
 * using \a options. This function returns the allocated buffer, which is NUL
 * terminated. The returned buffer should be freed with free() when no longer
 * needed.
 */
#ifdef __GNUC__
__attribute__((malloc))
#endif
LIBBASE64_API char *
base64_encode_alloc(const unsigned char *in, size_t len, int options);

/*!
 * Decodes \a len Base64-encoded bytes from the \a in buffer to the \a out
 * buffer, using the decoding options specified in \a options. The \a out
 * buffer must be big enough to accommodate the entire decoded data. The \a in
 * and \a out pointers may be the same, in which case this function will
 * perform in-place decoding.
 *
 * If \a len is SIZE_MAX, this function decodes up until the NUL-terminating
 * character. Otherwise, it will process the NUL characters like any other.
 *
 * If there was an error during decoding, this function returns SIZE_MAX and
 * the contents of the \a out buffer are undefined. Otherwise, it returns the
 * number of bytes decoded.
 */
LIBBASE64_API size_t
base64_decode(unsigned char *out, const char *in, size_t len, int options);

/*!
 * Allocates a buffer and decodes \a len Base64-encoded bytes from the \a in
 * buffer to it, using the decoding options specified in \a options. The buffer
 * is returned in \a out and should be freed with free() when no longer needed.
 *
 * If \a len is SIZE_MAX, this function decodes up until the NUL-terminating
 * character. Otherwise, it will process the NUL characters like any other.
 *
 * If there was an error during decoding, this function returns SIZE_MAX and \a
 * out will be set to NULL. Otherwise, it returns the number of bytes decoded.
 */
LIBBASE64_API size_t base64_decode_alloc(unsigned char **out, const char *in, size_t len, int options);

#ifdef __cplusplus
}
#endif

#endif // LIBBASE64_BASE64_H
