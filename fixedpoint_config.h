#ifndef FIXEDPOINT_CONFIG_H
#define FIXEDPOINT_CONFIG_H

#ifdef __x86_64__
    #define FRACT_HAS_128BITS
    typedef __uint128_t uint128_t;
    typedef __int128_t long int128_t;
#endif

// Avoid using any division
#define FRACT_AVOID_DIVISION

#endif // FIXEDPOINT_CONFIG_H
