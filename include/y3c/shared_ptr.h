#pragma once
#include "y3c/terminate.h"
#include <memory>

namespace y3c {

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

    template <typename... Args>
    shared_ptr(Args &&...args) : base_(std::forward<Args>(args)...) {}
    template <typename Arg>
    shared_ptr &operator=(Arg &&arg) {
        base_ = arg;
        return *this;
    }

    operator std::shared_ptr<T>() const { return check_(); }

    template <typename... Args>
    void reset(Args &&...args) {
        base_.reset(std::forward<Args>(args)...);
    }

    void swap(std::shared_ptr<T> &other) noexcept { base_.swap(other); }
    void swap(shared_ptr &other) noexcept { this->swap(other.base_); }

    template <typename U = T>
    U *get() const {
        return check_().get();
    }
    template <typename U = T>
    U &operator*() const {
        return *check_strict_();
    }
    template <typename U = T>
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
    template <typename Arg>
    bool owner_before(const Arg &arg) const {
        return base_.owner_before(arg);
    }
};

namespace strict {
template <typename T>
using shared_ptr = y3c::shared_ptr<T, true>;
}
} // namespace y3c
