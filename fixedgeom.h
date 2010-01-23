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
 * This file contains a simple fixed-point N-dimensional geometric library.
 * It is useful to implement siple geometric computation with fixed-point numbers.
 */

#ifndef FIXEDGEOM_H
#define FIXEDGEOM_H

#include "fixedpoint.h"

template <int I, int F>
class Vector3D
{
    enum { DIMS = 3 };
    typedef Fract<I,F> VFract;

    VFract c[DIMS];

public:
    Vector3D() {}

    template <class T0, class T1>
    Vector3D(T0 a0, T1 a1)
    {
        c[0] = a0;
        c[1] = a1;
    }
    template <class T0, class T1, class T2>
    Vector3D(T0 a0, T1 a1, T2 a2)
    {
        c[0] = a0;
        c[1] = a1;
        c[2] = a2;
    }

    Vector3D(const Vector3D& v)
    {
        *this = v;
    }

public:
    Vector3D& operator=(const Vector3D& v)
    {
        for (int i=0;i<DIMS;i++)
            c[i] = v.c[i];
        return *this;
    }

    template <int I2, int F2>
    Vector3D operator*(Fract<I2,F2> f)
    {
        Vector3D m;
        for (int i=0;i<DIMS;i++)
            m.c[i] = c[i]*f;
        return m;
    }
    template <int I2, int F2>
    Vector3D& operator*=(Fract<I2,F2> f)
    {
        for (int i=0;i<DIMS;i++)
            c[i] *= f;
        return *this;
    }

    // Return the square of the vector modulus
    VFract mod2() const
    {
        VFract m = 0;
        for (int i=0;i<DIMS;i++)
            m += c[i]*c[i];
        return m;
    }

    // Return the vector modulus (length)
    VFract mod() const
    {
        return sqrt(mod2());
    }

    // Return the normalized vector (direction)
    VFract dir() const
    {
        return *this * rsqrt(mod2());
    }

public:
    // Alias for vector's length
    friend VFract abs(const Vector3D& v)
    {
        return v.mod();
    }
};

#endif // FIXEDGEOM_H
