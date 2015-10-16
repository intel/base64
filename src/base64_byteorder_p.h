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

#ifndef BASE64_BYTEORDER_P_H
#define BASE64_BYTEORDER_P_H

#include <stdint.h>
#include <string.h>

#ifndef __has_builtin
#  define __has_builtin(x)  0
#endif

#if (defined(__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 403)) || \
    __has_builtin(__builtin_bswap32)
#  if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#    define _base64_ntohl      __builtin_bswap32
#    define _base64_htonl      __builtin_bswap32
#  else
#    define _base64_ntohl
#    define _base64_htonl
#  endif
#elif defined(_MSC_VER)
/* MSVC, which implies Windows, which implies little-endian */
#  define _base64_ntohl        _byteswap_ulong
#  define _base64_htonl        _byteswap_ulong
#endif
#ifndef _base64_ntohl
#  include <arpa/inet.h>
#  define _base64_ntohl        ntohl
#  define _base64_htonl        htonl
#endif

static inline uint32_t _base64_ntohl_unaligned(unsigned const char *ptr)
{
    uint32_t val;
    memcpy(&val, ptr, sizeof(val));
    return _base64_ntohl(val);
}

#endif
