#pragma once
#include "y3c/exception.h"
#include <array>

namespace y3c {

/*!
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
    using iterator = typename Base::iterator;
    using const_iterator = typename Base::const_iterator;
    using reverse_iterator = typename Base::reverse_iterator;
    using const_reverse_iterator = typename Base::const_reverse_iterator;
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
            y3c::internal::undefined_behavior("y3c::array::operator[]()", N, n);
        }
        return base_[n];
    }
    const_reference operator[](size_type n) const {
        if (n >= N) {
            y3c::internal::undefined_behavior("y3c::array::operator[]()", N, n);
        }
        return base_[n];
    }
};

} // namespace y3c
