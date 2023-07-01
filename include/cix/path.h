// CIX C++ library
// Copyright (c) Jean-Charles Lefebvre
// SPDX-License-Identifier: MIT

#pragma once

#include "detail/ensure_cix.h"

namespace cix {
namespace path {

#ifdef _WIN32
    template <typename Char>
    static constexpr Char native_sep = Char('\\');

    template <typename Char>
    static constexpr Char alt_sep = Char('/');
#else
    template <typename Char>
    static constexpr Char native_sep = Char('/');

    template <typename Char>
    static constexpr Char alt_sep = Char('\\');
#endif


template <typename Char>
static constexpr Char unix_sep = Char('/');

template <typename Char>
static constexpr Char win_sep = Char('\\');


template <typename Char>
static constexpr std::array<Char, 3> all_seps_arr{
    native_sep<Char>, alt_sep<Char>, Char(0) };

template <typename Char>
static constexpr std::array<Char, 1> native_sep_arr{ native_sep<Char> };

template <typename Char>
static constexpr std::array<Char, 1> unix_sep_arr{ unix_sep<Char> };

template <typename Char>
static constexpr std::array<Char, 1> win_sep_arr{ win_sep<Char> };


template <typename Char>
static constexpr auto all_seps_str = string::to_string_view(all_seps_arr<Char>);

template <typename Char>
static constexpr auto native_sep_str = string::to_string_view(native_sep_arr<Char>);

template <typename Char>
static constexpr auto unix_sep_str = string::to_string_view(unix_sep_arr<Char>);

template <typename Char>
static constexpr auto win_sep_str = string::to_string_view(win_sep_arr<Char>);



template <typename Char>
constexpr bool is_sep(Char c) noexcept;

template <typename Char>
constexpr bool is_drive_letter(Char c) noexcept;

template <typename String>
std::enable_if_t<
    string::is_string_viewable_v<String>,
    bool>
is_absolute(const String& path) noexcept;


// template <
//     typename String,
//     typename Char = string::char_t<String>>
// std::enable_if_t<
//     string::is_string_viewable_v<String>,
//     std::basic_string_view<Char>>
// root(const String& path) noexcept;

// template <
//     typename String,
//     typename Char = string::char_t<String>>
// std::enable_if_t<
//     string::is_string_viewable_v<String>,
//     std::basic_string_view<Char>>
// nonroot(const String& path) noexcept;

// POSIX basename()
template <
    typename String,
    typename Char = string::char_t<String>>
std::enable_if_t<
    string::is_string_viewable_v<String>,
    std::basic_string_view<Char>>
basename(const String& path) noexcept;

// POSIX dirname()
template <
    typename String,
    typename Char = string::char_t<String>>
std::enable_if_t<
    string::is_string_viewable_v<String>,
    std::basic_string_view<Char>>
dirname(const String& path) noexcept;

// title = basename with its .ext stripped
template <
    typename String,
    typename Char = string::char_t<String>>
std::enable_if_t<
    string::is_string_viewable_v<String>,
    std::basic_string_view<Char>>
title(const String& path) noexcept;

template <
    typename String,
    typename Char = string::char_t<String>>
std::enable_if_t<
    string::is_string_viewable_v<String>,
    std::basic_string_view<Char>>
trim_ext(const String& path) noexcept;

template <
    typename String,
    typename Char = string::char_t<String>>
std::enable_if_t<
    string::is_string_viewable_v<String>,
    std::basic_string_view<Char>>
trim_all_ext(const String& path) noexcept;

// remove leading separators
template <
    typename String,
    typename Char = string::char_t<String>>
std::enable_if_t<
    string::is_string_viewable_v<String>,
    std::basic_string_view<Char>>
ltrim_sep(const String& path) noexcept;

// remove trailing separators
template <
    typename String,
    typename Char = string::char_t<String>>
std::enable_if_t<
    string::is_string_viewable_v<String>,
    std::basic_string_view<Char>>
rtrim_sep(const String& path) noexcept;


// join (variadic)
template <
    typename String,
    typename... Args,
    typename Char = string::char_t<String>>
std::enable_if_t<
    string::is_string_viewable_v<String>,
    std::basic_string<Char>>
join(
    const String& head,
    Args&&... args) noexcept;

// join (from a container of strings)
template <
    typename Container,
    typename Char = string::is_container_of_strings<Container>::char_type>
std::enable_if_t<
    string::is_container_of_strings_v<Container>,
    std::basic_string<Char>>
join(
    const Container& elements) noexcept;


// join_with (variadic)
template <
    typename String,
    typename... Args,
    typename Char = string::char_t<String>>
std::enable_if_t<
    string::is_string_viewable_v<String>,
    std::basic_string<Char>>
join_with(
    Char sep,
    const String& head,
    Args&&... args) noexcept;

// join_with (from a container of strings)
template <
    typename Container,
    typename Char = string::is_container_of_strings<Container>::char_type>
std::enable_if_t<
    string::is_container_of_strings_v<Container>,
    std::basic_string<Char>>
join_with(
    Char sep,
    const Container& elements) noexcept;

}  // namespace path
}  // namespace cix


#include "path.inl.h"
