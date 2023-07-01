// CIX C++ library
// Copyright (c) Jean-Charles Lefebvre
// SPDX-License-Identifier: MIT

#pragma once

#include "detail/ensure_cix.h"

#if !CIX_PLATFORM_WINDOWS || !defined(CIX_DEBUG)

#define CIX_WINMEMDBG(init_mode)  ((void)0)
#define CIX_WINMEMDBG_ENABLE  ((void)0)
#define CIX_WINMEMDBG_DISABLE  ((void)0)
#define CIX_WINMEMDBG_RESTORE  ((void)0)
#define CIX_WINMEMDBG_ADD(flags)  ((void)0)
#define CIX_WINMEMDBG_SET(flags)  ((void)0)

#else

#define CIX_WINMEMDBG(init_mode) \
    ::cix::win_memdbg _cix_win_memdbg_guard_( \
        CIX_CONCAT(::cix::win_memdbg::initmode_, init_mode))
#define CIX_WINMEMDBG_ENABLE  _cix_win_memdbg_guard_.enable()
#define CIX_WINMEMDBG_DISABLE  _cix_win_memdbg_guard_.disable()
#define CIX_WINMEMDBG_RESTORE  _cix_win_memdbg_guard_.restore()
#define CIX_WINMEMDBG_ADD(flags)  _cix_win_memdbg_guard_.add_flags(flags)
#define CIX_WINMEMDBG_SET(flags)  _cix_win_memdbg_guard_.set_flags(flags)


namespace cix {

// Control MSVC's CRT memory corruption and leaking tracking.
//
// ``_CrtSetDbgFlag()`` is defined only when ``_DEBUG`` is defined.
class win_memdbg
{
public:
    enum initial_mode_t
    {
        initmode_keep = 1,  // keep current state
        initmode_enable = 2,  // enable memory leak tracking
        initmode_disable = 3,  // disable memory leak tracking
    };

public:
    CIX_NONCOPYABLE(win_memdbg)

    win_memdbg()
        : m_old_flags{_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)}
        , m_changed{false}
    { }

    explicit win_memdbg(initial_mode_t init_mode)
        : m_old_flags{_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)}
        , m_changed{false}
    {
        if (init_mode != initmode_keep)
        {
            if (init_mode == initmode_disable)
            {
                const int new_flags = m_old_flags & ~(_CRTDBG_ALLOC_MEM_DF);
                if (new_flags != m_old_flags)
                {
                    _CrtSetDbgFlag(new_flags);
                    m_changed = true;
                }
            }
            else if (init_mode == initmode_enable)
            {
                const int new_flags = m_old_flags | _CRTDBG_ALLOC_MEM_DF;
                if (new_flags != m_old_flags)
                {
                    _CrtSetDbgFlag(new_flags);
                    m_changed = true;
                }
            }
            else
            {
                assert(0);
            }
        }
    }

    ~win_memdbg()
    {
        if (m_changed)
            _CrtSetDbgFlag(m_old_flags);
    }

    int old_flags() const { return m_old_flags; }

    bool changed() const { return m_changed; }

    void enable()
    {
        const int current_flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
        const int new_flags = current_flags | _CRTDBG_ALLOC_MEM_DF;
        if (new_flags != current_flags)
        {
            _CrtSetDbgFlag(new_flags);
            m_changed = true;
        }
    }

    void disable()
    {
        const int current_flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
        const int new_flags = current_flags & ~(_CRTDBG_ALLOC_MEM_DF);
        if (new_flags != current_flags)
        {
            _CrtSetDbgFlag(new_flags);
            m_changed = true;
        }
    }

    void add_flags(int extra_flags)
    {
        const int current_flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
        const int new_flags = current_flags | extra_flags;
        if (new_flags != current_flags)
        {
            _CrtSetDbgFlag(new_flags);
            m_changed = true;
        }
    }

    void set_flags(int flags)
    {
        const int current_flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
        if (flags != current_flags)
        {
            _CrtSetDbgFlag(flags);
            m_changed = true;
        }
    }

    void restore()
    {
        if (m_changed)
        {
            _CrtSetDbgFlag(m_old_flags);
            m_changed = false;
        }
    }

private:
    int m_old_flags;
    bool m_changed;
};

}  // namespace cix

#endif
