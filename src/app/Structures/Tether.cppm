module;
#include "pch.hpp"
export module Tether;


export template<typename T>
class Tether {
    /// Weak reference to a region in memory of a certain type.
    struct iterator {
        using iterator_category = std::contiguous_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        explicit iterator(const T *p_data) : ptr(p_data) {}

        iterator &operator=(const iterator &) noexcept = default;
        [[nodiscard]] reference operator*() const noexcept { return *ptr; }
        [[nodiscard]] pointer operator->() const noexcept { return ptr; }
        iterator &operator++() noexcept { ++ptr; return *this; }
        iterator operator++(int) noexcept { iterator retval = *this; ++*this; return retval; } // Gets a warning, but this is exactly the msvc code for vector::operator++(int)
        iterator &operator--() noexcept { --ptr; return *this; }
        iterator operator--(int) noexcept { iterator retval = *this; --*this; return retval; }  // Gets a warning, but this is exactly the msvc code for vector::operator--(int)
        iterator &operator+=(const difference_type rhs) noexcept { ptr += rhs; return *this; }
        [[nodiscard]] iterator operator+(const difference_type rhs) const noexcept { iterator retval = *this; retval += rhs; return retval; }
        iterator &operator-=(const difference_type _off) noexcept { ptr += -_off; return *this; }
        [[nodiscard]] iterator operator-(const difference_type rhs) const noexcept { iterator retval = *this; retval -= rhs; return retval; }
        [[nodiscard]] difference_type operator-(const iterator &rhs) const noexcept { return ptr - rhs.ptr; }
        [[nodiscard]] reference operator[](const difference_type rhs) const noexcept { return *(*this + rhs); }
        [[nodiscard]] bool operator==(const iterator &rhs) const noexcept { return ptr == rhs.ptr; }
        [[nodiscard]] std::strong_ordering operator<=>(const iterator &rhs) { return std::addressof(*ptr) <=> std::addressof(*rhs.ptr); }

    private:
        pointer ptr;
    };

    struct const_iterator {
        using iterator_category = std::contiguous_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        explicit const_iterator(const T *p_data) : ptr(p_data) {}

        constexpr const_iterator &operator=(const const_iterator &) noexcept = default;
        [[nodiscard]] constexpr reference operator*() const noexcept { return *ptr; }
        [[nodiscard]] constexpr pointer operator->() const noexcept { return ptr; }
        constexpr const_iterator &operator++() noexcept { ++ptr; return *this; }
        constexpr const_iterator operator++(int) noexcept { const_iterator retval = *this; ++*this; return retval; } // Gets a warning, but this is exactly the msvc code for vector::operator++(int)
        constexpr const_iterator &operator--() noexcept { --ptr; return *this; }
        constexpr const_iterator operator--(int) noexcept { const_iterator retval = *this; --*this; return retval; }  // Gets a warning, but this is exactly the msvc code for vector::operator--(int)
        constexpr const_iterator &operator+=(const difference_type rhs) noexcept { ptr += rhs; return *this; }
        [[nodiscard]] constexpr const_iterator operator+(const difference_type rhs) const noexcept { const_iterator retval = *this; retval += rhs; return retval; }
        constexpr const_iterator &operator-=(const difference_type _off) noexcept { ptr += -_off; return *this; }
        [[nodiscard]] constexpr const_iterator operator-(const difference_type rhs) const noexcept { const_iterator retval = *this; retval -= rhs; return retval; }
        [[nodiscard]] constexpr difference_type operator-(const const_iterator &rhs) const noexcept { return ptr - rhs.ptr; }
        [[nodiscard]] constexpr reference operator[](const difference_type rhs) const noexcept { return *(*this + rhs); }
        [[nodiscard]] constexpr bool operator==(const const_iterator &rhs) const noexcept { return ptr == rhs.ptr; }
        [[nodiscard]] constexpr std::strong_ordering operator<=>(const const_iterator &rhs) { return std::addressof(*ptr) <=> std::addressof(*rhs.ptr); }

    private:
        pointer ptr;
    };

    T *m_data;
    const std::size_t m_size;
public:

    Tether(T *data, const std::size_t size) : m_data(data), m_size(size) {}

    constexpr T& at(std::size_t pos) noexcept {
        return m_data[pos];
    }

    constexpr const T& at(std::size_t pos) const noexcept {
        return m_data[pos];
    }

    [[nodiscard]] constexpr T& operator[](std::size_t pos) noexcept {
        // assert here
        return m_data[pos];
    }

    [[nodiscard]] constexpr const T& operator[](std::size_t pos) const noexcept {
        // assert here
        return m_data[pos];
    }

    [[nodiscard]] constexpr iterator begin() noexcept { return iterator(m_data); }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return const_iterator(m_data); }

    [[nodiscard]] constexpr iterator end() noexcept { return iterator(m_data + m_size); }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return const_iterator(m_data + m_size); }

    [[nodiscard]] constexpr const T* data() const noexcept { return m_data; }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return m_size; }

    static void copy(const Tether &dst, const Tether &src) { std::memcpy(dst.data, src.data, src.size); }
    static void copy(const Tether &dst, T const *src) { std::memcpy(dst.data, src, dst.size); }
};
