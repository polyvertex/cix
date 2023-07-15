// CIX C++ library
// Copyright (c) Jean-Charles Lefebvre
// SPDX-License-Identifier: MIT

#pragma once

#include "detail/ensure_cix.h"

#define CIX_WSTR(x)    __CIX_WSTR(x)
#define __CIX_WSTR(x)  L##x


namespace cix {
namespace string {

typedef std::basic_string<signed char>::size_type size_type;

static constexpr auto npos = std::basic_string<signed char>::npos;

using ctype_mask = std::ctype_base::mask;

static constexpr auto ctype_space = std::ctype_base::space;
static constexpr auto ctype_print = std::ctype_base::print;
static constexpr auto ctype_cntrl = std::ctype_base::cntrl;
static constexpr auto ctype_upper = std::ctype_base::upper;
static constexpr auto ctype_lower = std::ctype_base::lower;
static constexpr auto ctype_alpha = std::ctype_base::alpha;
static constexpr auto ctype_digit = std::ctype_base::digit;
static constexpr auto ctype_punct = std::ctype_base::punct;
static constexpr auto ctype_xdigit = std::ctype_base::xdigit;
static constexpr auto ctype_blank = std::ctype_base::blank;
static constexpr auto ctype_alnum = std::ctype_base::alnum;
static constexpr auto ctype_graph = std::ctype_base::graph;


// char_is
template <typename Char> constexpr bool char_is(Char c, ctype_mask masks);

// isspace - shorthand for ``char_is(c, ctype_space)``
template <typename Char> constexpr bool isspace(Char c);



// is_char
template <typename Char> struct is_char : std::false_type { };
template <> struct is_char<signed char> : std::true_type { };
template <> struct is_char<wchar_t> : std::true_type { };
template <> struct is_char<char8_t> : std::true_type { };
template <> struct is_char<char16_t> : std::true_type { };
template <> struct is_char<char32_t> : std::true_type { };

// is_char_v
template <typename Char>
inline constexpr bool is_char_v = is_char<Char>::value;



// strnlen_s

template <typename Char>
constexpr size_type strnlen_s(const Char* s, size_type max_length);

// template <>
// inline constexpr size_type strnlen_s<signed char>(const signed char* s, size_type max_length)
// { return ::strnlen_s(s, max_length); }  // SIMD-optimized (MSVC at least)

// template <>
// inline constexpr size_type strnlen_s<wchar_t>(const wchar_t* s, size_type max_length)
// { return ::wcsnlen_s(s, max_length); }  // SIMD-optimized (MSVC at least)

template <typename Char, size_type Count>
inline constexpr size_type strnlen_s(const std::array<Char, Count>& s)
{ return strnlen_s(s.data(), s.size()); }

template <typename Char>
inline constexpr size_type strnlen_s(const std::basic_string_view<Char>& s)
{ return strnlen_s(s.data(), s.size()); }

template <typename Char>
inline constexpr size_type strnlen_s(const std::vector<Char>& s)
{ return strnlen_s(s.data(), s.size()); }


// to_string_view

template <typename Char>
inline constexpr std::enable_if_t<
    is_char_v<Char>,
    std::basic_string_view<Char>>
to_string_view(const Char* s)
{ return s; }

template <typename Char>
inline constexpr std::enable_if_t<
    is_char_v<Char>,
    std::basic_string_view<Char>>
to_string_view(const Char* s, size_type length)
{ return { s, length }; }

template <typename Char>
inline constexpr std::basic_string_view<Char>
to_string_view(std::basic_string_view<Char> s)
{ return s; }

template <typename Char, size_type Count>
inline constexpr std::basic_string_view<Char>
to_string_view(const std::array<Char, Count>& s)
{ return { s.data(), strnlen_s(s) }; }

template <typename Char, typename Traits, typename Alloc>
inline constexpr std::basic_string_view<Char>
to_string_view(const std::basic_string<Char, Traits, Alloc>& s)
{ return { s.data(), s.size() }; }

template <
    typename Char,
    typename Alloc>
inline constexpr std::enable_if_t<
    is_char_v<Char>,
    std::basic_string_view<Char>>
to_string_view(const std::vector<Char, Alloc>& s)
{ return { s.data(), strnlen_s(s) }; }

#ifdef CPPWINRT_VERSION  // C++/WinRT enabled
inline std::basic_string_view<winrt::hstring::value_type>
to_string_view(const winrt::hstring& s)
{ return { s.data(), s.size() }; }
#endif



// is_string_viewable

template <typename String, typename = void>
struct is_string_viewable : std::false_type { };

template <typename String>
struct is_string_viewable<
    String,
    std::enable_if_t<
        std::is_class_v<decltype(to_string_view(std::declval<String>()))>>>
    : std::true_type
{
    using _result = decltype(to_string_view(std::declval<String>()));
    using char_type = typename _result::value_type;
};

template <typename String>
inline constexpr bool is_string_viewable_v =
    is_string_viewable<String>::value;



// char_t

namespace detail
{
    template <typename String, typename = void>
    struct char_t_impl { };

    template <typename String>
    struct char_t_impl<
        String,
        std::enable_if_t<is_string_viewable_v<String>>>
    {
        using _result = decltype(to_string_view(std::declval<String>()));
        using type = typename _result::value_type;
    };
}

template <typename String>
using char_t = typename detail::char_t_impl<String>::type;



// is_container_of_strings

template <typename Container, typename = void>
struct is_container_of_strings : std::false_type { };

template <typename Container>
struct is_container_of_strings<
    Container,
    std::enable_if_t<
        std::is_class_v<std::decay_t<Container>> &&
        std::is_class_v<typename std::decay_t<Container>::value_type> &&
        is_string_viewable_v<typename std::decay_t<Container>::value_type>>>
    : std::true_type
{
    using string_type = typename std::decay_t<Container>::value_type;
    using char_type = char_t<string_type>;
};

template <typename Container>
inline constexpr bool is_container_of_strings_v =
    is_container_of_strings<Container>::value;



// to_string
template <
    typename String,
    typename Char = char_t<String>>
std::enable_if_t<
    is_string_viewable_v<String>,
    std::basic_string<Char>>
to_string(const String& s);



/**
    \rst
    Safely expose the internal buffer of a standard'ish container object like
    ``std::vector`` or ``std::basic_string``, to an external API.

    .. note::
        Each call to :func:`prepare` must be followed by a call to
        :func:`finalize`.
    \endrst
*/
template <typename Container>
typename Container::value_type*
prepare(
    Container& container,
    size_type required_len,
    size_type offset=0);


/**
    \rst
    Counterpart of :func:`prepare`. Clean up the internal buffer of a
    standard'ish container object after

    If *length* equals to ``cix::string::npos``, this function will search for a
    null character in the range ``[0 .. container.size()]`` and eventually
    resize the container if one has been found inside this range.

    Otherwise, if *length* is specified, no check is performed.
    \endrst
*/
template <typename Container>
Container&
finalize(
    Container& container,
    size_type length=npos,
    size_type offset=0);


// widen (utf-8 to wchar_t)
#ifdef _WIN32
template <
    typename String,
    typename Char = char_t<String>>
std::enable_if_t<
    is_string_viewable_v<String> && std::is_same_v<signed char, Char>,
    std::wstring>
u8towrepl(const String& input);

template <
    typename String,
    typename Char = char_t<String>>
std::enable_if_t<
    is_string_viewable_v<String> && std::is_same_v<signed char, Char>,
    std::wstring>
u8tow(const String& input);
#endif


// narrow (wchar_t to utf-8)
// CAUTION: wtou8() behaves like wtou8repl() on pre-Vista platforms
#ifdef _WIN32
template <
    typename String,
    typename Char = char_t<String>>
std::enable_if_t<
    is_string_viewable_v<String> && std::is_same_v<wchar_t, Char>,
    std::string>
wtou8repl(const String& input);

template <
    typename String,
    typename Char = char_t<String>>
std::enable_if_t<
    is_string_viewable_v<String> && std::is_same_v<wchar_t, Char>,
    std::string>
wtou8(const String& input);
#endif


// split_any_of
template <
    typename StringA,
    typename StringB,
    typename Char = char_t<StringA>>
constexpr std::enable_if_t<
    is_string_viewable_v<StringA> &&
    is_string_viewable_v<StringB> &&
    std::is_same_v<char_t<StringA>, char_t<StringB>>,
    std::vector<std::basic_string_view<Char>>>
split_any_of(
    const StringA& input,
    const StringB& seps,
    size_type max_split=0);

// split_if
template <
    typename String,
    typename UnaryPredicate,
    typename Char = char_t<String>>
constexpr std::enable_if_t<
    is_string_viewable_v<String>,
    std::vector<std::basic_string_view<Char>>>
split_if(
    const String& input,
    UnaryPredicate pred,
    size_type max_split=0);

// split (split on whitespaces)
template <
    typename String,
    typename Char = char_t<String>>
std::enable_if_t<
    is_string_viewable_v<String>,
    std::vector<std::basic_string_view<Char>>>
split(
    const String& input,
    size_type max_split=0);



// join (variadic)
template <
    typename StringA,
    typename StringB,
    typename... Args,
    typename Char = char_t<StringA>>
std::enable_if_t<
        is_string_viewable_v<StringA> &&
        is_string_viewable_v<StringB> &&
        std::is_same_v<char_t<StringA>, char_t<StringB>>,
    std::basic_string<Char>>
join(
    const StringA& glue,
    const StringB& head,
    Args&&... args) noexcept;

// join (from a container of strings)
template <
    typename String,
    typename Container,
    typename CharA = char_t<String>,
    typename CharB = is_container_of_strings<Container>::char_type>
std::enable_if_t<
        is_string_viewable_v<String> &&
        is_container_of_strings_v<Container> &&
        std::is_same_v<CharA, CharB>,
    std::basic_string<CharA>>
join(
    const String& glue,
    const Container& elements) noexcept;



// melt (variadic)
// like `cix::string::join` except that empty elements are not joined
template <
    typename StringA,
    typename StringB,
    typename... Args,
    typename Char = char_t<StringA>>
std::enable_if_t<
        is_string_viewable_v<StringA> &&
        is_string_viewable_v<StringB> &&
        std::is_same_v<char_t<StringA>, char_t<StringB>>,
    std::basic_string<Char>>
melt(
    const StringA& glue,
    const StringB& head,
    Args&&... args) noexcept;

// melt (from a container of strings)
template <
    typename String,
    typename Container,
    typename CharA = char_t<String>,
    typename CharB = is_container_of_strings<Container>::char_type>
std::enable_if_t<
        is_string_viewable_v<String> &&
        is_container_of_strings_v<Container> &&
        std::is_same_v<CharA, CharB>,
    std::basic_string<CharA>>
melt(
    const String& glue,
    const Container& elements) noexcept;



// melt_trimmed
//
// Like `cix::string::melt` except that glue is stripped from both ends of an
// element before it gets appended.
//
// Typically useful to join parts of a path. See `cix::path::join` and
// `cix::path::join_with`.
template <
    typename StringA,
    typename StringB,
    typename... Args,
    typename Char = char_t<StringA>>
std::enable_if_t<
        is_string_viewable_v<StringA> &&
        is_string_viewable_v<StringB> &&
        std::is_same_v<char_t<StringA>, char_t<StringB>>,
    std::basic_string<Char>>
melt_trimmed(
    const StringA& glue,
    const StringB& head,
    Args&&... args) noexcept;

// melt_trimmed (from a container of strings)
template <
    typename String,
    typename Container,
    typename CharA = char_t<String>,
    typename CharB = is_container_of_strings<Container>::char_type>
std::enable_if_t<
        is_string_viewable_v<String> &&
        is_container_of_strings_v<Container> &&
        std::is_same_v<CharA, CharB>,
    std::basic_string<CharA>>
melt_trimmed(
    const String& glue,
    const Container& elements) noexcept;



// trim_if

template <
    typename String,
    typename UnaryPredicate,
    typename Char = char_t<String>>
constexpr std::enable_if_t<
    is_string_viewable_v<String>,
    std::basic_string_view<Char>>
ltrim_if(
    const String& input,
    UnaryPredicate to_remove);

template <
    typename String,
    typename UnaryPredicate,
    typename Char = char_t<String>>
constexpr std::enable_if_t<
    is_string_viewable_v<String>,
    std::basic_string_view<Char>>
rtrim_if(
    const String& input,
    UnaryPredicate to_remove);

template <
    typename String,
    typename UnaryPredicate,
    typename Char = char_t<String>>
constexpr std::enable_if_t<
    is_string_viewable_v<String>,
    std::basic_string_view<Char>>
trim_if(
    const String& input,
    UnaryPredicate to_remove);



// trim (whitespaces)

template <
    typename String,
    typename Char = char_t<String>>
inline constexpr std::enable_if_t<
    is_string_viewable_v<String>,
    std::basic_string_view<Char>>
ltrim(const String& input)
    { return ltrim_if(input, isspace<Char>); }

template <
    typename String,
    typename Char = char_t<String>>
inline constexpr std::enable_if_t<
    is_string_viewable_v<String>,
    std::basic_string_view<Char>>
rtrim(const String& input)
    { return rtrim_if(input, isspace<Char>); }

template <
    typename String,
    typename Char = char_t<String>>
inline constexpr std::enable_if_t<
    is_string_viewable_v<String>,
    std::basic_string_view<Char>>
trim(const String& input)
    { return trim_if(input, isspace<Char>); }



// replace_all
template <
    typename StringA,
    typename StringB,
    typename StringC,
    typename Char = char_t<StringA>>
std::enable_if_t<
        is_string_viewable_v<StringA> &&
        is_string_viewable_v<StringB> &&
        is_string_viewable_v<StringC> &&
        std::is_same_v<char_t<StringA>, char_t<StringB>> &&
        std::is_same_v<char_t<StringA>, char_t<StringC>>,
    std::basic_string<Char>>
replace_all(
    const StringA& input,
    const StringB& from,
    const StringC& to);


// replace_all_of
template <
    typename StringA,
    typename StringB,
    typename StringC,
    typename Char = char_t<StringA>>
std::enable_if_t<
        is_string_viewable_v<StringA> &&
        is_string_viewable_v<StringB> &&
        is_string_viewable_v<StringC> &&
        std::is_same_v<char_t<StringA>, char_t<StringB>> &&
        std::is_same_v<char_t<StringA>, char_t<StringC>>,
    std::basic_string<Char>>
replace_all_of(
    const StringA& input,
    const StringB& from_any,
    const StringC& to);



// fmt
template <
    typename String,
    typename... Args,
    typename Char = ::fmt::char_t<String>,
    typename Container = std::basic_string<Char>>
Container
fmt(
    const String& format,
    Args&&... args);

// fmt_size
// returns the number of characters in the output of fmt(format, args...)
template <
    typename String,
    typename... Args,
    typename Char = ::fmt::char_t<String>,
    typename Container = std::basic_string<Char>>
Container
fmt_size(
    const String& format,
    Args&&... args);

// fmt_to (OutContainer)
// append to dest
template <
    typename Container,
    typename String,
    typename... Args,
    typename Char = ::fmt::char_t<String>>
std::back_insert_iterator<Container>
fmt_to(
    Container& dest,
    const String& format,
    Args&&... args);

// fmt_to (OutIterator)
template <
    typename String,
    typename... Args,
    typename Char = ::fmt::char_t<String>,
    typename OutputIt = std::back_insert_iterator<Char>>
OutputIt
fmt_to(
    OutputIt out,
    const String& format,
    Args&&... args);

// fmt_to_n (OutContainer)
// append to dest, up to N characters
template <
    typename Container,
    typename String,
    typename... Args,
    typename Char = ::fmt::char_t<String>>
std::back_insert_iterator<Container>
fmt_to_n(
    Container& dest,
    std::size_t n,
    const String& format,
    Args&&... args);

// fmt_to_n (OutIterator)
template <
    typename Container,
    typename String,
    typename... Args,
    typename Char = ::fmt::char_t<String>,
    typename OutputIt = std::back_insert_iterator<Container>>
OutputIt
fmt_to_n(
    OutputIt out,
    std::size_t n,
    const String& format,
    Args&&... args);

// vfmt_to
template <
    typename String,
    typename Char = ::fmt::char_t<String>,
    typename OutputIt = std::back_insert_iterator<std::basic_string<Char>>,
    typename FmtArgs = ::fmt::basic_format_args<
        ::fmt::buffer_context<::fmt::type_identity_t<Char>>>>
OutputIt
vfmt_to(
    OutputIt out,
    const String& format,
    FmtArgs args);

// vfmt_to_n
template <
    typename String,
    typename Char = ::fmt::char_t<String>,
    typename OutputIt = std::back_insert_iterator<std::basic_string<Char>>,
    typename FmtArgs = ::fmt::basic_format_args<::fmt::buffer_context<Char>>>
OutputIt
vfmt_to_n(
    OutputIt out,
    std::size_t n,
    const String& format,
    FmtArgs args);



// to_lower(Char)
template <typename Char>
std::enable_if_t<is_char_v<Char>, Char>
to_lower(Char c);

// to_lower(String)
template <
    typename String,
    typename Char = char_t<String>>
std::enable_if_t<
    is_string_viewable_v<String>,
    std::basic_string<Char>>
to_lower(const String& input);


// to_upper(Char)
template <typename Char>
std::enable_if_t<is_char_v<Char>, Char>
to_upper(Char c);

// to_upper(String)
template <
    typename String,
    typename Char = char_t<String>>
std::enable_if_t<
    is_string_viewable_v<String>,
    std::basic_string<Char>>
to_upper(const String& input);


}  // namespace string
}  // namespace cix


#include "string.inl.h"
