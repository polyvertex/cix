// CIX C++ library
// Copyright (c) Jean-Charles Lefebvre
// SPDX-License-Identifier: MIT

#pragma once

#include "detail/ensure_cix.h"

// a monotonic clock with millisecond precision

namespace cix {

#ifdef CIX_TICKS_64BIT
#error CIX_TICKS_64BIT manually defined?
#endif

#ifdef CIX_TICKS_32BIT
#error CIX_TICKS_32BIT manually defined?
#endif


// ticks_t (milliseconds)
#if CIX_PLATFORM_WINDOWS && WINVER < 0x0600  // pre-Vista
    typedef std::uint32_t ticks_t;

    #define CIX_TICKS_32BIT
#else
    typedef std::uint64_t ticks_t;

    #define CIX_TICKS_64BIT
#endif


static constexpr ticks_t ticks_max = std::numeric_limits<ticks_t>::max();
static constexpr ticks_t ticks_second = 1000;
static constexpr ticks_t ticks_minute = 60 * ticks_second;
static constexpr ticks_t ticks_hour = 60 * ticks_minute;
static constexpr ticks_t ticks_day = 24 * ticks_hour;


ticks_t ticks_now();

// CAUTION when using ticks_now64() and ticks_now32() since ticks_elapsed() and
// ticks_to_go() will return huge gaps in case of overflow, depending on ticks_t
// bitness.
std::uint64_t ticks_now64();
std::uint32_t ticks_now32();

ticks_t ticks_elapsed(ticks_t start);
ticks_t ticks_elapsed(ticks_t start, ticks_t now);
ticks_t ticks_to_go(ticks_t start, ticks_t end);
ticks_t ticks_to_go(ticks_t start, ticks_t end, ticks_t now);

std::string ticks_to_string(ticks_t milliseconds);
std::wstring ticks_to_wstring(ticks_t milliseconds);

}  // namespace cix


#include "monotonic.inl.h"
