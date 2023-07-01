// CIX C++ library
// Copyright (c) Jean-Charles Lefebvre
// SPDX-License-Identifier: MIT

#include <cix/cix>
#include <cix/detail/intro.h>

namespace cix {

static_assert(
    std::is_integral<ticks_t>::value &&
    std::is_unsigned<ticks_t>::value &&
    sizeof(ticks_t) >= 2,
    "this code assumes ticks_t is an unsigned integral type");


#if !defined(_WIN32) && defined(CLOCK_MONOTONIC)
ticks_t ticks_now()
{
    // ref: CPython's pytime.c

    struct timespec ts;

    if (0 != clock_gettime(CLOCK_MONOTONIC, &ts))
        CIX_THROW_RUNTIME("clock_gettime failed (error {})", errno);

    ticks_t milliseconds = static_cast<ticks_t>(ts.tv_sec) * 1'000;

    // paranoid check
    while (ts.tv_nsec >= 1'000'000'000L)
    {
        milliseconds += 1'000;
        ts.tv_nsec -= 1'000'000'000L;
    }

    return milliseconds + static_cast<ticks_t>(ts.tv_nsec / 1'000'000);
}
#endif


ticks_t ticks_elapsed(ticks_t start, ticks_t now)
{
    if (now >= start)
        return now - start;
    else
        return (ticks_max - start) + 1 + now;
}


ticks_t ticks_to_go(ticks_t start, ticks_t end, ticks_t now)
{
    if (start <= end)
    {
        if (now <= end && now >= start)
            return end - now;
        else
            return 0;
    }
    else if (now >= start)
    {
        return (ticks_max - now) + 1 + end;
    }
    else if (now <= end)
    {
        return end - now;
    }
    else
    {
        return 0;
    }
}


std::string ticks_to_string(ticks_t milliseconds)
{
    if (milliseconds < ticks_second)
    {
        return fmt::format(
            "0.{:03} sec", milliseconds);
    }
    else if (milliseconds < ticks_minute)
    {
        return fmt::format(
            "{}.{:03} sec",
            milliseconds / ticks_second,
            milliseconds % ticks_second);
    }
    else
    {
        std::string out;

        out.reserve(32);

        if (milliseconds >= ticks_day)
        {
            const ticks_t days = milliseconds / ticks_day;
            milliseconds = milliseconds % ticks_day;
            out += fmt::format("{} day{}", days, (days > 1) ? "s" : "");
        }

        if (!out.empty() || milliseconds >= ticks_hour)
        {
            if (!out.empty())
                out += " and ";
            out += fmt::format("{:02}", milliseconds / ticks_hour);
            milliseconds = milliseconds % ticks_hour;
        }

        if (!out.empty() || milliseconds >= ticks_minute)
        {
            if (!out.empty())
                out += ':';
            out += fmt::format("{:02}", milliseconds / ticks_minute);
            milliseconds = milliseconds % ticks_minute;
        }

        if (!out.empty() || milliseconds >= ticks_second)
        {
            if (!out.empty())
                out += ':';
            out += fmt::format("{:02}", milliseconds / ticks_second);
            milliseconds = milliseconds % ticks_second;
        }

        assert(milliseconds < ticks_second);

        if (milliseconds)
        {
            if (!out.empty())
                out += '.';
            out += fmt::format("{:03}", milliseconds);
        }

        return out;
    }
}


std::wstring ticks_to_wstring(ticks_t milliseconds)
{
    const auto str = ticks_to_string(milliseconds);
    const auto len = str.length();
    std::wstring out(len, 0);

    // this is ugly but safe (and more optimized?) since we know
    // ticks_to_string() only outputs ASCII characters
    for (std::size_t idx = 0; idx < len; ++idx)
    {
        const auto c = str[idx];

        if (c >= 32 && c <= 126)
        {
            out[idx] = wchar_t(c);
        }
        else
        {
            assert(0);
            out[idx] = wchar_t('?');
        }
    }

    // paranoid checks to ensure null-byte is there
    assert(out.length() == len);
    assert(::wcsnlen_s(out.c_str(), len + 1) == len);

    return out;
}


}  // namespace cix
