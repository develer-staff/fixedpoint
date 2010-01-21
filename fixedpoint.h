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

#ifndef FIXEDPOINT_H
#define FIXEDPOINT_H

#include "fputils.h"
#include "anyint.h"
#include <iostream>
#include <stdio.h>
#include <cassert>
#include <string>
#include <math.h>

#ifdef FRACT_CHECKS_WITH_EXCEPTIONS
    #include <stdexcept>
    struct FractOverflowError : public std::runtime_error
    {
        FractOverflowError() : std::runtime_error("overflow") {}
    };

    struct FractDomainError : public std::runtime_error
    {
        FractDomainError() : std::runtime_error("domain") {}
    };

    void throwFractOverflowError(void) __attribute__((__noinline__));
    void throwFractOverflowError(void)
    {
        throw FractOverflowError();
    }

    void throwFractDomainError(void) __attribute__((__noinline__));
    void throwFractDomainError(void)
    {
        throw FractDomainError();
    }

    #define LIKELY(x)       __builtin_expect(!!(x), 1)
    #define UNLIKELY(x)     __builtin_expect(!!(x), 0)

    #define OVERFLOW_IF(x) do { if (UNLIKELY(x)) { throwFractOverflowError(); } } while(0)
    #define DOMAIN_IF(x)   do { if (UNLIKELY(x)) { throwFractDomainError(); } } while(0)
#else
    #define OVERFLOW_IF(x) assert(x)
    #define DOMAIN_IF(x)   assert(x)
#endif


// Fwd decl
template <int I, int F>
class Fract;

namespace detail {

    template <class T>
    struct FractBuilder
    {
        T x;
        FractBuilder(T x_) : x(x_) {}
    };

    //////////////////////////////////////////////////////////////////////////
    // Forward declarations
    //////////////////////////////////////////////////////////////////////////
    template <class IntType>
    std::string toString(IntType value, int F, int prec, bool zeropad);

    template <class IntType>
    IntType fromString(const std::string& s, int F, bool *ok);

    template <class IntType>
    std::string toHex(IntType value);   

    template <class IntType>
    IntType inverse(IntType x);
}

/////////////////////////////////////////////////////////////////////////
// fit_in -- check if a signed integer can fit in a specified number of bits
// Note: this function assumes *signed* integer, and the specified number
// of bits must include the sign bit.
/////////////////////////////////////////////////////////////////////////
template <class IntType>
bool fit_in(IntType x, int nbits) __attribute__((__always_inline__));

template <class IntType>
bool fit_in(IntType x, int nbits)
{
    assert(int(sizeof(IntType)*8) >= nbits);
    IntType imin = ~IntType(0) << (nbits-1);
    return int(sizeof(IntType)*8) >= nbits &&
           x <= ~imin && x >= imin;
}

template <class ToType, class FromType>
inline ToType fx_align(FromType x, int from_bits, int to_bits) __attribute__((__always_inline__));

template <class ToType, class FromType>
inline ToType fx_align(FromType x, int from_bits, int to_bits)
{
    if (from_bits > to_bits)
    {
        assert((from_bits - to_bits) < (int)sizeof(FromType)*8);
        return x >> (from_bits - to_bits);
    }
    else
        return ToType(x) << (to_bits - from_bits);
}


/////////////////////////////////////////////////////////////////////////////////////////
// Fract -- Fixed point type
//    Template arguments:
//        I - number of bits used in the integer part
//        F - number of bits used in the fractional part
//
// This class automatically selects the best underlying representation (that is: the
// fastest integer supported by the implementation that can fully represent the specified
// precision).
//
/////////////////////////////////////////////////////////////////////////////////////////
template <int I, int F>
class Fract
{
    STATIC_ASSERT(I > 0, "At least one bit needed in integer part for sign (or use FractU)");

private:
    typedef typename AnyInt::SelectFastest<I+F>::type IntType;
    typedef typename AnyInt::SelectSmallest<I>::type TruncIntType;
    IntType x;

    template <int I2, int F2>
    friend class Fract;

private:
    Fract(detail::FractBuilder<IntType> b) : x(b.x) {}

    template <class IntType2>
    void set(IntType2 x2, int F2)
    {
        OVERFLOW_IF(!fit_in(x2>>F2, I));

        x = (IntType(x2 >> F2) << F);

        x2 &= (1 << F2) - 1;
        x |= fx_align<IntType>(x2, F2, F);
    }

    IntType integ(void) const { return x >> F; }
    void fract(void) const { return x & ((1<<F)-1); }
    static Fract gen(IntType x) { return Fract(detail::FractBuilder<IntType>(x)); }

public:
    Fract() : x(0)
    {}

    template <class IntType2>
    Fract(IntType2 x2, int F2)
    {
        set(x2, F2);
    }

    Fract(const Fract& f)
    {
        x = f.x;
    }

    template <int I2, int F2>
    explicit Fract(const Fract<I2,F2>& f)
    {
        set(f.x, F2);
    }

    Fract(int i) __attribute__((__always_inline__))
    {
        OVERFLOW_IF(!fit_in(i, I));
        x = IntType(i) << F;
    }

    Fract(double f) __attribute__((__always_inline__))
    {
        x = f * (IntType(1) << F);
        OVERFLOW_IF((x >> F) != IntType(::floor(f)));
    }
    Fract(float f) __attribute__((__always_inline__))
    {
        x = f * (IntType(1) << F);
        OVERFLOW_IF((x >> F) != IntType(::floor(f)));
    }

    static Fract fromString(const std::string& s, bool *ok=NULL)
    {
        return gen(detail::fromString<IntType>(s, F, ok));
    }

    Fract& operator=(Fract f) { x = f.x; return *this; }
    Fract operator+(Fract f) { OVERFLOW_IF(AnyInt::AddOverflow(x, f.x)); return gen(x+f.x); }
    Fract operator-(Fract f) { OVERFLOW_IF(AnyInt::SubOverflow(x, f.x)); return gen(x-f.x); }
    Fract& operator+=(Fract f) { OVERFLOW_IF(AnyInt::AddOverflow(x, f.x)); x+=f.x; return *this; }
    Fract& operator-=(Fract f) { OVERFLOW_IF(AnyInt::SubOverflow(x, f.x)); x-=f.x; return *this; }
    bool operator<(Fract<I,F> f) const { return this->x < f.x; }
    bool operator==(Fract<I,F> f) const { return this->x == f.x; }

    template <int I2, int F2>
    Fract<I,F>& operator=(Fract<I2,F2> f) { set(f.x, F2); return *this; }
    template <int I2, int F2>
    Fract<I,F> operator+(Fract<I2,F2> f)  {return *this + Fract<I,F>(f); }
    template <int I2, int F2>
    Fract<I,F> operator-(Fract<I2,F2> f)  {return *this - Fract<I,F>(f); }
    template <int I2, int F2>
    Fract<I,F>& operator+=(Fract<I2,F2> f)  {return (*this += Fract<I,F>(f)); }
    template <int I2, int F2>
    Fract<I,F> operator-=(Fract<I2,F2> f)  {return (*this -= Fract<I,F>(f)); }

    TruncIntType floor() const
    {
        return TruncIntType(x >> F);
    }

    TruncIntType ceil() const
    {
        return TruncIntType((x + ((IntType(1)<<F)-1)) >> F);
    }

    float toFloat() const
    { return double(x) / float(IntType(1)<<F); }
    double toDouble() const
    { return double(x) / double(IntType(1)<<F); }

    std::string toString(int prec=-1, bool zeropad=false) const
    { return detail::toString(x, F, prec, zeropad); }

    std::string toHex() const
    { return detail::toHex(x); }

    Fract inverse(void) const
    {
        // On most processors, there is a division opcode using double-words
        // (eg: x86-32 has a fast "64bit div 32bit" opcode)
        if (sizeof(IntType) <= sizeof(long) && 0)
            return gen(Fract<I*2,F*2>(1).x / this->x);
        else
        {
            return gen(detail::inverse(this->x));
        }
    }

public:
    // Compute the number of bits of difference between a and b
    static int error(Fract a, Fract b)
    {
        return AnyInt::Log2Ceil(AnyInt::Abs(a.x - b.x));
    }

public:
    //////////////////////////////////////////////////////////////////
    // Friend functions
    //////////////////////////////////////////////////////////////////

    // Fast square root. The result has only half of
    // the argument precision, but it is fully correct up to that precision.
    friend Fract<I/2,F/2> sqrt_fast(Fract<I,F> x)
    {
        DOMAIN_IF(x<0);

        IntType temp, val=x.x, g=0, bshft=(AnyInt::Log2Ceil(val)-1)>>1, b=(1<<bshft);
        do
        {
            if (val >= (temp = ((g + g + b) << bshft)))
            {
               g += b;
               val -= temp;
            }
            b >>= 1;
        } while (bshft--);

        return Fract<I/2,F/2>::gen(g);
    }

    // Full square root. The result has the same precision of the argument,
    // but it requires doing intermediate calculations with values which are
    // two times bigger (eg: computing the square root on an uint32-based fract
    // requires 64-bit calculations).
    friend Fract<I,F> sqrt(Fract<I,F> x)
    {
        Fract<I*2,F*2> x2(x);
        return sqrt_fast(x2);
    }

    friend Fract abs(Fract x)
    {
        return gen(::abs(x.x));
    }
};



//////////////////////////////////////////////////////////////////////////////////////
// Integer inversion
//////////////////////////////////////////////////////////////////////////////////////

namespace detail {

    template <class IntType>
    IntType inverse(IntType x)
    {
        int shift = AnyInt::clz(x);
        x <<= shift;
        return (0x7fffffffu - (unsigned)x);
    }

} /* namespace detail */





//////////////////////////////////////////////////////////////////////////////////////
// String formatting functions
//////////////////////////////////////////////////////////////////////////////////////

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



#endif /* FIXEDPOINT_H */
