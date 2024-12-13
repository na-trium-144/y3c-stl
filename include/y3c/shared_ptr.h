#pragma once
#ifdef Y3C_MESON
#include "y3c-config.h"
#else
#include "y3c/y3c-config.h"
#endif
#include "y3c/exception.h"
#include "y3c/wrap.h"
#include <memory>
#include <type_traits>

Y3C_NS_BEGIN

/*!
 * `*ptr` と `ptr->` 使用時にnullptrチェックを行う。
 *
 * `get()` は ptr<T> を返し、
 * 使用時にnullptrと参照先が生きているかのチェックができる。
 *
 */
template <typename T>
class shared_ptr : public wrap<std::shared_ptr<T>> {
    using base_type = std::shared_ptr<T>;
    mutable std::shared_ptr<bool> ptr_alive_;

    void check_dead() const noexcept {
        // 複数スレッドから同時アクセスしたらばぐるかな?

        if (use_count() == 1 && ptr_alive_) {
            *ptr_alive_ = false;
        }
    }

  public:
    shared_ptr(std::nullptr_t = nullptr) noexcept
        : wrap<std::shared_ptr<T>>(), ptr_alive_() {}
    ~shared_ptr() noexcept { check_dead(); }
    using element_type = T;

    template <typename U>
    shared_ptr(const std::shared_ptr<U> &ptr)
        : wrap<std::shared_ptr<T>>(ptr),
          ptr_alive_(std::make_shared<bool>(true)) {}
    template <typename U>
    shared_ptr &operator=(const std::shared_ptr<U> &ptr) {
        check_dead();
        this->wrap<std::shared_ptr<T>>::operator=(std::shared_ptr<T>(ptr));
        ptr_alive_ = std::make_shared<bool>(true);
        return *this;
    }
    template <typename U>
    shared_ptr(std::shared_ptr<U> &&ptr)
        : wrap<std::shared_ptr<T>>(std::move(ptr)),
          ptr_alive_(std::make_shared<bool>(true)) {}
    template <typename U>
    shared_ptr &operator=(std::shared_ptr<U> &&ptr) {
        check_dead();
        this->wrap<std::shared_ptr<T>>::operator=(
            std::shared_ptr<T>(std::move(ptr)));
        ptr_alive_ = std::make_shared<bool>(true);
        return *this;
    }

    template <typename U>
    shared_ptr(U *ptr)
        : wrap<std::shared_ptr<T>>(std::shared_ptr<T>(ptr)),
          ptr_alive_(std::make_shared<bool>(true)) {}
    template <typename U>
    shared_ptr &operator=(U *ptr) {
        check_dead();
        this->wrap<std::shared_ptr<T>>::operator=(std::shared_ptr<T>(ptr));
        ptr_alive_ = std::make_shared<bool>(true);
        return *this;
    }

    template <typename U>
    shared_ptr(const shared_ptr<U> &ref)
        : wrap<std::shared_ptr<T>>(std::shared_ptr<T>(unwrap(ref))),
          ptr_alive_(ref.ptr_alive_) {}
    template <typename U>
    shared_ptr &operator=(const shared_ptr<U> &ref) {
        check_dead();
        this->wrap<std::shared_ptr<T>>::operator=(
            std::shared_ptr<T>(unwrap(ref)));
        ptr_alive_ = ref.ptr_alive_;
        return *this;
    }
    shared_ptr(const shared_ptr &ref) = default;
    shared_ptr &operator=(const shared_ptr &ref) {
        if (this != std::addressof(ref)) {
            check_dead();
            this->wrap<std::shared_ptr<T>>::operator=(unwrap(ref));
            ptr_alive_ = ref.ptr_alive_;
        }
        return *this;
    }
    template <typename U>
    shared_ptr(shared_ptr<U> &&ref)
        : wrap<std::shared_ptr<T>>(std::shared_ptr<T>(unwrap(ref))),
          ptr_alive_(std::move(ref.ptr_alive_)) {
        ref.reset();
    }
    template <typename U>
    shared_ptr &operator=(shared_ptr<U> &&ref) {
        check_dead();
        this->wrap<std::shared_ptr<T>>::operator=(
            std::shared_ptr<T>(unwrap(ref)));
        ptr_alive_ = std::move(ref.ptr_alive_);
        ref.reset();
        return *this;
    }
    shared_ptr(shared_ptr &&ref) = default;
    shared_ptr &operator=(shared_ptr &&ref) {
        if (this != std::addressof(ref)) {
            check_dead();
            this->wrap<std::shared_ptr<T>>::operator=(unwrap(ref));
            ptr_alive_ = std::move(ref.ptr_alive_);
            ref.reset();
        }
        return *this;
    }

    template <typename U>
    friend class shared_ptr;

    void reset() noexcept {
        check_dead();
        this->unwrap().reset();
        ptr_alive_.reset();
    }
    void swap(shared_ptr &other) noexcept {
        this->unwrap().swap(other.base_);
        ptr_alive_.swap(other.ptr_alive_);
    }

    ptr<element_type> get() const noexcept {
        return ptr<element_type>(this->unwrap().get(), ptr_alive_);
    }
    template <typename U = T>
    y3c::wrap_ref<U> operator*() const {
        if (!this->unwrap()) {
            y3c::internal::undefined_behavior("y3c::shared_ptr::operator*()",
                                              y3c::msg::access_nullptr());
        }
        assert(ptr_alive_);
        return y3c::wrap_ref<T>(this->unwrap().get(), ptr_alive_);
    }
    element_type *operator->() const {
        if (!this->unwrap()) {
            y3c::internal::undefined_behavior("y3c::shared_ptr::operator->()",
                                              y3c::msg::access_nullptr());
        }
        assert(ptr_alive_);
        return this->unwrap().get();
    }
    // template <typename U = T>
    // U &operator[](std::ptrdiff_t i) const {
    //     if (i < 0 || todo)
    //     return check_()[i];
    // }

    long use_count() const noexcept { return this->unwrap().use_count(); }
    explicit operator bool() const noexcept {
        return static_cast<bool>(this->unwrap());
    }
    bool owner_before(const shared_ptr &arg) const {
        return this->unwrap().owner_before(arg.base_);
    }

    friend bool operator==(const shared_ptr &lhs,
                           const shared_ptr &rhs) noexcept {
        return unwrap(lhs) == unwrap(rhs);
    }
    friend bool operator!=(const shared_ptr &lhs,
                           const shared_ptr &rhs) noexcept {
        return unwrap(lhs) != unwrap(rhs);
    }
    friend bool operator<(const shared_ptr &lhs,
                          const shared_ptr &rhs) noexcept {
        return unwrap(lhs) < unwrap(rhs);
    }
    friend bool operator<=(const shared_ptr &lhs,
                           const shared_ptr &rhs) noexcept {
        return unwrap(lhs) <= unwrap(rhs);
    }
    friend bool operator>(const shared_ptr &lhs,
                          const shared_ptr &rhs) noexcept {
        return unwrap(lhs) > unwrap(rhs);
    }
    friend bool operator>=(const shared_ptr &lhs,
                           const shared_ptr &rhs) noexcept {
        return unwrap(lhs) >= unwrap(rhs);
    }
    template <class CharT, class Traits>
    friend std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> &os, const shared_ptr &p) {
        return os << unwrap(p);
    }
};

template <typename T>
void swap(shared_ptr<T> &lhs, shared_ptr<T> &rhs) noexcept {
    lhs.swap(rhs);
}

Y3C_NS_END
