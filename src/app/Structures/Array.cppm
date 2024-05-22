module;
#include "pch.hpp"
export module Array;


template<typename T>
class Array {
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
    std::unique_ptr<T[]> data;
    const std::size_t size;

    explicit Array(const std::size_t size) : data(std::make_unique<T[]>(size)), size(size) {}
    // Array(const std::size_t size, allocator) : data(std::make_unique<T[]>(size)), size(size) {}

    iterator begin() {
        return iterator(&data);
    }

    iterator end() {
        return iterator(&data + size);
    }
};
