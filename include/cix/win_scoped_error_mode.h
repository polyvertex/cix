// CIX C++ library
// Copyright (c) Jean-Charles Lefebvre
// SPDX-License-Identifier: MIT

#pragma once

#include "detail/ensure_cix.h"

#if CIX_PLATFORM_WINDOWS

namespace cix {

class win_scoped_error_mode
{
public:
    enum : DWORD
    {
        QUIET =
            SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX |
            SEM_NOOPENFILEERRORBOX,

        INVALID_ERRMODE = std::numeric_limits<DWORD>::max(),
    };

public:
    CIX_NONCOPYABLE(win_scoped_error_mode)

    explicit win_scoped_error_mode(DWORD new_error_mode=QUIET)
    {
        if (!SetThreadErrorMode(new_error_mode, &m_old_error_mode))
            m_old_error_mode = INVALID_ERRMODE;
    }

    ~win_scoped_error_mode()
    {
        if (m_old_error_mode != INVALID_ERRMODE)
        {
            const auto error = GetLastError();

            SetThreadErrorMode(m_old_error_mode, nullptr);
            m_old_error_mode = INVALID_ERRMODE;

            // so that the caller can still rely on GetLastError() value
            SetLastError(error);
        }
    }

    bool succeeded() const noexcept
    {
        return m_old_error_mode != INVALID_ERRMODE;
    }

private:
    DWORD m_old_error_mode;
};

}  // namespace cix

#endif  // #if CIX_PLATFORM_WINDOWS
