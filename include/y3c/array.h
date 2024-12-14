#pragma once
#include "y3c/internal.h"
#include "y3c/wrap.h"
#include <array>

Y3C_NS_BEGIN

/*!
 * `at()`, `operator[]`, `front()`, `back()` で範囲外アクセスを検出する
 *
 * `data()`, `begin()`, `end()`
 * などが返すポインタ、イテレータはラップされたものであり、
 * 使用時に範囲外でないかと参照先が生きているかのチェックができる。
 *
 * std::arrayと違って集成体初期化はできない
 * (できるようにする必要はあるのか?)
 */
template <class T, std::size_t N>
class array : wrap<std::array<T, N>> {
  public:
    array() = default;
    template <typename... Args>
    array(Args &&...args)
        : wrap<std::array<T, N>>(
              std::array<T, N>{std::forward<Args>(args)...}) {}
    array(const std::array<T, N> &elems) : wrap<std::array<T, N>>(elems) {}
    array(std::array<T, N> &&elems)
        : wrap<std::array<T, N>>(std::move(elems)) {}

    array &operator=(const std::array<T, N> &elems) {
        this->wrap<std::array<T, N>>::operator=(elems);
        return *this;
    }
    array &operator=(std::array<T, N> &&elems) {
        this->wrap<std::array<T, N>>::operator=(std::move(elems));
        return *this;
    }

    using reference = wrap_ref<T>;
    using const_reference = const_wrap_ref<T>;
    using iterator = ptr<T, internal::ptr_type_enum::array_iterator>;
    using const_iterator =
        ptr<const T, internal::ptr_type_enum::array_iterator>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = ptr<T>;
    using const_pointer = const_ptr<T>;
    using value_type = T;

    wrap_auto<T> at(size_type n) {
        if (n >= N) {
            throw y3c::out_of_range("y3c::array::at()", N, n);
        }
        return wrap_auto<T>(&this->unwrap().front(), N, &this->unwrap()[n],
                            this->alive());
    }
    wrap_auto<const T> at(size_type n) const {
        if (n >= N) {
            throw y3c::out_of_range("y3c::array::at()", N, n);
        }
        return wrap_auto<const T>(&this->unwrap().front(), N,
                                  &this->unwrap()[n], this->alive());
    }
    wrap_auto<T> operator[](size_type n) {
        if (n >= N) {
            y3c::internal::terminate_ub_out_of_range(
                "y3c::array::operator[]()", N, static_cast<std::ptrdiff_t>(n));
        }
        return wrap_auto<T>(&this->unwrap().front(), N, &this->unwrap()[n],
                            this->alive());
    }
    wrap_auto<const T> operator[](size_type n) const {
        if (n >= N) {
            y3c::internal::terminate_ub_out_of_range(
                "y3c::array::operator[]()", N, static_cast<std::ptrdiff_t>(n));
        }
        return wrap_auto<const T>(&this->unwrap().front(), N,
                                  &this->unwrap()[n], this->alive());
    }

    wrap_auto<T> front() {
        if (N == 0) {
            y3c::internal::terminate_ub_out_of_range("y3c::array::front()", N,
                                                     0);
        }
        return wrap_auto<T>(&this->unwrap().front(), N, &this->unwrap().front(),
                            this->alive());
    }
    wrap_auto<const T> front() const {
        if (N == 0) {
            y3c::internal::terminate_ub_out_of_range("y3c::array::front()", N,
                                                     0);
        }
        return wrap_auto<const T>(&this->unwrap().front(), N,
                                  &this->unwrap().front(), this->alive());
    }
    wrap_auto<T> back() {
        if (N == 0) {
            y3c::internal::terminate_ub_out_of_range("y3c::array::back()", N,
                                                     0);
        }
        return wrap_auto<T>(&this->unwrap().front(), N, &this->unwrap().back(),
                            this->alive());
    }
    wrap_auto<const T> back() const {
        if (N == 0) {
            y3c::internal::terminate_ub_out_of_range("y3c::array::back()", N,
                                                     0);
        }
        return wrap_auto<const T>(&this->unwrap().front(), N,
                                  &this->unwrap().back(), this->alive());
    }

    pointer data() {
        return pointer(&this->unwrap().front(), N, &this->unwrap().front(),
                       this->alive());
    }
    const_pointer data() const {
        return const_pointer(&this->unwrap().front(), N,
                             &this->unwrap().front(), this->alive());
    }

    iterator begin() {
        return iterator(&this->unwrap().front(), N, &this->unwrap().front(),
                        this->alive());
    }
    const_iterator begin() const {
        return const_iterator(&this->unwrap().front(), N,
                              &this->unwrap().front(), this->alive());
    }
    const_iterator cbegin() const { return begin(); }
    iterator end() { return begin() + N; }
    const_iterator end() const { return begin() + N; }
    const_iterator cend() const { return begin() + N; }

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }
    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(end());
    }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const { return reverse_iterator(begin()); }
    const_reverse_iterator crend() const { return reverse_iterator(begin()); }

    bool empty() const noexcept { return N == 0; }
    size_type size() const noexcept { return N; }
    size_type max_size() const noexcept { return N; }

    void fill(const T &value) { this->unwrap().fill(value); }
    void swap(array &other) { this->unwrap().swap(unwrap(other)); }
};

Y3C_NS_END
