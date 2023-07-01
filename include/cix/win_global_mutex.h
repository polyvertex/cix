// CIX C++ library
// Copyright (c) Jean-Charles Lefebvre
// SPDX-License-Identifier: MIT

#pragma once

#include "detail/ensure_cix.h"

#if CIX_PLATFORM_WINDOWS

namespace cix {

class win_global_mutex
{
public:
    CIX_NONCOPYABLE(win_global_mutex)

    win_global_mutex() = delete;

    explicit win_global_mutex(const wchar_t* name)
    {
        assert(name);
        assert(*name);
        SetLastError(0);
        m_handle = CreateMutexW(nullptr, FALSE, name);
        m_error = GetLastError();
        assert(m_error == 0 || m_error == ERROR_ALREADY_EXISTS);
    }

    ~win_global_mutex()
    {
        if (m_handle)
        {
            CloseHandle(m_handle);
            m_handle = nullptr;
        }
    }

    bool owned() const { return m_handle != nullptr; }

    DWORD error() const { return m_error; }

private:
    HANDLE m_handle;
    DWORD m_error;
};

}  // namespace cix

#endif  // #if CIX_PLATFORM_WINDOWS
