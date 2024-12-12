#pragma once
#include "y3c/exception.h"
#include <array>

namespace y3c {
namespace internal {
template <typename T>
class array_iterator {
  public:
    array_iterator() = default;

    array_iterator &operator++() {
        ++iter_;
        return *this;
    }
    array_iterator operator++(int) {
        auto tmp = *this;
        ++iter_;
        return tmp;
    }
    array_iterator &operator--() {
        --iter_;
        return *this;
    }
    array_iterator operator--(int) {
        auto tmp = *this;
        --iter_;
        return tmp;
    }
    reference operator*() { return *iter_; }
    pointer operator->() { return iter_.operator->(); }

    bool operator==(const iterator &rhs) const { return iter_ == rhs.iter_; }
    bool operator!=(const iterator &rhs) const { return iter_ != rhs.iter_; }
};

} // namespace internal

/*!
 * `at()`, `operator[]`, `front()`, `back()` で範囲外アクセスを検出する
 *
 * std::arrayと違って集成体初期化はできない
 * (できるようにする必要はあるのか?)
 */
template <class T, std::size_t N>
class array {
    using Base = std::array<T, N>;
    Base base_;

  public:
    array() = default;
    array(const Base &elems) : base_(elems) {}
    array(Base &&elems) : base_(std::move(elems)) {}

    array &operator=(const Base &elems) {
        base_ = elems;
        return *this;
    }
    array &operator=(Base &&elems) {
        base_ = std::move(elems);
        return *this;
    }

    const Base &unwrap() const noexcept { return base_; }

    using reference = typename Base::reference;
    using const_reference = typename Base::const_reference;
    // using iterator = typename Base::iterator;
    // using const_iterator = typename Base::const_iterator;
    // using reverse_iterator = typename Base::reverse_iterator;
    // using const_reverse_iterator = typename Base::const_reverse_iterator;
    using size_type = typename Base::size_type;
    using difference_type = typename Base::difference_type;
    using pointer = typename Base::pointer;
    using const_pointer = typename Base::const_pointer;
    using value_type = typename Base::value_type;

    reference at(size_type n) {
        if (n >= N) {
            throw y3c::exception_std::out_of_range("y3c::array::at()", N, n);
        }
        return base_.at(n);
    }
    const_reference at(size_type n) const {
        if (n >= N) {
            throw y3c::exception_std::out_of_range("y3c::array::at()", N, n);
        }
        return base_.at(n);
    }
    reference operator[](size_type n) {
        if (n >= N) {
            y3c::internal::ub_out_of_range("y3c::array::operator[]()", N, n);
        }
        return base_[n];
    }
    const_reference operator[](size_type n) const {
        if (n >= N) {
            y3c::internal::ub_out_of_range("y3c::array::operator[]()", N, n);
        }
        return base_[n];
    }

    reference front() {
        if (N == 0) {
            y3c::internal::ub_out_of_range("y3c::array::front()", N, 0);
        }
        return base_.front();
    }
    const_reference front() const {
        if (N == 0) {
            y3c::internal::ub_out_of_range("y3c::array::front()", N, 0);
        }
        return base_.front();
    }
    reference back() {
        if (N == 0) {
            y3c::internal::ub_out_of_range("y3c::array::back()", N, 0);
        }
        return base_.back();
    }
    const_reference back() const {
        if (N == 0) {
            y3c::internal::ub_out_of_range("y3c::array::back()", N, 0);
        }
        return base_.back();
    }

    pointer data() noexcept { return base_.data(); }
    const_pointer data() const noexcept { return base_.data(); }
};

} // namespace y3c
