// CIX C++ library
// Copyright (c) Jean-Charles Lefebvre
// SPDX-License-Identifier: MIT

namespace cix {
namespace path {

template <typename Char>
inline constexpr bool is_sep(Char c) noexcept
{
    #ifdef _WIN32
        return c == win_sep<Char> || c == unix_sep<Char>;
    #else
        return c == unix_sep<Char> || c == win_sep<Char>;
    #endif
}


template <typename Char>
constexpr bool is_drive_letter(Char c) noexcept
{
    // Not positive string::char_is() strictly matches alpha characters in the
    // ASCII range *only*. We do not want alpha unicode characters, so go for
    // the explicit manual way.

    // return string::char_is(c, string::ctype_alpha);
    return
        (c >= Char('A') && c <= Char('Z')) ||
        (c >= Char('a') && c <= Char('z'));
}


template <typename String>
inline std::enable_if_t<
    string::is_string_viewable_v<String>,
    bool>
is_absolute(const String& path) noexcept
{
    const auto view = string::to_string_view(path);
    const auto len = view.length();

    if (len > 0)
    {
        if (is_sep(view[0]))
            return true;

        if (len >= 2 &&
            view[1] == decltype(view)::value_type(':') &&
            is_drive_letter(view[0]))
        {
            return true;
        }
    }

    return false;
}


#if 0
template <
    typename String,
    typename Char>
inline std::enable_if_t<
    string::is_string_viewable_v<String>,
    std::basic_string_view<Char>>
root(const String& path) noexcept
{
    // supported forms:
    //   <drive_letter>:
    //   <drive_letter>:\
    //   \ (relative to current working directory root)
    //   \\<server>\<sharename>\
    //   \\?\<drive_spec>:\
    //   \\.\<physical_device>\
    //   \\?\<server>\<sharename>\
    //   \\?\UNC\<server>\<sharename>\
    //
    // ref:
    //   https://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx
    //     <- especially the "Namespaces" section
    //   https://msdn.microsoft.com/en-us/library/aa365247.aspx
    //   https://en.wikipedia.org/wiki/Path_(computing)

    auto view = string::to_string_view(path);

    if (view.empty())
        return view;

    if (view.size() >= 2 && view[1] == Char(':') && is_drive_letter(view[0]))
    {
        std::size_t root_len = 2;

        if (view.size() >= 3)
        {
            root_len = view.find_first_not_of(all_sep_str<Char>, 2);
            if (root_len == view.npos)
                root_len = view.size();
        }

        return std::basic_string_view<Char>(view.data(), root_len);
    }

    CIXTODO;  // see wstr::path_root()
}
#endif


// template <
//     typename String,
//     typename Char>
// inline std::enable_if_t<
//     string::is_string_viewable_v<String>,
//     std::basic_string_view<Char>>
// nonroot(const String& path) noexcept
// {
//     CIXTODO;
// }


template <
    typename String,
    typename Char>
inline std::enable_if_t<
    string::is_string_viewable_v<String>,
    std::basic_string_view<Char>>
basename(const String& path) noexcept
{
    // POSIX compliance:
    //   input       dirname  basename
    //   "/usr/lib"  "/usr"   "lib"
    //   "/usr/"     "/"      "usr"
    //   "usr"       "."      "usr"
    //   "/"         "/"      "/"
    //   "."         "."      "."
    //   ".."        ".."     ".."
    //   ""          "."      ""

    // TODO: split root part first in case of an absolute path

    static constexpr auto sep = native_sep_str<Char>;

    auto view = string::to_string_view(path);
    if (view.empty())
        return view;

    // trim trailing separators
    view = string::rtrim_if(view, is_sep<Char>);
    if (view.empty())
        return sep;  // there was only separator(s) in path

    // search for the last separator in the string
    auto rit = std::find_if(view.rbegin(), view.rend(), is_sep<Char>);
    if (rit == view.rend())
        return view;  // no separator

    // strip prefix up to the last found separator
    view.remove_prefix(std::distance(view.begin(), rit.base()));
    return view;
}


template <
    typename String,
    typename Char>
inline std::enable_if_t<
    string::is_string_viewable_v<String>,
    std::basic_string_view<Char>>
dirname(const String& path) noexcept
{
    // POSIX compliance:
    //   input       dirname  basename
    //   "/usr/lib"  "/usr"   "lib"
    //   "/usr/"     "/"      "usr"
    //   "usr"       "."      "usr"
    //   "/"         "/"      "/"
    //   "."         "."      "."
    //   ".."        ".."     ".."
    //   ""          "."      ""

    // TODO: split root part first in case of an absolute path

    static constexpr auto sep = native_sep_str<Char>;
    static constexpr auto dot_arr = std::array<Char, 1>{ Char('.') };
    static constexpr auto dot = string::to_string_view(dot_arr);

    auto view = string::to_string_view(path);
    if (view.empty())
        return dot;

    // trim trailing separators
    view = string::rtrim_if(view, is_sep<Char>);
    if (view.empty())
        return sep;  // there was only separator(s) in path

    // search for the last separator in the string
    auto rit = std::find_if(view.rbegin(), view.rend(), is_sep<Char>);
    if (rit == view.rend())
        return dot;  // no separator

    // strip basename
    view.remove_suffix(std::distance(rit.base(), view.end()));

    // trim trailing separators
    view = string::rtrim_if(view, is_sep<Char>);
    if (view.empty())
        return sep;  // there was only separator(s) remaining

    return view;
}


template <
    typename String,
    typename Char>
inline std::enable_if_t<
    string::is_string_viewable_v<String>,
    std::basic_string_view<Char>>
title(const String& path) noexcept
{
    return trim_ext(basename(path));
}


template <
    typename String,
    typename Char>
inline std::enable_if_t<
    string::is_string_viewable_v<String>,
    std::basic_string_view<Char>>
trim_ext(const String& path) noexcept
{
    const auto name_ = basename(path);
    const auto pos = name_.find_last_of(Char('.'));

    if (pos == name_.npos || pos == 0)
        return string::to_string_view(path);

    auto view = string::to_string_view(path);
    view.remove_suffix(name_.length() - pos);

    return view;
}


template <
    typename String,
    typename Char>
inline std::enable_if_t<
    string::is_string_viewable_v<String>,
    std::basic_string_view<Char>>
trim_all_ext(const String& path) noexcept
{
    auto result = trim_ext(path);

    for (;;)
    {
        auto tmp = trim_ext(result);
        if (tmp.size() == result.size())
            break;

        result.swap(tmp);
    }

    return result;
}


template <
    typename String,
    typename Char>
inline std::enable_if_t<
    string::is_string_viewable_v<String>,
    std::basic_string_view<Char>>
ltrim_sep(const String& path) noexcept
{
    auto view = string::to_string_view(path);
    if (!view.empty())
        view = string::ltrim_if(view, is_sep<Char>);
    return view;
}


template <
    typename String,
    typename Char>
inline std::enable_if_t<
    string::is_string_viewable_v<String>,
    std::basic_string_view<Char>>
rtrim_sep(const String& path) noexcept
{
    auto view = string::to_string_view(path);
    if (!view.empty())
        view = string::rtrim_if(view, is_sep<Char>);
    return view;
}


template <
    typename String,
    typename... Args,
    typename Char>
inline std::enable_if_t<
    string::is_string_viewable_v<String>,
    std::basic_string<Char>>
join(const String& head, Args&&... args) noexcept
{
    return string::melt_trimmed(
        native_sep_str<Char>, head, std::forward<Args>(args)...);
}


template <
    typename Container,
    typename Char>
inline std::enable_if_t<
    string::is_container_of_strings_v<Container>,
    std::basic_string<Char>>
join(const Container& elements) noexcept
{
    return string::melt_trimmed(native_sep_str<Char>, elements);
}


template <
    typename String,
    typename... Args,
    typename Char>
inline std::enable_if_t<
    string::is_string_viewable_v<String>,
    std::basic_string<Char>>
join_with(Char sep, const String& head, Args&&... args) noexcept
{
    const Char sep_str[2] = { sep, Char(0) };
    return string::melt_trimmed(
        sep_str, head, std::forward<Args>(args)...);
}


template <
    typename Container,
    typename Char>
inline std::enable_if_t<
    string::is_container_of_strings_v<Container>,
    std::basic_string<Char>>
join_with(Char sep, const Container& elements) noexcept
{
    const Char sep_str[2] = { sep, Char(0) };
    return string::melt_trimmed(sep_str, elements);
}


}  // namespace path
}  // namespace cix
