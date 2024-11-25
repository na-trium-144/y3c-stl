#pragma once
#include "y3c/terminate.h"
#include <memory>
#include <type_traits>

namespace y3c {

/*!
 * y3c::shared_ptr は `*ptr` と `ptr->` 使用時にnullptrチェックを行う。
 *
 * y3c::strict::shared_ptr はそれに加えて `get()`, `y3c::unwrap()`
 * でもnullptrチェックを行う。
 *
 */
template <typename T, bool StrictMode = false>
class shared_ptr {
    std::shared_ptr<T> base_;

    std::shared_ptr<T> check_() const {
        if (StrictMode && !base_) {
            y3c::internal::terminate();
        }
        return base_;
    }
    std::shared_ptr<T> check_strict_() const {
        if (!base_) {
            y3c::internal::terminate();
        }
        return base_;
    }

  public:
    shared_ptr() = default;
    shared_ptr(const shared_ptr &) = default;
    shared_ptr &operator=(const shared_ptr &) = default;
    shared_ptr(shared_ptr &&) = default;
    shared_ptr &operator=(shared_ptr &&) = default;
    ~shared_ptr() = default;

    shared_ptr(const std::shared_ptr<T> &ptr) noexcept : base_(ptr) {}
    shared_ptr(std::shared_ptr<T> &&ptr) noexcept : base_(std::move(ptr)) {}
    shared_ptr &operator=(const std::shared_ptr<T> &ptr) noexcept {
        base_ = ptr;
        return *this;
    }
    shared_ptr &operator=(std::shared_ptr<T> &&ptr) noexcept {
        base_ = std::move(ptr);
        return *this;
    }

    std::shared_ptr<T> unwrap() const { return check_(); }

    void reset() noexcept { base_.reset(); }
    void swap(shared_ptr &other) noexcept { this->swap(other.base_); }

    template <typename U = T,
              typename std::enable_if<std::is_same<U, T>::value,
                                      std::nullptr_t>::type = nullptr>
    U *get() const {
        return check_().get();
    }
    template <typename U = T,
              typename std::enable_if<std::is_same<U, T>::value,
                                      std::nullptr_t>::type = nullptr>
    U &operator*() const {
        return *check_strict_();
    }
    template <typename U = T,
              typename std::enable_if<std::is_same<U, T>::value,
                                      std::nullptr_t>::type = nullptr>
    U *operator->() const {
        return check_strict_().get();
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

    template <typename U>
    shared_ptr<U> static_pointer_cast() const {
        return std::static_pointer_cast<U>(base_);
    }
    template <typename U>
    shared_ptr<U> dynamic_pointer_cast() const {
        return std::dynamic_pointer_cast<U>(base_);
    }
    template <typename U>
    shared_ptr<U> const_pointer_cast() const {
        return std::const_pointer_cast<U>(base_);
    }
};

template <typename T, bool StrictMode>
std::shared_ptr<T> unwrap(const shared_ptr<T, StrictMode> &ptr) {
    return ptr.unwrap();
}
template <typename T, bool StrictMode>
void swap(shared_ptr<T, StrictMode> &lhs,
          shared_ptr<T, StrictMode> &rhs) noexcept {
    lhs.swap(rhs);
}
template <typename U, typename T, bool StrictMode>
shared_ptr<U> static_pointer_cast(const shared_ptr<T, StrictMode> &r) {
    return r.template static_pointer_cast<U>();
}
template <typename U, typename T, bool StrictMode>
shared_ptr<U> dynamic_pointer_cast(const shared_ptr<T, StrictMode> &r) {
    return r.template dynamic_pointer_cast<U>();
}
template <typename U, typename T, bool StrictMode>
shared_ptr<U> const_pointer_cast(const shared_ptr<T, StrictMode> &r) {
    return r.template const_pointer_cast<U>();
}


namespace strict {
template <typename T>
using shared_ptr = y3c::shared_ptr<T, true>;
}
} // namespace y3c
