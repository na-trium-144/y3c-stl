#pragma once
#include "y3c/exception.h"
#include <memory>
#include <type_traits>

namespace y3c {

/*!
 * `*ptr` と `ptr->` 使用時にnullptrチェックを行う。
 *
 */
template <typename T>
class shared_ptr {
    using Base = std::shared_ptr<T>;
    Base base_;

  public:
    shared_ptr() = default;

    shared_ptr(const Base &ptr) noexcept : base_(ptr) {}
    shared_ptr(Base &&ptr) noexcept : base_(std::move(ptr)) {}
    shared_ptr &operator=(const Base &ptr) noexcept {
        base_ = ptr;
        return *this;
    }
    shared_ptr &operator=(Base &&ptr) noexcept {
        base_ = std::move(ptr);
        return *this;
    }

    const Base &unwrap() const noexcept { return base_; }

    using element_type = typename Base::element_type;

    shared_ptr(T *ptr) : base_(ptr) {}

    void reset() noexcept { base_.reset(); }
    void swap(shared_ptr &other) noexcept { this->swap(other.base_); }

    template <typename U = T,
              typename std::enable_if<std::is_same<U, T>::value,
                                      std::nullptr_t>::type = nullptr>
    U *get() const noexcept {
        return base_.get();
    }
    template <typename U = T,
              typename std::enable_if<std::is_same<U, T>::value,
                                      std::nullptr_t>::type = nullptr>
    U &operator*() const {
        if (!base_) {
            y3c::internal::undefined_behavior(
                "y3c::shared_ptr::operator*()",
                "tried to access the value of nullptr, "
                "or uninitialized shared_ptr.");
        }
        return *base_;
    }
    template <typename U = T,
              typename std::enable_if<std::is_same<U, T>::value,
                                      std::nullptr_t>::type = nullptr>
    U *operator->() const {
        if (!base_) {
            y3c::internal::undefined_behavior(
                "y3c::shared_ptr::operator->()",
                "tried to access the value of nullptr, "
                "or uninitialized shared_ptr.");
        }
        return get();
    }
    // template <typename U = T>
    // U &operator[](std::ptrdiff_t i) const {
    //     if (i < 0 || todo)
    //     return check_()[i];
    // }

    long use_count() const noexcept { return base_.use_count(); }
    explicit operator bool() const noexcept { return static_cast<bool>(base_); }
    bool owner_before(const shared_ptr &arg) const {
        return base_.owner_before(arg.base_);
    }

    friend bool operator==(const shared_ptr &lhs,
                           const shared_ptr &rhs) noexcept {
        return lhs.base_ == rhs.base_;
    }
    friend bool operator!=(const shared_ptr &lhs,
                           const shared_ptr &rhs) noexcept {
        return lhs.base_ != rhs.base_;
    }
    friend bool operator<(const shared_ptr &lhs,
                          const shared_ptr &rhs) noexcept {
        return lhs.base_ < rhs.base_;
    }
    friend bool operator<=(const shared_ptr &lhs,
                           const shared_ptr &rhs) noexcept {
        return lhs.base_ <= rhs.base_;
    }
    friend bool operator>(const shared_ptr &lhs,
                          const shared_ptr &rhs) noexcept {
        return lhs.base_ > rhs.base_;
    }
    friend bool operator>=(const shared_ptr &lhs,
                           const shared_ptr &rhs) noexcept {
        return lhs.base_ >= rhs.base_;
    }
    template <class CharT, class Traits>
    friend std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> &os, const shared_ptr &p) {
        return os << p.base_;
    }
};

template <typename T>
void swap(shared_ptr<T> &lhs, shared_ptr<T> &rhs) noexcept {
    lhs.swap(rhs);
}
template <typename U, typename T>
shared_ptr<U> static_pointer_cast(const shared_ptr<T> &r) noexcept {
    return static_pointer_cast<U>(r.unwrap());
}
template <typename U, typename T>
shared_ptr<U> dynamic_pointer_cast(const shared_ptr<T> &r) noexcept {
    return dynamic_pointer_cast<U>(r.unwrap());
}
template <typename U, typename T>
shared_ptr<U> const_pointer_cast(const shared_ptr<T> &r) noexcept {
    return const_pointer_cast<U>(r.unwrap());
}

template <typename T>
const std::shared_ptr<T> &unwrap(const shared_ptr<T> &ptr) noexcept {
    return ptr.unwrap();
}

} // namespace y3c
