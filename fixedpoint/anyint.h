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
#define bitsof(t)   ((int)sizeof(t)*8)

    // Largest integer types supported by the compiler
    // FIXME: compiler/platform-dependent
    typedef long long Largest;
    typedef unsigned long long ULargest;

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
        int c = bitsof(IntType);
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
        char buf[bitsof(IntType)] = {0};
        int i = bitsof(IntType)-2;
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
        return bitsof(IntType) - clz(x);
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
    template <> struct Unsigned<uint8_t>  { typedef uint8_t type; };
    template <> struct Unsigned<uint16_t> { typedef uint16_t type; };
    template <> struct Unsigned<uint32_t> { typedef uint32_t type; };
    template <> struct Unsigned<uint64_t> { typedef uint64_t type; };

    //////////////////////////////////////////////////////////////////////////
    // DoubleType<T> - select the type which is two times bigger than T
    //////////////////////////////////////////////////////////////////////////
    template <class IntType> struct DoubleType;
    template <> struct DoubleType<int8_t> { typedef int16_t type; };
    template <> struct DoubleType<uint8_t> { typedef uint16_t type; };
    template <> struct DoubleType<int16_t> { typedef int32_t type; };
    template <> struct DoubleType<uint16_t> { typedef uint32_t type; };
    template <> struct DoubleType<int32_t> { typedef int64_t type; };
    template <> struct DoubleType<uint32_t> { typedef uint64_t type; };
#ifdef FRACT_HAS_128BITS
    template <> struct DoubleType<int64_t> { typedef int128_t type; };
    template <> struct DoubleType<uint64_t> { typedef uint128_t type; };
#endif

    //////////////////////////////////////////////////////////////////////////
    // IsSignExtension(x) - check if x is only a sign extension (x==0 || x==-1)
    //////////////////////////////////////////////////////////////////////////
    template <class IntType>
    bool IsSignExtension(IntType x)
    {
        typedef typename Unsigned<IntType>::type UIntType;

        // Clever bit-trick to do only one compare/branch
        UIntType ux = x;
        return (ux >> (bitsof(IntType)-1)) + ux == 0;
    }

    //////////////////////////////////////////////////////////////////////////
    // FitIn(a,n) - check if the signed number 'a' would fit in only 'n' bits
    //////////////////////////////////////////////////////////////////////////
    template <class IntType>
    bool FitIn(IntType x, int nbits)
    {
        assert(bitsof(IntType) >= nbits);

        if (CONSTANT(nbits))
        {
            IntType imin = ~IntType(0) << (nbits-1);
            return x <= ~imin && x >= imin;
        }
        else
            return IsSignExtension(x >> (nbits-1));
    }

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

    //////////////////////////////////////////////////////////////////////////
    // ScaledAdd<N>(a,b,shift) - compute (a+b) >> shift, taking care of not
    //  overflowing from the highest bit during the sum.
    //////////////////////////////////////////////////////////////////////////
    template <class IntType>
    IntType ScaledAdd(IntType a, IntType b, int shift, int N=bitsof(IntType))
    {
        if (N < bitsof(IntType))
            return (a+b) >> shift;

        typedef typename DoubleType<IntType>::type DIntType;
        return (DIntType(a) + b) >> shift;
    }

    template <>
    Largest ScaledAdd<Largest>(Largest a, Largest b, int shift, int N)
    {
        if (N < bitsof(Largest))
            return (a+b) >> shift;

        // (a+b)>>n =
        // (a+b) / 2^n =
        // (a+b)/2 / 2^(n-1) =
        // (a+(b-a)/2) / 2^(n-1) ==> never overflows!
        return (a + ((b-a) >> 1)) >> (shift-1);
    }

    template <>
    ULargest ScaledAdd<ULargest>(ULargest a, ULargest b, int shift, int N)
    {
        return (Largest)ScaledAdd((Largest)a, (Largest)b, shift, N);
    }

    //////////////////////////////////////////////////////////////////////////
    // MulHU(a,b) - get the highest part of the result of an unsigned multiplication
    //////////////////////////////////////////////////////////////////////////
    template <class IntType>
    IntType MulHU(IntType a, IntType b, int shift=bitsof(IntType))
    {
        assert(shift >= bitsof(IntType));
        typedef typename Unsigned<IntType>::type UIntType;
        typedef typename DoubleType<UIntType>::type DUIntType;
        return ((DUIntType)(UIntType)a * (UIntType)b) >> shift;
    }

    template <> ULargest MulHU(ULargest a, ULargest b, int shift)
    {
        assert(shift >= bitsof(ULargest));

        int hs = shift / 2;
        ULargest hs_mask = (ULargest(1) << hs) - 1;

        ULargest
            ahi = a >> hs,
            alo = a & hs_mask,
            bhi = b >> hs,
            blo = b & hs_mask;

        ULargest mid1 = ((ahi*blo) >> (shift-hs-1));
        ULargest mid2 = ((alo*bhi) >> (shift-hs-1));

        return ahi*bhi + ScaledAdd(mid1, mid2, 1, bitsof(ULargest)-(shift-hs-1));
    }

    template <> Largest MulHU(Largest a, Largest b, int shift)
    {
        return (Largest)MulHU((ULargest)a, (ULargest)b, shift);
    }
}

#endif // ANYINT_H
