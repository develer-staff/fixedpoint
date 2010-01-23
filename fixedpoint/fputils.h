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
 * fputils: internal helpers
 */

#ifndef FPUTILS_H
#define FPUTILS_H

#define countof(arr) (sizeof(arr)/sizeof(arr[0]))

namespace detail
{
    /////////////////////////////////////////////////////////////////////////
    // if_t -- if with types
    /////////////////////////////////////////////////////////////////////////
    template <bool COND, typename TRUE_T, typename FALSE_T>
    struct if_t;

    template <typename TRUE_T, typename FALSE_T>
    struct if_t<true, TRUE_T, FALSE_T>
    {
        typedef TRUE_T type;
    };

    template <typename TRUE_T, typename FALSE_T>
    struct if_t<false, TRUE_T, FALSE_T>
    {
        typedef FALSE_T type;
    };


    /////////////////////////////////////////////////////////////////////////
    // STATIC_ASSERT - compile-time assertions
    // NOTE: GCC has builtin support for C++0x static_assert(), if activated
    // on command line with -std=c++0x (useful for debugging)
    /////////////////////////////////////////////////////////////////////////
    #ifdef __GXX_EXPERIMENTAL_CXX0X__
        #define STATIC_ASSERT(x, msg) static_assert(x, msg)
    #else
        template <bool> struct StaticAssert;
        template <> struct StaticAssert<true> { typedef int COMPILE_TIME_ERROR; };
        #define STATIC_ASSERT(x, msg) typedef typename detail::StaticAssert<(x)>::COMPILE_TIME_ERROR _STATIC_ASSERT_ ## __LINE__
    #endif

    #ifndef __GNUC__
        #define __attribute__(x)
    #endif

    // INLINE: force inling
    #define INLINE    __attribute__((__always_inline__))

    // FLATTEN: force recursive inlining of all called functions,
    //  making the function "flat".
    #define FLATTEN   __attribute__((__flatten__))

    // CONSTANT: check if the compiler sees x as a compile-time constant
    #define CONSTANT(x)  __builtin_constant_p(x)
}


#endif // FPUTILS_H
