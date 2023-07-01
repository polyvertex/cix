// CIX C++ library
// Copyright (c) Jean-Charles Lefebvre
// SPDX-License-Identifier: MIT

#include <cix/cix>
#include <cix/detail/intro.h>

#if CIX_PLATFORM_WINDOWS

namespace cix {

std::recursive_mutex win_dll::ms_mutex;
std::map<std::wstring, HMODULE> win_dll::ms_handles;


FARPROC win_dll::get_proc(
    const wchar_t* file, const char* procName, LoadFlags flags)
{
    std::scoped_lock lock(ms_mutex);
    HMODULE hmod = win_dll::get_module_handle(file, flags);
    return hmod ? GetProcAddress(hmod, procName) : nullptr;
}


HMODULE win_dll::get_module_handle(const wchar_t* file, LoadFlags flags)
{
    std::scoped_lock lock(ms_mutex);

    const auto clean_mod_name{win_dll::normalize_module_name(file)};
    HMODULE hmod = win_dll::find_module_handle(clean_mod_name);

    if (!hmod)
    {
        if (flags & LOAD_AS_DATAFILE)  // SAFE implied (to simplify code)
            hmod = win_dll::load_library_as_datafile(clean_mod_name.c_str());
        else if (flags & LOAD_SAFE)
            hmod = win_dll::safe_load_library(clean_mod_name.c_str());
        else
            hmod = LoadLibrary(clean_mod_name.c_str());

        if (!hmod)
            return nullptr;  // GetLastError() is preserved

        ms_handles.insert(std::make_pair(clean_mod_name, hmod));
    }

    return hmod;
}


void win_dll::free_all_modules()
{
    std::scoped_lock lock(ms_mutex);

    for (const auto& it : ms_handles)
        FreeLibrary(it.second);

    ms_handles.clear();
}


#if 0
bool win_dll::free_module(const wchar_t* file)
{
    std::scoped_lock lock(ms_mutex);

    const auto clean_mod_name{win_dll::normalize_module_name(file)};
    auto it = ms_handles.find(clean_mod_name);

    if (it != ms_handles.end())
    {
        FreeLibrary(it->second);
        const auto error{GetLastError()};

        ms_handles.erase(it);

        SetLastError(error);
        return true;
    }

    return false;
}
#endif


#if 0
bool win_dll::free_module(HMODULE hmod)
{
    std::scoped_lock lock(ms_mutex);

    for (auto it = ms_handles.begin(); it != ms_handles.end(); it++)
    {
        if (it->second == hmod)
        {
            FreeLibrary(it->second);
            const auto error{GetLastError()};

            ms_handles.erase(it);

            SetLastError(error);
            return true;
        }
    }

    return false;
}
#endif


HMODULE win_dll::safe_load_library(const wchar_t* file, DWORD flags) noexcept
{
    cix::win_scoped_error_mode scopedErrorMode(cix::win_scoped_error_mode::QUIET);
    return LoadLibraryEx(file, nullptr, flags);
}


HMODULE win_dll::load_library_as_datafile(const wchar_t* file) noexcept
{
    return win_dll::safe_load_library(
        file,
        LOAD_LIBRARY_AS_IMAGE_RESOURCE | LOAD_LIBRARY_AS_DATAFILE |
        LOAD_IGNORE_CODE_AUTHZ_LEVEL);
}


HMODULE win_dll::find_module_handle(const std::wstring& clean_mod_name)
{
    const auto it = ms_handles.find(clean_mod_name);
    return it != ms_handles.end() ? it->second : nullptr;
}


std::wstring win_dll::normalize_module_name(const wchar_t* name)
{
    auto result = string::to_lower(string::trim(name));

    if (result.find_first_of(CIX_WSTR("/"), 0, 1) != result.npos)
        result = string::replace_all(result, CIX_WSTR("/"), CIX_WSTR("\\"));

    while (result.find(CIX_WSTR("\\\\"), 0, 2) != result.npos)
        result = string::replace_all(result, CIX_WSTR("\\\\"), CIX_WSTR("\\"));

    return result;
}

}  // namespace cix

#endif  // #if CIX_PLATFORM_WINDOWS
