#pragma once
#include "y3c/exception.h"
#include <memory>
#include <cassert>

namespace y3c {
template <typename T>
class wrap;
template <typename T>
class wrap_ref;
template <typename T>
class ptr;
template <typename T>
class shared_ptr;
template <typename T>
T &unwrap(wrap<T> &wrapper) noexcept;
template <typename T>
const T &unwrap(const wrap<T> &wrapper) noexcept;
template <typename T>
T &unwrap(const wrap_ref<T> &wrapper);

/*!
 * T型のデータ(base_)と、このコンテナの生存状態(alive_)を持つクラス
 *
 * y3cの各コンテナ型のベース
 *
 * alive_はshared_ptrとして wrap_ref
 * に渡され、wrapが破棄される時にfalseにする
 *
 */
template <typename T>
class wrap {
    using base_type = T;
    static_assert(!std::is_const<base_type>::value,
                  "y3c::wrap cannot have const value");
    static_assert(!std::is_reference<base_type>::value,
                  "y3c::wrap cannot have reference value");

    base_type base_;
    mutable std::shared_ptr<bool> alive_;

    const std::shared_ptr<bool> &alive() const {
        if (!alive_) {
            alive_ = std::make_shared<bool>(true);
        }
        return alive_;
    }

  protected:
    base_type &unwrap() noexcept { return base_; }
    const base_type &unwrap() const noexcept { return base_; }

  public:
    wrap() : base_(), alive_() {}
    wrap(const wrap &other) : base_(other.base_), alive_() {}
    wrap(wrap &&other) : base_(std::move(other.base_)), alive_() {}
    wrap &operator=(const wrap &other) {
        base_ = other.base_;
        return *this;
    }
    wrap &operator=(wrap &&other) {
        base_ = std::move(other.base_);
        return *this;
    }
    ~wrap() noexcept {
        if (alive_) {
            *alive_ = false;
        }
    }

    template <typename... Args>
    wrap(Args &&...args) : base_(std::forward<Args>(args)...), alive_() {}
    template <typename Args>
    wrap operator=(Args &&args) {
        base_ = std::forward<Args>(args);
        return *this;
    }
    wrap(const base_type &base) : base_(base), alive_() {}
    wrap(base_type &&base) : base_(std::move(base)), alive_() {}
    wrap &operator=(const base_type &base) {
        base_ = base;
        return *this;
    }
    wrap &operator=(base_type &&base) {
        base_ = std::move(base);
        return *this;
    }

    friend class wrap_ref<base_type>;
    friend base_type &y3c::unwrap<base_type>(wrap<base_type> &) noexcept;
    friend const base_type &
    y3c::unwrap<base_type>(const wrap<base_type> &) noexcept;

    operator base_type &() noexcept { return this->unwrap(); }
    operator const base_type &() const noexcept { return this->unwrap(); }

    ptr<base_type> operator&();
    ptr<const base_type> operator&() const;
};
template <typename T>
T &unwrap(wrap<T> &wrapper) noexcept {
    return wrapper.unwrap();
}
template <typename T>
const T &unwrap(const wrap<T> &wrapper) noexcept {
    return wrapper.unwrap();
}

/*!
 * T型のデータへのポインタと、その生存状態を持つクラス
 *
 * wrap<remove_const_t<T>> の左辺値からキャストできる
 *
 * wrap_ref<T> から const_wrap_ref<T> にキャストできる
 *
 * const_wrap_ref<T> = wrap_ref<const T> = const T &
 *
 */
template <typename T>
class wrap_ref {
    using element_type = T;
    static_assert(!std::is_void<typename std::remove_const<T>::type>::value,
                  "y3c::wrap_ref cannot have reference to void");
    static_assert(
        !std::is_reference<typename std::remove_const<T>::type>::value,
        "y3c::wrap cannot have reference to reference");

    element_type *ptr_;
    std::shared_ptr<bool> ptr_alive_;

    element_type *ptr_unwrap(const char *func) const {
        if (!ptr_) {
            y3c::internal::ub_nullptr(func);
        }
        assert(ptr_alive_);
        if (!*ptr_alive_) {
            y3c::internal::ub_deleted(func);
        }
        return ptr_;
    }

    wrap_ref(element_type *ptr, std::shared_ptr<bool> ptr_alive) noexcept
        : ptr_(ptr), ptr_alive_(ptr_alive) {}

    wrap_ref(std::nullptr_t = nullptr) noexcept : ptr_(nullptr), ptr_alive_() {}

  public:
    template <typename U>
    wrap_ref(wrap<U> &wrapper)
        : ptr_(&wrapper.base_), ptr_alive_(wrapper.alive()) {}

    template <typename U>
    wrap_ref(const wrap_ref<U> &ref)
        : ptr_(ref.ptr_), ptr_alive_(ref.ptr_alive_) {}

    friend class ptr<element_type>;
    friend class shared_ptr<element_type>;
    friend element_type &
    y3c::unwrap<element_type>(const wrap_ref<element_type> &);

    operator element_type &() {
        return *ptr_unwrap("cast from y3c::wrap_ref to reference");
    }
};

template <typename T>
using const_wrap_ref = wrap_ref<const T>;

template <typename T>
T &unwrap(const wrap_ref<T> &wrapper) {
    return *wrapper.ptr_unwrap("y3c::unwrap()");
}

/*!
 * 生ポインタのラッパー。
 *
 * `*ptr` と `ptr->`
 * 使用時にnullptrでないかと、参照先が生きているかのチェックを行う。
 *
 * T* にキャストできるがその場合チェックされないので注意
 *
 * ptr<T> から const_ptr<T> にキャストできる
 *
 * const_ptr<T> = ptr<const T> = const T *
 *
 */
template <typename T>
class ptr : public wrap<T *> {
    using element_type = T;
    static_assert(
        !std::is_reference<typename std::remove_const<T>::type>::value,
        "y3c::wrap cannot have pointer to reference");

    element_type *ptr_;
    std::shared_ptr<bool> ptr_alive_;

    element_type *ptr_unwrap(const char *func) const {
        if (!ptr_) {
            y3c::internal::ub_nullptr(func);
        }
        assert(ptr_alive_);
        if (!*ptr_alive_) {
            y3c::internal::ub_deleted(func);
        }
        return ptr_;
    }

    ptr(element_type *ptr, std::shared_ptr<bool> ptr_alive) noexcept
        : ptr_(ptr), ptr_alive_(ptr_alive) {}

  public:
    ptr(std::nullptr_t = nullptr) noexcept : ptr_(nullptr), ptr_alive_() {}

    template <typename U>
    ptr(const ptr<U> &ref) : ptr_(ref.ptr_), ptr_alive_(ref.ptr_alive_) {}
    template <typename U>
    ptr &operator=(const ptr<U> &ref) {
        ptr_ = ref.ptr_;
        ptr_alive_ = ref.ptr_alive_;
        return *this;
    }

    friend class wrap<element_type>;
    friend class shared_ptr<element_type>;

    wrap_ref<element_type> operator*() const {
        return wrap_ref<element_type>(ptr_unwrap("y3c::ptr::operator*()"),
                                      ptr_alive_);
    }
    element_type *operator->() const {
        return ptr_unwrap("y3c::ptr::operator->()");
    }
};
template <typename T>
using const_ptr = ptr<const T>;
template <typename T>
using ptr_const = const ptr<T>;
template <typename T>
using const_ptr_const = const const_ptr<T>;

template <typename T>
inline ptr<T> wrap<T>::operator&() {
    return ptr<T>(&base_, alive());
}
template <typename T>
inline ptr<const T> wrap<T>::operator&() const {
    return ptr<const T>(&base_, alive());
}

} // namespace y3c
