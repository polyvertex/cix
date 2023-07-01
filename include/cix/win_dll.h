// CIX C++ library
// Copyright (c) Jean-Charles Lefebvre
// SPDX-License-Identifier: MIT

#pragma once

#include "detail/ensure_cix.h"

#if CIX_PLATFORM_WINDOWS

namespace cix {

class win_dll
{
public:
    enum LoadFlags
    {
        LOAD_DEFAULT = 0,
        LOAD_AS_DATAFILE = 0x01,
        LOAD_SAFE = 0x02,
    };

public:
    static FARPROC get_proc(
        const wchar_t* file,
        const char* proc_name,
        LoadFlags flags=LOAD_DEFAULT);

    static HMODULE get_module_handle(
        const wchar_t* file,
        LoadFlags flags=LOAD_DEFAULT);

    static void free_all_modules();
    // static bool free_module(const wchar_t* file);
    // static bool free_module(HMODULE hmod);

private:
    static HMODULE safe_load_library(const wchar_t* file, DWORD flags=0) noexcept;
    static HMODULE load_library_as_datafile(const wchar_t* file) noexcept;

    static HMODULE find_module_handle(const std::wstring& clean_mod_name);

    static std::wstring normalize_module_name(const wchar_t* name);

private:
    static std::recursive_mutex ms_mutex;
    static std::map<std::wstring, HMODULE> ms_handles;
};

CIX_IMPLEMENT_ENUM_BITOPS(win_dll::LoadFlags)

}  // namespace cix

#endif  // #if CIX_PLATFORM_WINDOWS
