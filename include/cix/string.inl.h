// CIX C++ library
// Copyright (c) Jean-Charles Lefebvre
// SPDX-License-Identifier: MIT

namespace cix {
namespace string {

namespace detail
{
    template <typename Char>
    const auto& cfacet = std::use_facet<std::ctype<Char>>(std::locale::classic());

#ifdef _WIN32
    inline std::wstring widen(const char* src, size_type len, bool strict)
    {
        if (!src || !*src || !len)
            return {};

        const UINT src_cp = CP_UTF8;
        const DWORD flags = strict ? MB_ERR_INVALID_CHARS : 0;
        int ilen;

        // deal with unlikely overflow - MultiByteToWideChar expects an int
        if (len <= static_cast<size_type>(std::numeric_limits<int>::max()))
        {
            ilen = static_cast<int>(len);
        }
        else
        {
            assert(0);
            ilen = static_cast<size_type>(std::numeric_limits<int>::max());
        }

        // get required size
        int count = MultiByteToWideChar(src_cp, flags, src, ilen, nullptr, 0);
        if (count <= 0)
        {
            assert(0);
            return {};
        }

        // convert
        std::wstring dest(static_cast<size_type>(count), 0);
        MultiByteToWideChar(src_cp, flags, src, ilen, dest.data(), count);
        while (dest.back() == 0)
            dest.erase(dest.size() - 1);

        return dest;
    }
#endif  // #ifdef _WIN32


#ifdef _WIN32
    inline std::string narrow(const wchar_t* src, size_type len, bool strict)
    {
        if (!src || !*src || !len)
            return {};

        const UINT dest_cp = CP_UTF8;
        DWORD flags = 0;
        int ilen;

        // WC_ERR_INVALID_CHARS flag is supported on Vista or later
#if (WINVER >= 0x0600) || defined(WC_ERR_INVALID_CHARS)
        if (strict)
            flags |= WC_NO_BEST_FIT_CHARS | WC_ERR_INVALID_CHARS;
#else
        if (strict)
            flags |= WC_NO_BEST_FIT_CHARS;
#endif

        // deal with unlikely overflow - WideCharToMultiByte expects an int
        if (len <= static_cast<size_type>(std::numeric_limits<int>::max()))
        {
            ilen = static_cast<int>(len);
        }
        else
        {
            assert(0);
            ilen = static_cast<size_type>(std::numeric_limits<int>::max());
        }

        // get required size
        int count = WideCharToMultiByte(
            dest_cp, flags, src, ilen, nullptr, 0, nullptr, nullptr);
        if (count <= 0)
        {
            assert(0);
            return {};
        }

        // convert
        std::string dest(static_cast<size_type>(count), 0);
        WideCharToMultiByte(
            dest_cp, flags, src, ilen, dest.data(), count, nullptr, nullptr);
        while (dest.back() == 0)
            dest.erase(dest.size() - 1);

        return dest;
    }
#endif  // #ifdef _WIN32


    // this fmt::arg_formatter forcefully casts signed integers to unsigned when
    // the 'x' or 'X' type specifier is used
#if 0
    template <typename RangeT>
    class fmt_custom_arg_formatter : public ::fmt::arg_formatter<RangeT>
    {
    public:
        using base = ::fmt::arg_formatter<RangeT>;
        using base::range;
        using base::iterator;
        using base::format_specs;

    private:
        using char_type = typename range::value_type;
        using context_type = ::fmt::basic_format_context<iterator, char_type>;

    public:
        explicit fmt_custom_arg_formatter(
                context_type& ctx,
                ::fmt::basic_format_parse_context<char_type>* parse_ctx=nullptr,
                format_specs* spec=nullptr)
            : base(ctx, parse_ctx, spec)
            { }

        using base::operator();

        inline iterator operator()(std::int8_t value)
            { return this->_handle_signed_int(value); }

        inline iterator operator()(std::int16_t value)
            { return this->_handle_signed_int(value); }

        inline iterator operator()(std::int32_t value)
            { return this->_handle_signed_int(value); }

        inline iterator operator()(std::int64_t value)
            { return this->_handle_signed_int(value); }

    private:
        template <typename T>
        iterator _handle_signed_int(T value)
        {
            if (this->specs() && value < T(0))
            {
                const auto type_lc = this->specs()->type | 0x20;  // lower case
                if (type_lc == 'x')  // || type_lc == 'b')
                {
                    typedef std::make_unsigned<T>::type unsigned_type;
                    return base::operator()<unsigned_type>(value);
                }
            }

            return base::operator()(value);
        }
    };
#endif


    template <typename Char>
    inline std::basic_string<Char>
    join_stub_final(
        std::size_t final_length,
        const std::vector<std::basic_string_view<Char>>& glued_views) noexcept
    {
        std::basic_string<Char> out;

        out.reserve(final_length);

        auto outit = std::back_inserter(out);
        for (const auto& view : glued_views)
            std::copy(view.begin(), view.end(), outit);

        return out;
    }

    template <typename Char>
    inline void
    join_stub(
        std::size_t& out_length,
        std::vector<std::basic_string_view<Char>>& out_views,
        bool keep_empty,
        bool trim_glue,
        const std::basic_string_view<Char>& glue) noexcept
    {
        CIX_UNUSED(out_length);
        CIX_UNUSED(out_views);
        CIX_UNUSED(keep_empty);
        CIX_UNUSED(trim_glue);
        CIX_UNUSED(glue);
    }

    template <
        typename String,
        typename... Args,
        typename Char = char_t<String>,
        typename std::enable_if_t<is_string_viewable_v<String>, int> = 0>
    inline void
    join_stub(
        std::size_t& out_length,
        std::vector<std::basic_string_view<Char>>& out_views,
        bool keep_empty,
        bool trim_glue,
        const std::basic_string_view<Char>& glue,
        const String& head_,
        Args&&... args) noexcept
    {
        auto head = to_string_view(head_);

        if (trim_glue && !glue.empty())
        {
#if 1  // C++20
            while (head.starts_with(glue))
                head.remove_prefix(glue.size());

            while (head.ends_with(glue))
                head.remove_suffix(glue.size());
#else
            const auto glue_size = glue.size();

            // strip prefix
            while (
                head.size() >= glue_size &&
                std::equal(
                    head.begin(),
                    std::next(head.begin(), glue_size),
                    glue.begin()))
            {
                head.remove_prefix(glue_size);
            }

            // strip suffix
            while (
                head.size() >= glue_size &&
                std::equal(
                    // this instead of (end - glue_size), so that we stick to
                    // ForwardIt
                    std::next(head.begin(), head.size() - glue_size),
                    head.end(),
                    glue.begin()))
            {
                head.remove_suffix(glue_size);
            }
#endif
        }

        if (keep_empty || !head.empty())
        {
            if (!out_views.empty())
            {
                out_views.push_back(glue);
                out_length += glue.size();
            }

            out_views.push_back(head);
            out_length += head.size();
        }

        join_stub(out_length, out_views, keep_empty, trim_glue, glue, args...);
    }

}  // namespace detail


template <typename Char>
inline constexpr bool char_is(Char c, ctype_mask masks)
{
    return detail::cfacet<Char>.is(masks, c);
}


template <typename Char>
inline constexpr bool isspace(Char c)
{
    return char_is(c, ctype_space);
}


template <typename Char>
inline constexpr size_type strnlen_s(const Char* s, size_type max_length)
{
    if (max_length <= 0 || !s)
    {
        return 0;
    }
    else
    {
        const std::basic_string_view<Char> view(s, max_length);
        const auto pos = view.find(Char(0));
        return (pos != view.npos) ? pos : max_length;
    }
}


template <
    typename String,
    typename Char>
inline std::enable_if_t<
    is_string_viewable_v<String>,
    std::basic_string<Char>>
to_string(const String& s)
{
    const auto view = to_string_view(s);
    return std::basic_string<Char>(view.begin(), view.end());
}


template <typename Container>
inline typename Container::value_type*
prepare(
    Container& container,
    size_type required_len,
    size_type offset)
{
    container.resize(offset + required_len);
    return container.data();  // return &container[offset];
}


template <typename Container>
inline Container&
finalize(
    Container& container,
    size_type length,
    size_type offset)
{
    assert(length == npos || (offset + length) <= container.size());

    if (length == npos)
    {
        typename Container::value_type* p = &container[0];

        p += offset;
        length = offset;
        for (; length < container.size() && *p++; ++length) { ; }

        assert(
            (length < container.size() && !container[length]) ||
            length == container.size());
    }
    else
    {
        length += offset;
    }
    assert(length <= container.size());

    if (length < container.size())
    {
        container.erase(
            std::next(container.begin(), length),
            container.end());
    }

    return container;
}


#ifdef _WIN32
template <
    typename String,
    typename Char>
inline std::enable_if_t<
    is_string_viewable_v<String> && std::is_same_v<char, Char>,
    std::wstring>
u8towrepl(const String& input)
{
    const auto view = to_string_view(input);
    return detail::widen(view.data(), view.length(), false);
}
#endif


#ifdef _WIN32
template <
    typename String,
    typename Char>
inline std::enable_if_t<
    is_string_viewable_v<String> && std::is_same_v<char, Char>,
    std::wstring>
u8tow(const String& input)
{
    const auto view = to_string_view(input);
    return detail::widen(view.data(), view.length(), true);
}
#endif


#ifdef _WIN32
template <
    typename String,
    typename Char>
inline std::enable_if_t<
    is_string_viewable_v<String> && std::is_same_v<wchar_t, Char>,
    std::string>
wtou8repl(const String& input)
{
    const auto view = to_string_view(input);
    return detail::narrow(view.data(), view.length(), false);
}
#endif


#ifdef _WIN32
template <
    typename String,
    typename Char>
inline std::enable_if_t<
    is_string_viewable_v<String> && std::is_same_v<wchar_t, Char>,
    std::string>
wtou8(const String& input)
{
    const auto view = to_string_view(input);
    return detail::narrow(view.data(), view.length(), true);
}
#endif


template <
    typename StringA,
    typename StringB,
    typename Char>
inline constexpr std::enable_if_t<
    is_string_viewable_v<StringA> &&
    is_string_viewable_v<StringB> &&
    std::is_same_v<char_t<StringA>, char_t<StringB>>,
    std::vector<std::basic_string_view<Char>>>
split_any_of(
        const StringA& input,
        const StringB& seps,
        size_type max_split)
{
    const auto inputv = to_string_view(input);
    const auto sepsv = to_string_view(seps);
    std::vector<std::basic_string_view<Char>> out;
    auto base = inputv.begin();
    decltype(base) it;

    while (base != inputv.end())
    {
        if (max_split > 0 && out.size() >= max_split)
        {
            it = inputv.end();
        }
        else
        {
            it = std::find_first_of(
                base, inputv.end(),
                sepsv.begin(), sepsv.end());
        }

        out.push_back(decltype(out)::value_type(&*base, it - base));

        if (it == inputv.end())
            break;

        base = std::next(it);
    }

    return out;
}


template <
    typename String,
    typename UnaryPredicate,
    typename Char>
inline constexpr std::enable_if_t<
    is_string_viewable_v<String>,
    std::vector<std::basic_string_view<Char>>>
split_if(
    const String& input,
    UnaryPredicate pred,
    size_type max_split)
{
    const auto inputv = to_string_view(input);
    std::vector<std::basic_string_view<Char>> out;
    auto base = inputv.begin();
    decltype(base) it;

    for (;;)
    {
        if (max_split > 0 && out.size() >= max_split)
            it = inputv.end();
        else
            it = std::find_if(base, inputv.end(), pred);

        out.push_back(decltype(out)::value_type(&*base, it - base));

        if (it == inputv.end())
            break;

        base = std::next(it);
    }

    return out;
}


template <
    typename String,
    typename Char>
inline std::enable_if_t<
    is_string_viewable_v<String>,
    std::vector<std::basic_string_view<Char>>>
split(
    const String& input,
    size_type max_split)
{
    auto pred = [](Char c) { return isspace(c); };
    return split_if(input, pred, max_split);
}


template <
    typename StringA,
    typename StringB,
    typename... Args,
    typename Char>
inline std::enable_if_t<
        is_string_viewable_v<StringA> &&
        is_string_viewable_v<StringB> &&
        std::is_same_v<char_t<StringA>, char_t<StringB>>,
    std::basic_string<Char>>
join(
    const StringA& glue_,
    const StringB& head,
    Args&&... args) noexcept
{
    std::vector<std::basic_string_view<Char>> glued_views;
    std::size_t final_strlen = 0;
    const auto glue = to_string_view(glue_);

    glued_views.reserve(2 * (1 + sizeof...(args)));  // +1 for glue and head

    detail::join_stub(
        final_strlen, glued_views, true, false, glue, head, args...);

    return detail::join_stub_final(final_strlen, glued_views);
}


template <
    typename String,
    typename Container,
    typename CharA,
    typename CharB>
inline std::enable_if_t<
        is_string_viewable_v<String> &&
        is_container_of_strings_v<Container> &&
        std::is_same_v<CharA, CharB>,
    std::basic_string<CharA>>
join(
    const String& glue_,
    const Container& elements) noexcept
{
    if (elements.empty())
        return {};

    std::vector<std::basic_string_view<CharA>> glued_views;
    std::size_t final_strlen = 0;
    const auto glue = to_string_view(glue_);

    glued_views.reserve(2 * elements.size());

    for (const auto& elem : elements)
        detail::join_stub(final_strlen, glued_views, true, false, glue, elem);

    return detail::join_stub_final(final_strlen, glued_views);
}


template <
    typename StringA,
    typename StringB,
    typename... Args,
    typename Char>
inline std::enable_if_t<
        is_string_viewable_v<StringA> &&
        is_string_viewable_v<StringB> &&
        std::is_same_v<char_t<StringA>, char_t<StringB>>,
    std::basic_string<Char>>
melt(
    const StringA& glue,
    const StringB& head,
    Args&&... args) noexcept
{
    std::vector<std::basic_string_view<Char>> glued_views;
    std::size_t final_strlen = 0;
    const auto glue_view = to_string_view(glue);

    glued_views.reserve(2 * (1 + sizeof...(args)));  // +1 for glue_view and head

    detail::join_stub(
        final_strlen, glued_views, false, false, glue_view, head, args...);

    return detail::join_stub_final(final_strlen, glued_views);
}


template <
    typename String,
    typename Container,
    typename CharA,
    typename CharB>
inline std::enable_if_t<
        is_string_viewable_v<String> &&
        is_container_of_strings_v<Container> &&
        std::is_same_v<CharA, CharB>,
    std::basic_string<CharA>>
melt(
    const String& glue,
    const Container& elements) noexcept
{
    if (elements.empty())
        return {};

    std::vector<std::basic_string_view<CharA>> glued_views;
    std::size_t final_strlen = 0;
    const auto glue_view = to_string_view(glue);

    glued_views.reserve(2 * elements.size());

    for (const auto& elem : elements)
        detail::join_stub(
            final_strlen, glued_views, false, false, glue_view, elem);

    return detail::join_stub_final(final_strlen, glued_views);
}


template <
    typename StringA,
    typename StringB,
    typename... Args,
    typename Char>
inline std::enable_if_t<
        is_string_viewable_v<StringA> &&
        is_string_viewable_v<StringB> &&
        std::is_same_v<char_t<StringA>, char_t<StringB>>,
    std::basic_string<Char>>
melt_trimmed(
    const StringA& glue,
    const StringB& head,
    Args&&... args) noexcept
{
    std::vector<std::basic_string_view<Char>> glued_views;
    std::size_t final_strlen = 0;
    const auto glue_view = to_string_view(glue);

    glued_views.reserve(2 * (1 + sizeof...(args)));  // +1 for glue_view and head

    detail::join_stub(
        final_strlen, glued_views, false, true, glue_view, head, args...);

    return detail::join_stub_final(final_strlen, glued_views);
}


template <
    typename String,
    typename Container,
    typename CharA,
    typename CharB>
inline std::enable_if_t<
        is_string_viewable_v<String> &&
        is_container_of_strings_v<Container> &&
        std::is_same_v<CharA, CharB>,
    std::basic_string<CharA>>
melt_trimmed(
    const String& glue,
    const Container& elements) noexcept
{
    if (elements.empty())
        return {};

    std::vector<std::basic_string_view<CharA>> glued_views;
    std::size_t final_strlen = 0;
    const auto glue_view = to_string_view(glue);

    glued_views.reserve(2 * elements.size());

    for (const auto& elem : elements)
        detail::join_stub(
            final_strlen, glued_views, false, true, glue_view, elem);

    return detail::join_stub_final(final_strlen, glued_views);
}


template <
    typename String,
    typename UnaryPredicate,
    typename Char>
inline constexpr std::enable_if_t<
    is_string_viewable_v<String>,
    std::basic_string_view<Char>>
ltrim_if(
    const String& input,
    UnaryPredicate to_remove)
{
    const auto inputv = to_string_view(input);
    const auto it = std::find_if_not(inputv.begin(), inputv.end(), to_remove);

    if (it == inputv.end())
        return {};

    return inputv.substr(std::distance(inputv.begin(), it));
}


template <
    typename String,
    typename UnaryPredicate,
    typename Char>
inline constexpr std::enable_if_t<
    is_string_viewable_v<String>,
    std::basic_string_view<Char>>
rtrim_if(
    const String& input,
    UnaryPredicate to_remove)
{
    const auto inputv = to_string_view(input);
    const auto rit =
        std::find_if_not(inputv.rbegin(), inputv.rend(), to_remove);

    if (rit == inputv.rend())
        return {};

    return inputv.substr(0, std::distance(inputv.begin(), rit.base()));
}


template <
    typename String,
    typename UnaryPredicate,
    typename Char>
inline constexpr std::enable_if_t<
    is_string_viewable_v<String>,
    std::basic_string_view<Char>>
trim_if(
    const String& input,
    UnaryPredicate to_remove)
{
    const auto tail = ltrim_if(input, to_remove);
    return rtrim_if(tail, to_remove);
}


template <
    typename StringA,
    typename StringB,
    typename StringC,
    typename Char>
inline std::enable_if_t<
        is_string_viewable_v<StringA> &&
        is_string_viewable_v<StringB> &&
        is_string_viewable_v<StringC> &&
        std::is_same_v<char_t<StringA>, char_t<StringB>> &&
        std::is_same_v<char_t<StringA>, char_t<StringC>>,
    std::basic_string<Char>>
replace_all(
    const StringA& input_,
    const StringB& from_,
    const StringC& to_)
{
    const auto input = to_string_view(input_);
    const auto from = to_string_view(from_);
    const auto to = to_string_view(to_);

    std::basic_string<Char> out;

    if (input.empty())
        return out;

    out.reserve(input.size());

    typename decltype(input)::size_type start = 0;
    auto out_it = std::back_inserter(out);

    do
    {
        const auto pos = input.find(from.data(), start, from.size());

        if (pos != start)
        {
            const auto pos_it =
                (pos == input.npos) ? input.end() :
                std::next(input.begin(), pos);

            std::copy(std::next(input.begin(), start), pos_it, out_it);

            if (pos == input.npos)
                break;
        }

        std::copy(to.begin(), to.end(), out_it);

        start = pos + from.size();
    }
    while (start < input.size());

    return out;
}


template <
    typename StringA,
    typename StringB,
    typename StringC,
    typename Char>
inline std::enable_if_t<
        is_string_viewable_v<StringA> &&
        is_string_viewable_v<StringB> &&
        is_string_viewable_v<StringC> &&
        std::is_same_v<char_t<StringA>, char_t<StringB>> &&
        std::is_same_v<char_t<StringA>, char_t<StringC>>,
    std::basic_string<Char>>
replace_all_of(
    const StringA& input_,
    const StringB& from_any_,
    const StringC& to_)
{
    const auto input = to_string_view(input_);
    const auto from_any = to_string_view(from_any_);
    const auto to = to_string_view(to_);

    std::basic_string<Char> out;

    if (input.empty())
        return out;

    out.reserve(input.size());

    typename decltype(input)::size_type start = 0;
    auto out_it = std::back_inserter(out);

    do
    {
        const auto pos = input.find_first_of(from_any, start);

        if (pos != start)
        {
            const auto pos_it =
                (pos == input.npos) ? input.end() :
                std::next(input.begin(), pos);

            std::copy(std::next(input.begin(), start), pos_it, out_it);

            if (pos == input.npos)
                break;
        }

        std::copy(to.begin(), to.end(), out_it);

        start = pos + 1;
    }
    while (start < input.size());

    return out;
}


template <
    typename String,
    typename... Args,
    typename Char,
    typename Container>
inline Container
fmt(const String& format, Args&&... args)
{
    typedef std::back_insert_iterator<Container> OutputIt;

    Container dest;

    vfmt_to<String, Char, OutputIt>(
        std::back_inserter(dest),
        format,
        ::fmt::make_format_args<
            ::fmt::buffer_context<::fmt::type_identity_t<Char>>>(args...));

    return dest;
}


template <
    typename String,
    typename... Args,
    typename Char,
    typename Container>
inline Container
fmt_size(const String& format, Args&&... args)
{
    return ::fmt::formatted_size(format, std::forward<Args>(args)...);
}


template <
    typename Container,
    typename String,
    typename... Args,
    typename Char>
inline std::back_insert_iterator<Container>
fmt_to(Container& dest, const String& format, Args&&... args)
{
    typedef std::back_insert_iterator<Container> OutputIt;

    return vfmt_to<String, Char, OutputIt>(
        std::back_inserter(dest),
        format,
        ::fmt::make_format_args<
            ::fmt::buffer_context<::fmt::type_identity_t<Char>>>(args...));
}


template <
    typename String,
    typename... Args,
    typename Char,
    typename OutputIt>
inline OutputIt
fmt_to(
    OutputIt out,
    const String& format,
    Args&&... args)
{
    return vfmt_to<String, Char, OutputIt>(
        out,
        format,
        ::fmt::make_format_args<
            ::fmt::buffer_context<::fmt::type_identity_t<Char>>>(args...));
}


template <
    typename Container,
    typename String,
    typename... Args,
    typename Char>
inline std::back_insert_iterator<Container>
fmt_to_n(
    Container& dest, std::size_t n, const String& format, Args&&... args)
{
    typedef std::back_insert_iterator<Container> OutputIt;

    return vfmt_to_n<String, Char, OutputIt>(
        std::back_inserter(dest),
        n,
        format,
        ::fmt::make_format_args<
            ::fmt::buffer_context<::fmt::type_identity_t<Char>>>(args...));
}


template <
    typename Container,
    typename String,
    typename... Args,
    typename Char,
    typename OutputIt>
inline OutputIt
fmt_to_n(
    OutputIt out,
    std::size_t n,
    const String& format,
    Args&&... args)
{
    return vfmt_to_n<String, Char, OutputIt>(
        out,
        n,
        format,
        ::fmt::make_format_args<
            ::fmt::buffer_context<::fmt::type_identity_t<Char>>>(args...));
}


template <
    typename String,
    typename Char,
    typename OutputIt,
    typename FmtArgs>
inline OutputIt
vfmt_to(
    OutputIt out,
    const String& format,
    FmtArgs args)
{
    return ::fmt::vformat_to<OutputIt, String>(out, format, args);
}


template <
    typename String,
    typename Char,
    typename OutputIt,
    typename FmtArgs>
inline OutputIt
vfmt_to_n(
    OutputIt out,
    std::size_t n,
    const String& format,
    FmtArgs args)
{
    return ::fmt::vformat_to_n<OutputIt, Char>(out, n, format, args).out;
}


template <typename Char>
inline std::enable_if_t<is_char_v<Char>, Char>
to_lower(Char c)
{
    return detail::cfacet<Char>.tolower(c);
}


template <typename Char>
inline std::enable_if_t<is_char_v<Char>, Char>
to_upper(Char c)
{
    return detail::cfacet<Char>.toupper(c);
}


template <typename String, typename Char>
inline std::enable_if_t<
    is_string_viewable_v<String>,
    std::basic_string<Char>>
to_lower(const String& input_)
{
    const auto input = to_string_view(input_);
    if (input.empty())
        return {};

    std::basic_string<Char> output(input.begin(), input.end());

    detail::cfacet<Char>.tolower(&output.front(), &output.back() + 1);

    return output;
}


template <typename String, typename Char>
inline std::enable_if_t<
    is_string_viewable_v<String>,
    std::basic_string<Char>>
to_upper(const String& input_)
{
    const auto input = to_string_view(input_);
    if (input.empty())
        return {};

    std::basic_string<Char> output(input.begin(), input.end());

    detail::cfacet<Char>.toupper(&output.front(), &output.back() + 1);

    return output;
}

}  // namespace string
}  // namespace cix
