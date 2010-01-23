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

#include "fixedpoint_config.h"
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
}

// Internal functions
#include "fixedpoint/fputils.h"
#include "fixedpoint/anyint.h"
#include "fixedpoint/stringify.h"
#include "fixedpoint/reciprocal.h"

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

    template <class T>
    friend class detail::LazyFract;

private:
    Fract(detail::FractBuilder<IntType> b) : x(b.x) {}

    template <class IntType2>
    void set(IntType2 x2, int F2)
    {
        OVERFLOW_IF(!fit_in(x2>>F2, I));

        x = (IntType(x2 >> F2) << F);

        x2 &= (IntType(1) << F2) - 1;
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

    template <class T>
    explicit Fract(const detail::LazyFract<T>& f)
    {
        *this = f.template toFract<I,F>();
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
    Fract operator+(Fract f) const { OVERFLOW_IF(AnyInt::AddOverflow(x, f.x)); return gen(x+f.x); }
    Fract operator-(Fract f) const { OVERFLOW_IF(AnyInt::SubOverflow(x, f.x)); return gen(x-f.x); }
    Fract& operator+=(Fract f) { OVERFLOW_IF(AnyInt::AddOverflow(x, f.x)); x+=f.x; return *this; }
    Fract& operator-=(Fract f) { OVERFLOW_IF(AnyInt::SubOverflow(x, f.x)); x-=f.x; return *this; }
    bool operator<(Fract<I,F> f) const { return this->x < f.x; }
    bool operator==(Fract<I,F> f) const { return this->x == f.x; }

    template <int I2, int F2>
    Fract<I,F>& operator=(Fract<I2,F2> f) { set(f.x, F2); return *this; }
    template <int I2, int F2>
    Fract<I,F> operator+(Fract<I2,F2> f) const {return *this + Fract<I,F>(f); }
    template <int I2, int F2>
    Fract<I,F> operator-(Fract<I2,F2> f) const {return *this - Fract<I,F>(f); }
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


    template <class T>
    friend class detail::LazyReciprocal;

    friend detail::LazyReciprocal<IntType> reciprocal(Fract f)
    {
        // On most processors, there is a division opcode using double-words
        // (eg: x86-32 has a fast "64bit div 32bit" opcode)
#ifndef FRACT_AVOID_DIVISION
        if (sizeof(IntType) <= sizeof(long))
            return gen(Fract<I*2,F*2>(1).x / this->x);
#endif
        return detail::LazyReciprocal<IntType>(f);
    }
};

#endif /* FIXEDPOINT_H */
