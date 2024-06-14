module;
#include "pch.hpp"
export module SizedArray;

import Tether;


export template<typename T, std::size_t Size>
class SizedArray {
    struct iterator {
        using iterator_category = std::contiguous_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        explicit iterator(T *p_data) : ptr(p_data) {}

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

    T* m_data;
public:

    explicit SizedArray() : m_data(new T[Size]) {}
    // Array(const std::size_t size, allocator) : data(std::make_unique<T[]>(size)), size(size) {}

    ~SizedArray() { delete[] m_data; }

    constexpr T& at(std::size_t pos) noexcept { return m_data[pos]; }
    constexpr const T& at(std::size_t pos) const noexcept { return m_data[pos]; }

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

    [[nodiscard]] constexpr iterator end() noexcept { return iterator(m_data + Size); }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return const_iterator(m_data + Size); }

    [[nodiscard]] constexpr const T* data() const noexcept { return m_data; }
    [[nodiscard]] constexpr std::size_t size() const noexcept { return Size; }

    [[nodiscard]] constexpr Tether<T> tether() const noexcept { return Tether<T>{m_data, Size}; }
};
