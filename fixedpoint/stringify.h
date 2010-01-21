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
 * stringify: string formatting functions
 */

#ifndef STRINGIFY_H
#define STRINGIFY_H

#include "anyint.h"

namespace detail {

    template <class Derived, class IntType>
    struct Pow10BaseFuncs
    {
        static IntType pow10(int exp)
        {
            assert(exp >= 0);
            assert(exp < int(countof(Derived::pow10_table)));
            return Derived::pow10_table[exp];
        }

        // Return log10(pow2(exp))
        static int log10_pow2(int exp)
        {
            assert(exp >= 0);
            assert(exp < int(countof(Derived::log10_table)));
            return Derived::log10_table[exp];
        }

        // Return (num / 10^exp), within a fixed point with F fractional bits
        // This compute the result with the maximum possible precision within
        // IntType.
        static IntType div_pow10(int num, int exp, int F)
        {
            typedef typename AnyInt::Unsigned<IntType>::type UIntType;

            assert(num > 0);
            assert(exp > 0);
            assert(exp*2 < int(countof(Derived::pow10_inv_table)));

            int intbits = AnyInt::Log2Ceil(num);

            UIntType value = (UIntType)Derived::pow10_inv_table[exp*2];
            int value_shift = sizeof(IntType)*8 + Derived::pow10_inv_table[exp*2+1];
            value >>= intbits; value_shift -= intbits;
            value *= num;
            value >>= 1; value_shift -= 1;

            if (value_shift > F)
            {
                // A >> B is undefined if B > NUM_BITS(A)!
                if (value_shift - F > (int)sizeof(IntType)*8)
                    return 0;
                return (value + (1 << (value_shift-F-1))) >> (value_shift-F);
            }
            else
            {
                return value << (F - value_shift);
            }
        }
    };

    template <class IntType> struct Pow10Funcs;
    template <> struct Pow10Funcs<int32_t> : public Pow10BaseFuncs<Pow10Funcs<int32_t>, int32_t >
    {
        enum { MAX_LOG10 = 9, INV_ONE = 1U << 31 };
        static const int32_t pow10_table[MAX_LOG10+1];
        static const int32_t pow10_inv_table[(MAX_LOG10+1)*2];
        static const int log10_table[32];
    };
    template <> struct Pow10Funcs<int64_t> : public Pow10BaseFuncs<Pow10Funcs<int64_t>, int64_t >
    {
        enum { MAX_LOG10 = 18, INV_ONE = 1ULL << 63 };
        static const int64_t pow10_table[MAX_LOG10+1];
        static const int64_t pow10_inv_table[(MAX_LOG10+1)*2];
        static const int log10_table[64];
    };

    const int32_t Pow10Funcs<int32_t>::pow10_table[] =
    {
        1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000
    };

    const int32_t Pow10Funcs<int32_t>::pow10_inv_table[] =
    {
        // Computed with invpow10.py
        0xffffffff, 0,
        0xcccccccc, 3,
        0xa3d70a3d, 6,
        0x83126e97, 9,
        0xd1b71758, 13,
        0xa7c5ac47, 16,
        0x8637bd05, 19,
        0xd6bf94d5L, 23,
        0xabcc7711L, 26,
    };
    const int Pow10Funcs<int32_t>::log10_table[] =
    {
        0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9
    };
    const int64_t Pow10Funcs<int64_t>::pow10_table[19] =
    {
        1LL, 10LL, 100LL, 1000LL, 10000LL, 100000LL, 1000000LL, 10000000LL, 100000000LL,
        1000000000LL,
        10000000000LL,
        100000000000LL,
        1000000000000LL,
        10000000000000LL,
        100000000000000LL,
        1000000000000000LL,
        10000000000000000LL,
        100000000000000000LL,
        1000000000000000000LL,
    };

    const int Pow10Funcs<int64_t>::log10_table[] =
    {
        0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9,
        9, 9, 10, 10, 10, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 18, 18
    };
    const int64_t Pow10Funcs<int64_t>::pow10_inv_table[] =
    {
        // Computed with invpow10.py
        0xffffffffffffffffLL, 0,
        0xccccccccccccccccLL, 3,
        0xa3d70a3d70a3d70aLL, 6,
        0x83126e978d4fdf3bLL, 9,
        0xd1b71758e219652bLL, 13,
        0xa7c5ac471b478423LL, 16,
        0x8637bd05af6c69b5LL, 19,
        0xd6bf94d5e57a42bcLL, 23,
        0xabcc77118461cefcLL, 26,
        0x89705f4136b4a597LL, 29,
        0xdbe6fecebdedd5beLL, 33,
        0xafebff0bcb24aafeLL, 36,
        0x8cbccc096f5088cbLL, 39,
        0xe12e13424bb40e13LL, 43,
        0xb424dc35095cd80fLL, 46,
        0x901d7cf73ab0acd9LL, 49,
        0xe69594bec44de15bLL, 53,
        0xb877aa3236a4b449LL, 56,
        0x9392ee8e921d5d07LL, 59,
    };

    template <class IntType>
    std::string toString(IntType value, int F, int prec, bool zeropad)
    {
        typedef typename AnyInt::Unsigned<IntType>::type UIntType;
        typedef Pow10Funcs<IntType> Pow10Funcs;

        if (prec == -1)
            prec = Pow10Funcs::log10_pow2(F);
        else if (prec >= (int)Pow10Funcs::MAX_LOG10)
            prec = (int)Pow10Funcs::MAX_LOG10-1;

        std::string integ, frac;
        UIntType uvalue;

        if (value < 0)
        {
            integ += "-";
            uvalue = -value;
        }
        else
        {
            uvalue = value;
        }

        // Add .5 to the last digit of wanted precision
        uvalue += Pow10Funcs::div_pow10(5, prec+1, F);

        integ += AnyInt::ToString(uvalue >> F) + ".";

        for (int k = 0; k < prec; ++k)
        {
            uvalue &= (UIntType(1) << F) - 1;
            if (!zeropad && uvalue == 0)
                break;
            uvalue *= 10;
            assert((uvalue >> F) < 10);
            frac += '0' + (uvalue >> F);
        }

        if (!zeropad)
        {
            while (frac.length() > 0 && frac[frac.length()-1] == '0')
                frac.resize(frac.length()-1);
        }

        if (frac.length() == 0)
            frac.push_back('0');

        return integ + frac;
    }

    template <class IntType>
    IntType fromString(const std::string &s, int F, bool *ok)
    {
        typedef Pow10Funcs<IntType> Pow10Funcs;
        IntType xi = 0;
        IntType xf = 0;
        size_t i = 0;
        bool negate = false;

        while (i < s.length() && isspace(s[i]))
            ++i;

        if (s[i] == '-')
        {
            negate = true;
            ++i;
        }

        for (;;++i)
        {
            if (i == s.length())
            {
                if (ok) *ok = true;
                return xi << F;
            }

            if (s[i] >= '0' && s[i] <= '9')
            {
                xi *= 10;
                xi += s[i] - '0';
                continue;
            }
            else if (s[i] == '.')
                break;
            else
            {
                if (ok) *ok = false;
                return IntType(-1);
            }
        }

        // Compute fractional part at highest precision
        size_t fi = 1;
        for (++i; i < s.length() && fi < countof(Pow10Funcs::pow10_inv_table)/2; ++i, ++fi)
        {
            if (s[i] >= '1' && s[i] <= '9')
            {
                int digit = s[i] - '0';
                // Compute digit * 10^(-fi), at the highest possible precision
                IntType ipow10 = Pow10Funcs::div_pow10(digit, fi, sizeof(IntType)*8-1);
                xf += ipow10;
            }
            else if (s[i] == '0')
                ;
            else
            {
                if (ok) *ok = false;
                return IntType(-1);
            }
        }

        int xfshift = (sizeof(IntType)*8-1-F);
        IntType result = (xi << F) | ((xf + (IntType(1) << (xfshift-1))) >> xfshift);
        if (negate)
            result = -result;
        if (ok) *ok = true;
        return result;
    }

    template <class IntType>
    std::string toHex(IntType value)
    {
        typedef typename AnyInt::Unsigned<IntType>::type UIntType;
        char buf[sizeof(IntType)*2+3];

        UIntType x = (UIntType)value;
        size_t i = sizeof(IntType)*2;
        buf[i] = 0;
        while (i != 1)
        {
            buf[--i] = "0123456789abcdef"[x & 0xF];
            x >>= 4;
        }
        buf[i] = 'x';
        buf[0] = '0';
        return buf;
    }


} /* namespace detail */


#endif // STRINGIFY_H
