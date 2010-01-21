/*
 * Fixed-point C++ library
 * Copyright (C) 2010, Giovanni Bajo <rasky@develer.com>
 * Copyright (C) 2010, Develer Srl
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * anyint: basic operations that apply to all compiler-supported integers
 * through templates.
 */

#ifndef ANYINT_H
#define ANYINT_H

#include "fputils.h"
#include <string>
#include <math.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

namespace AnyInt
{
    /////////////////////////////////////////////////////////////////////////
    // clz -- count leading zeros
    // Return the number of leading zero bits in an integer argument.
    // This operation is supported in hardware by most CPU, while it
    // requires an iteration in C. Luckily, GCC provides a builting.
    /////////////////////////////////////////////////////////////////////////
    template <class IntType>
    int clz(IntType x) __attribute__((__always_inline__));

    #ifdef __GNUC__
    template <> int clz(int x) __attribute__((__always_inline__));
    template <> int clz(int x) { return __builtin_clz(x); }
    template <> int clz(long x) __attribute__((__always_inline__));
    template <> int clz(long x) { return __builtin_clzl(x); }
    template <> int clz(long long x) __attribute__((__always_inline__));
    template <> int clz(long long x) { return __builtin_clzll(x); }
    #else
    // Generic C implementation (should be used only on hw without clz)
    template <class IntType>
    int clz(IntType x)
    {
        int c = sizeof(IntType)*8;
        while (x)
        {
            x >>= 1;
            ++c;
        }
        return c;
    }
    #endif

    /////////////////////////////////////////////////////////////////////////
    // Abs - Absolute value
    /////////////////////////////////////////////////////////////////////////
    template <class IntType>
    IntType Abs(IntType);

    template<> int Abs(int x) { return ::abs(x); }
    template<> long Abs(long x) { return ::labs(x); }
    template<> long long Abs(long long x) { return ::llabs(x); }

    /////////////////////////////////////////////////////////////////////////
    // ToString - format integer number to string
    // This is similar to the non-standard "itoa" provided by some compilers
    // like Visual Studio.
    /////////////////////////////////////////////////////////////////////////
    template <class IntType>
    std::string ToString(IntType val, int base=10)
    {
        if (val == 0)
            return "0";
        assert(base > 0 && base < 16);
        char buf[sizeof(IntType)*8] = {0};
        int i = sizeof(IntType)*8-2;
        for(; val && i ; --i, val /= base)
            buf[i] = "0123456789abcdef"[val % base];
        return &buf[i+1];
    }

    //////////////////////////////////////////////////////////////////////////
    // Log2Ceil - log2 of an integer number (rounded to the ceiling)
    //////////////////////////////////////////////////////////////////////////
    template <class IntType>
    int Log2Ceil(IntType x)
    {
        return sizeof(IntType)*8 - clz(x);
    }


    //////////////////////////////////////////////////////////////////////////
    // SelectFastest<N> - select the builtin integer type that is able to represent
    //  a N-bits value, and which produces the best code when using it.
    //////////////////////////////////////////////////////////////////////////
    struct error_invalid_type;

    template <int N>
    struct SelectFastest
    {
        // FIXME: benchmark the fastest integer types on different CPUs.
        // eg: on x86, 16bits integers are very slow.
        typedef typename detail::if_t< (N<=8), int8_t,
            typename detail::if_t< (N<=32), int32_t,
                typename detail::if_t< (N<=64), int64_t,
                    error_invalid_type
                >::type
            >::type
        >::type type;
    };

    //////////////////////////////////////////////////////////////////////////
    // SelectSmallest<N> - select the smallest builtin integer type that is able
    // to represent a N-bits value.
    //////////////////////////////////////////////////////////////////////////
    template <int N>
    struct SelectSmallest
    {
        typedef typename detail::if_t< (N<=8), int8_t,
            typename detail::if_t< (N<=16), int16_t,
                typename detail::if_t< (N<=32), int32_t,
                    typename detail::if_t< (N<=64), int64_t,
                        error_invalid_type
                    >::type
                >::type
            >::type
        >::type type;
    };

    //////////////////////////////////////////////////////////////////////////
    // Bigger<A,B> - select the biggest integer type among arguments.
    //////////////////////////////////////////////////////////////////////////
    template <class A, class B>
    struct Bigger
    {
        typedef typename detail::if_t<(sizeof(A)>=sizeof(B)), A, B>::type type;
    };

    //////////////////////////////////////////////////////////////////////////
    // Unsigned<T> - given a signed integer type T, return its unsigned counterpart
    //////////////////////////////////////////////////////////////////////////
    template <class IntType> struct Unsigned;
    template <> struct Unsigned<int8_t>  { typedef uint8_t type; };
    template <> struct Unsigned<int16_t> { typedef uint16_t type; };
    template <> struct Unsigned<int32_t> { typedef uint32_t type; };
    template <> struct Unsigned<int64_t> { typedef uint64_t type; };


    //////////////////////////////////////////////////////////////////////////
    // AddOverflow(a,b) - check if there will be an overflow when adding a
    //   and b (a+b)
    //////////////////////////////////////////////////////////////////////////
    template <class IntType>
    bool AddOverflow(IntType a, IntType b)
    {
        // Switch to unsigned types to get wrapping beahviour on sum overflow
        // (signed types have undefined behavior on overflow)
        typedef typename Unsigned<IntType>::type UIntType;

        UIntType aa = a, bb = b, sum = aa+bb;
        return IntType((aa ^ sum) & (bb ^ sum)) < 0;
    }

    //////////////////////////////////////////////////////////////////////////
    // SubOverflow(a,b) - check if there will be an overflow when subtracting
    //   b from a (a-b)
    //////////////////////////////////////////////////////////////////////////
    template <class IntType>
    bool SubOverflow(IntType a, IntType b)
    {
        // Switch to unsigned types to get wrapping beahviour on sub overflow
        // (signed types have undefined behavior on overflow)
        typedef typename Unsigned<IntType>::type UIntType;

        UIntType aa = a, bb = b, diff = aa-bb;
        return IntType((bb ^ aa) & (bb ^ diff)) < 0;
    }


}




#endif // ANYINT_H