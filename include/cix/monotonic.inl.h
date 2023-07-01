// CIX C++ library
// Copyright (c) Jean-Charles Lefebvre
// SPDX-License-Identifier: MIT

namespace cix {


// ticks_now
#if CIX_PLATFORM_WINDOWS && WINVER < 0x0600  // pre-Vista
    inline ticks_t ticks_now()
    {
        return GetTickCount();
    }
#elif CIX_PLATFORM_WINDOWS && WINVER >= 0x0600  // Vista+
    inline ticks_t ticks_now()
    {
        return GetTickCount64();
    }
#elif !defined(CLOCK_MONOTONIC) && (CIX_PLATFORM_HPUX || CIX_PLATFORM_SOLARIS)
    inline ticks_t ticks_now()
    {
        const int64_t now = ::gethrtime();
        static_assert(sizeof(ticks_t) >= sizeof(now));
        return static_cast<ticks_t>(now / 1'000'000);
    }
#endif


#if defined(CIX_TICKS_64BIT)
    inline std::uint64_t ticks_now64()
    {
        return ticks_now();
    }

    inline std::uint32_t ticks_now32()
    {
        // CAUTION: ticks_elapsed() and ticks_to_go() will return huge gaps in
        // case of overflow
        return static_cast<std::uint32_t>(ticks_now() & 0xffffffff);
    }
#elif defined(CIX_TICKS_32BIT)
    inline std::uint64_t ticks_now64()
    {
        // CAUTION: ticks_elapsed() and ticks_to_go() will return huge gaps in
        // case of overflow
        return static_cast<std::uint64_t>(ticks_now());
    }

    inline std::uint32_t ticks_now32()
    {
        return ticks_now();
    }
#else
    #error ticks_now64() and ticks_now32() not implemented?
#endif


inline ticks_t ticks_elapsed(ticks_t start)
{
    return ticks_elapsed(start, ticks_now());
}


inline ticks_t ticks_to_go(ticks_t start, ticks_t end)
{
    return ticks_to_go(start, end, ticks_now());
}

}  // namespace cix
