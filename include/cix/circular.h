// CIX C++ library
// Copyright (c) Jean-Charles Lefebvre
// SPDX-License-Identifier: MIT

#pragma once

namespace cix {

namespace detail::circular
{
    template <typename Container, typename = void>
    struct is_resizable : std::false_type { };

    template <typename Container>
    struct is_resizable<
        Container,
        std::void_t<
            decltype(std::declval<Container>().resize(std::declval<std::size_t>()))>>
        : std::true_type { };

    template <typename Container>
    inline constexpr bool is_resizable_v = is_resizable<Container>::value;


    template <typename Container, typename = void>
    struct is_shrinkable : std::false_type { };

    template <typename Container>
    struct is_shrinkable<
        Container,
        std::void_t<
            decltype(std::declval<Container>().resize(std::declval<std::size_t>())),
            decltype(std::declval<Container>().shrink_to_fit())>>
        : std::true_type { };

    template <typename Container>
    inline constexpr bool is_shrinkable_v = is_shrinkable<Container>::value;
}


template <
    typename ElementT,
    std::size_t InitialCapacity,
    typename Container,
    typename std::enable_if_t<
        std::is_class_v<typename Container> &&
        std::is_same_v<typename Container::value_type, ElementT>, int> = 0>
class circular
{
public:
    typedef typename Container container_type;

    typedef std::size_t size_type;
    typedef std::make_signed_t<size_type> difference_type;

    typedef typename ElementT value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;

    static constexpr size_type initial_capacity = InitialCapacity;
    static constexpr bool resizable = detail::circular::is_resizable_v<Container>;


public:
    circular()
        : m_capacity{initial_capacity}
        , m_wpos{0}
        , m_loops{0}
    {
        static_assert(initial_capacity <= std::numeric_limits<difference_type>::max());

        if constexpr (resizable)
            this->resize(initial_capacity);
    }

    ~circular() = default;

    constexpr bool empty() const noexcept
    {
        return !(m_wpos | m_loops);
    }

    constexpr bool full() const noexcept
    {
        return 0 != m_loops;
    }

    constexpr size_type size() const noexcept
    {
        return (m_loops > 0) ? m_capacity : m_wpos;
    }

    constexpr size_type capacity() const noexcept
    {
        assert(m_container.size() == m_capacity);
        return m_capacity;
    }

    constexpr void clear() noexcept
    {
        m_wpos = 0;
        m_loops = 0;
    }

    constexpr void push_back(const_reference item) noexcept
    {
        assert(m_capacity > 0);
        assert(m_wpos <= m_capacity);
        if (m_capacity > 0)
        {
            if (m_wpos >= m_capacity)
            {
                m_wpos = 0;
                if (!++m_loops)  // overflow?
                    ++m_loops;
            }
            m_container[m_wpos++] = item;
        }
    }

    constexpr void push_back(value_type&& item) noexcept
    {
        assert(m_capacity > 0);
        assert(m_wpos <= m_capacity);
        if (m_capacity > 0)
        {
            if (m_wpos >= m_capacity)
            {
                m_wpos = 0;
                if (!++m_loops)  // overflow?
                    ++m_loops;
            }
            m_container[m_wpos++] = std::move(item);
        }
    }

    constexpr const_reference front() const noexcept
    {
        return (*this)[0];  // oldest pos
    }

    constexpr reference front() noexcept
    {
        return (*this)[0];  // oldest pos
    }

    constexpr const_reference back() const noexcept
    {
        return (*this)[this->size() - 1];  // most recent pos
    }

    constexpr reference back() noexcept
    {
        return (*this)[this->size() - 1];  // most recent pos
    }

    constexpr const_reference at(size_type pos) const
    {
        return (*this)[pos];
    }

    constexpr reference at(size_type pos)
    {
        return (*this)[pos];
    }

    constexpr reference operator[](size_type pos)
    {
        assert(!this->empty());
        assert(pos < this->size());
        const auto idx = m_loops ? m_wpos % m_capacity + pos : pos;
        return m_container[idx];
    }

    constexpr const_reference operator[](size_type pos) const
    {
        assert(!this->empty());
        assert(pos < this->size());
        const auto idx = m_loops ? m_wpos % m_capacity + pos : pos;
        return m_container[idx];
    }

    template <typename Dummy = Container>
    typename std::enable_if_t<
        detail::circular::is_resizable_v<Dummy>,
        void>
    resize(size_type new_capacity)
    {
        assert(new_capacity <= std::numeric_limits<difference_type>::max());

        if (new_capacity == m_capacity)
            return;

        if (new_capacity <= 0)
        {
            m_container.resize(1);
            m_capacity = 0;
            m_wpos = 0;
            m_loops = 0;
        }
        else if (this->empty())
        {
            assert(!m_wpos);
            assert(!m_loops);
            m_container.resize(new_capacity);
            m_capacity = new_capacity;
            m_wpos = 0;
            m_loops = 0;
        }
        else if (0 == m_loops)
        {
            // CAUTION: m_container.resize() is assumed not to destroy first
            // items (i.e. std::vector supported)
            m_container.resize(new_capacity);
            if (new_capacity <= m_capacity)
                m_wpos = m_wpos % (new_capacity + 1);
            m_capacity = new_capacity;
        }
        else
        {
            assert(m_loops > 0);
            assert(this->size() == m_capacity);  // because m_loops > 0

            const size_type new_size = std::min(new_capacity, m_capacity);
            container_type new_container;
            size_type new_wpos = 0;

            // do not assume constructor is standard, call resize() explicitly
            new_container.resize(new_capacity);

            // move the tail of source buffer to the head of dest buffer
            // if new_capacity < m_capacity, keep the most recent entries only
            if (m_wpos < m_capacity && new_size > m_wpos)
            {
                const auto pos = m_wpos + (m_capacity - new_size);

                assert(pos < m_capacity);

                std::move(
                    // std::execution::par_unseq,  // C++20
                    std::next(
                        m_container.begin(),
                        static_cast<difference_type>(pos)),
                    std::next(
                        m_container.begin(),
                        static_cast<difference_type>(m_capacity)),
                    new_container.begin());

                new_wpos += m_capacity - pos;

                assert(new_wpos <= new_size);
                assert(new_wpos <= new_capacity);
            }

            // move the head of source buffer to the tail of dest buffer
            // if new_capacity < m_capacity, keep the most recent entries only
            if (m_wpos > 0 && new_size > new_wpos)
            {
                const auto pos = m_wpos - (new_size - new_wpos);

                assert(pos >= 0);
                assert(pos < m_capacity);
                assert(pos < m_wpos);

                std::move(
                    // std::execution::par_unseq,  // C++20
                    std::next(
                        m_container.begin(),
                        static_cast<difference_type>(pos)),
                    std::next(
                        m_container.begin(),
                        static_cast<difference_type>(m_wpos)),
                    new_container.begin());

                new_wpos += m_wpos - pos;

                assert(new_wpos <= new_size);
                assert(new_wpos <= new_capacity);
            }

            assert(new_wpos <= new_size);
            assert(new_wpos <= new_capacity);

            m_container.swap(new_container);
            m_capacity = new_capacity;
            m_wpos = new_wpos;
            m_loops = 0;
        }
    }

    template <typename Dummy = Container>
    typename std::enable_if_t<
        detail::circular::is_shrinkable_v<Dummy>,
        constexpr void>
    shrink_to_fit()
    {
        m_container.shrink_to_fit();
    }

    constexpr container_type& container()
    {
        return m_container;
    }


private:
    container_type m_container;
    size_type m_capacity;
    size_type m_wpos;
    size_type m_loops;
};


template <typename T, std::size_t N>
using circular_array = circular<T, N, std::array<T, N>>;

template <
    typename T,
    std::size_t InitialCapacity,
    typename Allocator = std::allocator<T>>
using circular_vector =
    circular<T, InitialCapacity, std::vector<T, Allocator>>;


}  // namespace cix
