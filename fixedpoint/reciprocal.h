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

#ifndef RECIPROCAL_H
#define RECIPROCAL_H

#include "lazyfract.h"

namespace detail
{
    /*
     * The reciprocal algorithm is taken from:
     * http://ipa.ece.illinois.edu/mif/pubs/web-only/Frank-RawMemo12-1999.html
     * (mirrored in doc/).
     *
     * I don't even attempt to explain it here because it is complicated. The main
     * idea is to use the standard Newton-Raphson iteration (x' = x*(2 - x*d)), but
     * the implementation uses lots of clever bit-tricks to speed it up. The seeding
     * of the iteration is also calculated without a lookup table, again with a
     * clever trick.
     */

    template <class IntType>
    class LazyReciprocal : public LazyFract<LazyReciprocal<IntType> >
    {
    private:
        IntType input;
        int input_shift;

    private:
        template <int PREC>
        void nr_step(IntType& result, IntType input, int& curprec) const
        {
            enum { NBITS = sizeof(IntType)*8 };

            if ((PREC/2) < NBITS)
            {
                result = AnyInt::MulHU(result, -AnyInt::MulHU(result, input)) << 1;
                curprec = (PREC > NBITS) ? (NBITS-2) : PREC;
            }
        }

    public:
        template <int I, int F>
        LazyReciprocal(Fract<I,F> f)
            : input(f.x), input_shift(F)
        {}

    public:
        template <int prec>
        IntType evaluate() const
        {
            enum { NBITS = bitsof(IntType) };

            int shift = AnyInt::clz(input);

            this->result_highestbit = 0;
            this->result_shift = NBITS + (NBITS-shift) - input_shift - 1;

            IntType input = this->input << shift;
            if ((input << 1) == 0)  // Power of two
            {
                --this->result_shift;
                return input;
            }

            IntType result = 1;

            // 3-bits estimation
            result = ((~IntType(0) ^ (IntType(1)<<(NBITS-1))) - input);
            if (prec <= 3)
                return result;

            int curprec = 3;
            STATIC_ASSERT(NBITS <= 128, Integer larger than 128 bits are unsupported by the following unrolled loop);

            nr_step<6>(result, input, curprec);
            if (curprec >= prec)
                return result - (AnyInt::MulHU(result, input) << 1);

            nr_step<12>(result, input, curprec);
            if (curprec >= prec)
                return result - (AnyInt::MulHU(result, input) << 1);

            nr_step<24>(result, input, curprec);
            if (curprec >= prec)
                return result - (AnyInt::MulHU(result, input) << 1);

            nr_step<48>(result, input, curprec);
            if (curprec >= prec)
                return result - (AnyInt::MulHU(result, input) << 1);

            nr_step<96>(result, input, curprec);
            if (curprec >= prec)
                return result - (AnyInt::MulHU(result, input) << 1);

            nr_step<192>(result, input, curprec);
            if (curprec >= prec)
                return result - (AnyInt::MulHU(result, input) << 1);

            // Highest bit is always one at this point
            assert(result < 0);
            result <<= 1;
            curprec--;
            this->result_highestbit = 1;
            this->result_shift += 1;

            result -= 3;

            result -= AnyInt::MulHU(result, input) + input; curprec++;
            assert(curprec == NBITS-2);
            if (curprec >= prec)
                return result;
            result -= AnyInt::MulHU(result, input) + input; curprec++;
            assert(curprec == NBITS-1);
            if (curprec >= prec)
                return result;
            result -= AnyInt::MulHU(result, input) + input; curprec++;
            assert(curprec == NBITS);
            return result;
        }
    };
}

#endif // RECIPROCAL_H
