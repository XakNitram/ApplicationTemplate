module;
#include "pch.hpp"
export module Tether;


template<typename T>
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

public:
    T *data;
    const std::size_t size;

    Tether(const T *data, const std::size_t size) : data(data), size(size) {}

    iterator begin() {
        return iterator(data);
    }

    iterator end() {
        return iterator(data + size);
    }

    static void copy(const Tether &dst, const Tether &src) {
        std::memcpy(dst.data, src.data, src.size);
    }

    static void copy(const Tether &dst, T const *src) {
        std::memcpy(dst.data, src, dst.size);
    }
};
