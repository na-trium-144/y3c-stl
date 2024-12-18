#pragma once
#include "y3c/terminate.h"
#include "y3c/wrap.h"
#include "y3c/typename.h"
#include <array>

namespace y3c {

template <typename T, std::size_t N>
class array;
template <typename T, std::size_t N>
std::array<T, N> &unwrap(array<T, N> &wrapper) noexcept;
template <typename T, std::size_t N>
const std::array<T, N> &unwrap(const array<T, N> &wrapper) noexcept;


namespace internal {
template <typename element_type>
class array_iterator : public ptr<element_type> {

    const std::string *type_name_;
    const std::string &type_name() const override {
        // return internal::get_type_name<array_iterator>();
        return *type_name_;
    }

    array_iterator(element_type *ptr_, internal::life_observer observer,
                   const std::string *type_name) noexcept
        : ptr<element_type>(ptr_, observer), type_name_(type_name) {}

  public:
    template <typename T, std::size_t N>
    friend class y3c::array;

    array_iterator &operator++() {
        ++this->ptr_;
        return *this;
    }
    array_iterator operator++(int) {
        array_iterator copy = *this;
        ++this->ptr_;
        return copy;
    }
    array_iterator &operator--() {
        --this->ptr_;
        return *this;
    }
    array_iterator operator--(int) {
        array_iterator copy = *this;
        --this->ptr_;
        return copy;
    }
    array_iterator &operator+=(std::ptrdiff_t n) {
        this->ptr_ += n;
        return *this;
    }
    array_iterator &operator-=(std::ptrdiff_t n) {
        this->ptr_ -= n;
        return *this;
    }
    array_iterator operator+(std::ptrdiff_t n) const {
        return array_iterator(this->ptr_ + n, this->observer_, type_name_);
    }
    array_iterator operator-(std::ptrdiff_t n) const {
        return array_iterator(this->ptr_ - n, this->observer_, type_name_);
    }
};
} // namespace internal

/*!
 * * `at()`, `operator[]`, `front()`, `back()` で範囲外アクセスを検出する
 * * `at()`, `operator[]`, `front()`, `back()`, `data()`, `begin()`, `end()`
 * などが返すポインタ、イテレータはラップされたものであり、
 * 使用時に範囲外でないかと参照先が生きているかのチェックができる。
 * * std::arrayと違って集成体初期化はできない
 * (できるようにする必要はあるのか?)
 * 
 * \todo reverse_iterator, const_reverse_iterator
 * 
 */
template <typename T, std::size_t N>
class array {
    std::array<T, N> base_;
    internal::life life_;

    const std::string &type_name() const {
        return internal::get_type_name<array>();
    }
    const std::string &iter_name() const {
        static std::string name =
            internal::get_type_name<array>() + "::iterator";
        return name;
    }
    const std::string &const_iter_name() const {
        static std::string name =
            internal::get_type_name<array>() + "::const_iterator";
        return name;
    }

  public:
    array() : base_(), life_(&base_) {}
    array(const array &other) : base_(other.base_), life_(&base_) {}
    array &operator=(const array &other) {
        this->base_ = other.base_;
        return *this;
    }
    array(array &&other) : base_(std::move(other.base_)), life_(&base_) {}
    array &operator=(array &&other) {
        if (this != std::addressof(other)) {
            this->base_ = std::move(other.base_);
        }
        return *this;
    }

    template <typename... Args,
              typename std::enable_if<(sizeof...(Args) == N),
                                      std::nullptr_t>::type = nullptr>
    array(Args &&...args)
        : array(std::array<T, N>{std::forward<Args>(args)...}) {}
    array(const std::array<T, N> &elems) : base_(elems), life_(&base_) {}
    array(std::array<T, N> &&elems) : base_(std::move(elems)), life_(&base_) {}
    array &operator=(const std::array<T, N> &elems) {
        this->base_ = elems;
        return *this;
    }
    array &operator=(std::array<T, N> &&elems) {
        this->base_ = std::move(elems);
        return *this;
    }

    using reference = wrap_ref<T>;
    using const_reference = const_wrap_ref<T>;
    using iterator = internal::array_iterator<T>;
    using const_iterator = internal::array_iterator<const T>;
    // using reverse_iterator = std::reverse_iterator<iterator>;
    // using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = ptr<T>;
    using const_pointer = const_ptr<T>;
    using value_type = T;

    friend std::array<T, N> &y3c::unwrap<T, N>(array<T, N> &wrapper) noexcept;
    friend const std::array<T, N> &
    y3c::unwrap<T, N>(const array<T, N> &wrapper) noexcept;

    wrap_auto<T> at(size_type n, internal::skip_trace_tag = {}) {
        if (n >= N) {
            static std::string func = type_name() + "::at()";
            throw y3c::out_of_range(func, N, static_cast<std::ptrdiff_t>(n));
        }
        return wrap_auto<T>(&this->base_[n], this->life_.observer());
    }
    wrap_auto<const T> at(size_type n, internal::skip_trace_tag = {}) const {
        if (n >= N) {
            static std::string func = type_name() + "::at()";
            throw y3c::out_of_range(func, N, static_cast<std::ptrdiff_t>(n));
        }
        return wrap_auto<const T>(&this->base_[n], this->life_.observer());
    }
    template <typename = internal::skip_trace_tag>
    wrap_auto<T> operator[](size_type n) {
        if (n >= N) {
            static std::string func = type_name() + "::operator[]()";
            y3c::internal::terminate_ub_out_of_range(
                func, N, static_cast<std::ptrdiff_t>(n));
        }
        return wrap_auto<T>(&this->base_[n], this->life_.observer());
    }
    template <typename = internal::skip_trace_tag>
    wrap_auto<const T> operator[](size_type n) const {
        if (n >= N) {
            static std::string func = type_name() + "::operator[]()";
            y3c::internal::terminate_ub_out_of_range(
                func, N, static_cast<std::ptrdiff_t>(n));
        }
        return wrap_auto<const T>(&this->base_[n], this->life_.observer());
    }

    wrap_auto<T> front(internal::skip_trace_tag = {}) {
        if (N == 0) {
            static std::string func = type_name() + "::front()";
            y3c::internal::terminate_ub_out_of_range(func, N, 0);
        }
        return wrap_auto<T>(&this->base_.front(), this->life_.observer());
    }
    wrap_auto<const T> front(internal::skip_trace_tag = {}) const {
        if (N == 0) {
            static std::string func = type_name() + "::front()";
            y3c::internal::terminate_ub_out_of_range(func, N, 0);
        }
        return wrap_auto<const T>(&this->base_.front(), this->life_.observer());
    }
    wrap_auto<T> back(internal::skip_trace_tag = {}) {
        if (N == 0) {
            static std::string func = type_name() + "::back()";
            y3c::internal::terminate_ub_out_of_range(func, N, 0);
        }
        return wrap_auto<T>(&this->base_.back(), this->life_.observer());
    }
    wrap_auto<const T> back(internal::skip_trace_tag = {}) const {
        if (N == 0) {
            static std::string func = type_name() + "::back()";
            y3c::internal::terminate_ub_out_of_range(func, N, 0);
        }
        return wrap_auto<const T>(&this->base_.back(), this->life_.observer());
    }

    pointer data() {
        if (N == 0) {
            return pointer(nullptr, this->life_.observer());
        }
        return pointer(&this->base_[0], this->life_.observer());
    }
    const_pointer data() const {
        if (N == 0) {
            return const_pointer(nullptr, this->life_.observer());
        }
        return const_pointer(&this->base_[0], this->life_.observer());
    }

    iterator begin() {
        if (N == 0) {
            return iterator(nullptr, this->life_.observer(), &iter_name());
        }
        return iterator(&this->base_.front(), this->life_.observer(),
                        &iter_name());
    }
    const_iterator begin() const {
        if (N == 0) {
            return const_iterator(nullptr, this->life_.observer(),
                                  &const_iter_name());
        }
        return const_iterator(&this->base_.front(), this->life_.observer(),
                              &const_iter_name());
    }
    const_iterator cbegin() const { return begin(); }
    iterator end() { return begin() + N; }
    const_iterator end() const { return begin() + N; }
    const_iterator cend() const { return begin() + N; }

    bool empty() const noexcept { return N == 0; }
    size_type size() const noexcept { return N; }
    size_type max_size() const noexcept { return N; }

    void fill(const T &value) { this->base_.fill(value); }
    void swap(array &other) { this->base_.swap(unwrap(other)); }

    operator wrap<array &>() noexcept {
        return wrap<array &>(this, life_.observer());
    }
    operator wrap<const array &>() const noexcept {
        return wrap<const array &>(this, life_.observer());
    }
    wrap<array *> operator&() { return wrap<array *>(this, life_.observer()); }
    wrap<const array *> operator&() const {
        return wrap<const array *>(this, life_.observer());
    }
};

template <typename T, std::size_t N>
std::array<T, N> &unwrap(array<T, N> &wrapper) noexcept {
    return wrapper.base_;
}
template <typename T, std::size_t N>
const std::array<T, N> &unwrap(const array<T, N> &wrapper) noexcept {
    return wrapper.base_;
}

} // namespace y3c
