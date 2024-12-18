#pragma once
#include "y3c/terminate.h"
#include "y3c/wrap.h"
#include <memory>

Y3C_NS_BEGIN
template <typename T>
class shared_ptr;
// template <typename T>
// std::shared_ptr<T> &unwrap(shared_ptr<T> &wrapper) noexcept;
template <typename T>
const std::shared_ptr<T> &unwrap(const shared_ptr<T> &wrapper) noexcept;

/*!
 * * `operator*` と `operator->` 使用時にnullptrチェックを行う。
 * * `get()` が返すポインタはラップされたものであり、
 * 使用時にnullptrと参照先が生きているかのチェックができる。
 * * 初期化は std::make_shared<T> または y3c::make_shared<T> で行う。
 *
 */
template <typename T>
class shared_ptr {
    std::shared_ptr<T> base_;
    std::shared_ptr<internal::life> ptr_life_;
    internal::life life_;

  public:
    shared_ptr(std::nullptr_t = nullptr) noexcept
        : base_(nullptr), ptr_life_(nullptr), life_(&base_) {}
    shared_ptr(const shared_ptr &other)
        : base_(other.base_), ptr_life_(other.ptr_life_), life_(&base_) {}
    shared_ptr &operator=(const shared_ptr &other) {
        if (this != std::addressof(other)) {
            base_ = other.base_;
            ptr_life_ = other.ptr_life_;
        }
        return *this;
    }
    shared_ptr(shared_ptr &&other)
        : base_(std::move(other.base_)), ptr_life_(std::move(other.ptr_life_)),
          life_(&base_) {}
    shared_ptr &operator=(shared_ptr &&other) {
        if (this != std::addressof(other)) {
            base_ = std::move(other.base_);
            ptr_life_ = std::move(other.ptr_life_);
        }
        return *this;
    }
    ~shared_ptr() = default;

    using element_type = T;

    template <typename U>
    shared_ptr(const std::shared_ptr<U> &ptr)
        : base_(ptr), ptr_life_(std::make_shared<internal::life>(base_.get())),
          life_(&base_) {}
    template <typename U>
    shared_ptr &operator=(const std::shared_ptr<U> &ptr) {
        base_ = ptr;
        ptr_life_ = std::make_shared<internal::life>(base_.get());
        return *this;
    }
    template <typename U>
    shared_ptr(std::shared_ptr<U> &&ptr)
        : base_(std::move(ptr)),
          ptr_life_(std::make_shared<internal::life>(base_.get())),
          life_(&base_) {}
    template <typename U>
    shared_ptr &operator=(std::shared_ptr<U> &&ptr) {
        base_ = std::move(ptr);
        ptr_life_ = std::make_shared<internal::life>(base_.get());
        return *this;
    }

    template <typename U>
    shared_ptr(const shared_ptr<U> &other)
        : base_(other.base_), ptr_life_(other.ptr_life_), life_(&base_) {}
    template <typename U>
    shared_ptr &operator=(const shared_ptr<U> &other) {
        base_ = other.base_;
        ptr_life_ = other.ptr_life_;
        return *this;
    }
    template <typename U>
    shared_ptr(shared_ptr<U> &&other)
        : base_(std::move(other.base_)), ptr_life_(std::move(other.ptr_life_)),
          life_(&base_) {}
    template <typename U>
    shared_ptr &operator=(shared_ptr<U> &&other) {
        base_ = std::move(other.base_);
        ptr_life_ = std::move(other.ptr_life_);
        return *this;
    }

    template <typename U>
    friend class shared_ptr;
    // friend std::shared_ptr<T> &y3c::unwrap(shared_ptr<T> &wrapper) noexcept;
    friend const std::shared_ptr<T> &
    y3c::unwrap(const shared_ptr<T> &wrapper) noexcept;

    void reset() noexcept {
        base_.reset();
        ptr_life_.reset();
    }
    void swap(shared_ptr &other) noexcept {
        base_.swap(other.base_);
        ptr_life_.swap(other.ptr_life_);
    }

    ptr<element_type> get() const noexcept {
        if (!base_) {
            return nullptr;
        }
        assert(ptr_life_);
        return ptr<element_type>(base_.get(), ptr_life_->observer());
    }
    template <typename = internal::skip_trace_tag>
    y3c::wrap_auto<T> operator*() const {
        if (!base_) {
            y3c::internal::terminate_ub_access_nullptr(
                "y3c::shared_ptr::operator*()");
        }
        assert(ptr_life_);
        return y3c::wrap_auto<T>(base_.get(), ptr_life_->observer());
    }
    template <typename = internal::skip_trace_tag>
    element_type *operator->() const {
        if (!base_) {
            y3c::internal::terminate_ub_access_nullptr(
                "y3c::shared_ptr::operator->()");
        }
        assert(ptr_life_);
        return base_.get();
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

    operator wrap<shared_ptr &>() noexcept {
        return wrap<shared_ptr &>(this, life_.observer());
    }
    operator wrap<const shared_ptr &>() const noexcept {
        return wrap<const shared_ptr &>(this, life_.observer());
    }
    wrap<shared_ptr *> operator&() {
        return wrap<shared_ptr *>(this, life_.observer());
    }
    wrap<const shared_ptr *> operator&() const {
        return wrap<const shared_ptr *>(this, life_.observer());
    }
};

// template <typename T>
// std::shared_ptr<T> &unwrap(shared_ptr<T> &wrapper) noexcept {
//     return wrapper.base_;
// }
template <typename T>
const std::shared_ptr<T> &unwrap(const shared_ptr<T> &wrapper) noexcept {
    return wrapper.base_;
}

template <typename T>
void swap(shared_ptr<T> &lhs, shared_ptr<T> &rhs) noexcept {
    lhs.swap(rhs);
}

template <typename T, typename U>
bool operator==(const shared_ptr<T> &lhs, const shared_ptr<U> &rhs) noexcept {
    return unwrap(lhs) == unwrap(rhs);
}
template <typename T, typename U>
bool operator!=(const shared_ptr<T> &lhs, const shared_ptr<U> &rhs) noexcept {
    return unwrap(lhs) != unwrap(rhs);
}
template <typename T, typename U>
bool operator<(const shared_ptr<T> &lhs, const shared_ptr<U> &rhs) noexcept {
    return unwrap(lhs) < unwrap(rhs);
}
template <typename T, typename U>
bool operator<=(const shared_ptr<T> &lhs, const shared_ptr<U> &rhs) noexcept {
    return unwrap(lhs) <= unwrap(rhs);
}
template <typename T, typename U>
bool operator>(const shared_ptr<T> &lhs, const shared_ptr<U> &rhs) noexcept {
    return unwrap(lhs) > unwrap(rhs);
}
template <typename T, typename U>
bool operator>=(const shared_ptr<T> &lhs, const shared_ptr<U> &rhs) noexcept {
    return unwrap(lhs) >= unwrap(rhs);
}

template <class CharT, class Traits, typename T>
std::basic_ostream<CharT, Traits> &
operator<<(std::basic_ostream<CharT, Traits> &os, const shared_ptr<T> &p) {
    return os << unwrap(p);
}

template <typename T, typename... Args>
shared_ptr<T> make_shared(Args &&...args) {
    return shared_ptr<T>(std::make_shared<T>(std::forward<Args>(args)...));
}

Y3C_NS_END
