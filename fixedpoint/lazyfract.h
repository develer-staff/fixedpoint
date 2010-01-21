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

#ifndef LAZYFRACT_H
#define LAZYFRACT_H

#include <tr1/tuple>
#include "anyint.h"

namespace detail {

    ///////////////////////////////////////////////////
    // LazyFract
    //
    // This class holds a lazy fixed point number. With lazy,
    // we mean that this class does not hold a number: instead,
    // it holds a pointer to a function that will evaluate the
    // number, at a given precision.
    //
    // For instance, given the following code:
    //
    // Fract<8,8> f = sin(x)*cos(x);
    //
    // We would like sin() and cos() to compute their results
    // only to the extent necessary to evaluate 8 bits of precision
    // for the result. This is achieved by delaying calculation
    // of sin(x) (and cos(x)) until the result is assigned to
    // a fractional number; only at that point we know the required
    // precions and can evaluate sin and cos appropriately.
    //
    // Thus, the intermediate result must be stored somehow,
    // and this is done with LazyFract.
    //
    /////////////////////////////////////////////////////

    template <class Derived>
    class LazyFract
    {
    public:
        template <int I, int F>
        Fract<I,F> operator*(LazyFract a, Fract<I,F> b)
        {
            typedef typename Fract<I,F>::IntType IntType;

            IntType inv; int shift, int highestbit;
            tie(inv,shift, highestbit) = Derived::evaluate<IntType>(I+F);

            if (!highestbit)
                result = AnyInt::MulHU(inv, b.x) >> shift;
            else
                result = AnyInt::ScaledAdd<I+F>(AnyInt::MulHU(inv, b.x), b.x, shift);
        }
    };



} /* namespace detail */

#endif // LAZYFRACT_H
