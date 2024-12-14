#pragma once
#include "y3c/internal.h"
#include <memory>
#include <cassert>

Y3C_NS_BEGIN
namespace internal {
enum class ptr_type_enum {
    ptr,
    array_iterator,
};
}

template <typename T>
class wrap;
template <typename T>
class wrap_ref;
template <typename T>
class wrap_auto;
template <typename T,
          internal::ptr_type_enum ptr_type = internal::ptr_type_enum::ptr>
class ptr;
template <typename T>
class shared_ptr;
template <class T, std::size_t N>
class array;

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
    static_assert(!std::is_void<base_type>::value,
                  "y3c::wrap cannot have void value");
    static_assert(!std::is_const<base_type>::value,
                  "y3c::wrap cannot have const value");
    static_assert(!std::is_reference<base_type>::value,
                  "y3c::wrap cannot have reference value");

    base_type base_;
    mutable std::shared_ptr<bool> alive_;

  protected:
    const std::shared_ptr<bool> &alive() const {
        if (!alive_) {
            alive_ = std::make_shared<bool>(true);
        }
        return alive_;
    }

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
    wrap &operator=(Args &&args) {
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

    template <typename>
    friend class wrap_ref;
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
        "y3c::wrap_ref cannot have reference to reference");

    element_type *begin_;
    std::size_t size_;
    element_type *ptr_;
    std::shared_ptr<bool> range_alive_;

  protected:
    element_type *ptr_unwrap(const char *func) const {
        if (!ptr_) {
            y3c::internal::undefined_behavior(func, y3c::msg::access_nullptr());
        }
        assert(range_alive_);
        if (!*range_alive_) {
            y3c::internal::undefined_behavior(func, y3c::msg::access_deleted());
        }
        if (ptr_ - begin_ < 0 || ptr_ - begin_ >= size_) {
            y3c::internal::undefined_behavior(
                func, y3c::msg::out_of_range(
                          size_, static_cast<long long>(ptr_ - begin_)));
        }
        return ptr_;
    }

    wrap_ref(element_type *begin, std::size_t size, element_type *ptr,
             std::shared_ptr<bool> range_alive) noexcept
        : begin_(begin), size_(size), ptr_(ptr), range_alive_(range_alive) {}
    wrap_ref(element_type *ptr, std::shared_ptr<bool> ptr_alive) noexcept
        : wrap_ref(ptr, 1, ptr, std::move(ptr_alive)) {}

    wrap_ref(std::nullptr_t = nullptr) noexcept
        : begin_(nullptr), size_(0), ptr_(nullptr), range_alive_() {}

  public:
    template <typename U>
    wrap_ref(wrap<U> &wrapper)
        : begin_(&wrapper.base_), size_(1), ptr_(&wrapper.base_),
          range_alive_(wrapper.alive()) {}

    template <typename U>
    wrap_ref(const wrap_ref<U> &ref) noexcept
        : begin_(ref.begin_), size_(ref.size_), ptr_(ref.ptr_),
          range_alive_(ref.range_alive_) {}

    template <typename Args>
    wrap_ref &operator=(Args &&args) {
        *ptr_unwrap("y3c::wrap_ref::operator=()") = std::forward<Args>(args);
        return *this;
    }

    template <typename U>
    wrap_ref(const wrap_auto<U> &auto_ref) noexcept;

    /*!
     * コピー構築の場合は参照としてコピーする
     */
    wrap_ref(const wrap_ref &) noexcept = default;
    /*!
     * 参照のムーブはコピーと同じ
     */
    wrap_ref(wrap_ref &&other) noexcept
        : wrap_ref(static_cast<const wrap_ref &>(other)) {}
    /*!
     * コピー代入しようとした場合は値のコピーにする
     */
    wrap_ref &operator=(const wrap_ref &other) {
        *ptr_unwrap("y3c::wrap_ref::operator=()") =
            *other.ptr_unwrap("cast from y3c::wrap_ref to reference");
        return *this;
    }
    wrap_ref &operator=(wrap_ref &&other) {
        T &val = *other.ptr_unwrap("cast from y3c::wrap_ref to reference");
        if (this != &other) {
            *ptr_unwrap("y3c::wrap_ref::operator=()") = std::move(val);
        }
        return *this;
    }
    ~wrap_ref() = default;

    friend class wrap_auto<element_type>;
    friend element_type &
    y3c::unwrap<element_type>(const wrap_ref<element_type> &);

    operator element_type &() {
        return *ptr_unwrap("cast from y3c::wrap_ref to reference");
    }

    ptr<element_type> operator&() const noexcept;
};

template <typename T>
using const_wrap_ref = wrap_ref<const T>;

template <typename T>
T &unwrap(const wrap_ref<T> &wrapper) {
    return *wrapper.ptr_unwrap("y3c::unwrap()");
}

/*!
 * 値の参照を返す関数が、wrap_ref<T> を返す代わりにこれを返すことで、
 * ユーザーがそれをさらに明示的に wrap_ref<T> にキャストすれば元の参照を返すが、
 * autoで受け取るなどwrap_refにならなかった場合は参照ではなく値をコピーしたものとしてふるまう
 *
 * \todo
 * autoで受け取ったあとしばらく値を変更せずにあとでrefに変換した場合も元の値を参照することになるが、
 * それは直感的ではない
 *
 * ↑ しばらくしてから の定義ってなんだ?
 *
 */
template <typename T>
class wrap_auto : public wrap<typename std::remove_const<T>::type> {
    using base_type = typename std::remove_const<T>::type;
    using element_type = T;
    element_type *begin_;
    std::size_t size_;
    element_type *ptr_;
    std::shared_ptr<bool> range_alive_;

    void clear_ref() {
        begin_ = &this->unwrap();
        size_ = 1;
        ptr_ = &this->unwrap();
        range_alive_ = this->alive();
    }

    wrap_auto(element_type *begin, std::size_t size, element_type *ptr,
              std::shared_ptr<bool> range_alive) noexcept
        : wrap<base_type>(*ptr), begin_(begin), size_(size), ptr_(ptr),
          range_alive_(range_alive) {}
    wrap_auto(element_type *ptr, std::shared_ptr<bool> ptr_alive) noexcept
        : wrap_auto(ptr, 1, ptr, std::move(ptr_alive)) {}

  public:
    wrap_auto() = delete;
    wrap_auto(const wrap_auto &) = default;
    wrap_auto(wrap_auto &&) = default;
    ~wrap_auto() = default;

    template <typename>
    friend class wrap_ref;
    template <typename, internal::ptr_type_enum>
    friend class ptr;
    friend class shared_ptr<element_type>;
    template <typename, std::size_t>
    friend class array;

    /*!
     * 値が代入されたら元の参照は破棄し値のコピーだけが利用可能になる
     */
    template <typename Args>
    wrap_auto &operator=(Args &&args) {
        clear_ref();
        this->wrap<base_type>::operator=(std::forward<Args>(args));
        return *this;
    }
    wrap_auto &operator=(const wrap_auto &other) {
        clear_ref();
        if (this != &other) {
            this->wrap<base_type>::operator=(other);
        }
        return *this;
    }
    wrap_auto &operator=(wrap_auto &&other) {
        clear_ref();
        if (this != &other) {
            this->wrap<base_type>::operator=(other);
        }
        return *this;
    }

    ptr<element_type> operator&() const noexcept;
};
template <typename T>
template <typename U>
wrap_ref<T>::wrap_ref(const wrap_auto<U> &auto_ref) noexcept
    : wrap_ref(auto_ref.begin_, auto_ref.size_, auto_ref.ptr_,
               auto_ref.range_alive_) {}

template <typename T>
T &unwrap(const wrap_auto<T> &wrapper) {
    return unwrap(wrap_ref<T>(wrapper));
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
 * ptr_const<T> = const ptr<T> = T *const
 *
 * const_ptr_const<T> = const ptr<const T> = const T *const
 *
 */
template <typename T, internal::ptr_type_enum ptr_type>
class ptr : public wrap<T *> {
    using element_type = T;
    static_assert(
        !std::is_reference<typename std::remove_const<T>::type>::value,
        "y3c::ptr cannot have pointer to reference");

    static std::string func_name_() {
        switch (ptr_type) {
        case internal::ptr_type_enum::array_iterator:
            return "y3c::array::iterator";
        default:
            return "y3c::ptr";
        }
    }

  protected:
    element_type *begin_;
    std::size_t size_;
    std::shared_ptr<bool> range_alive_;

    element_type *ptr_unwrap(const char *func) const {
        if (!this->unwrap()) {
            y3c::internal::undefined_behavior(func, y3c::msg::access_nullptr());
        }
        assert(range_alive_);
        if (!*range_alive_) {
            y3c::internal::undefined_behavior(func, y3c::msg::access_deleted());
        }
        if (this->unwrap() - begin_ < 0 || this->unwrap() - begin_ >= size_) {
            y3c::internal::undefined_behavior(
                func,
                y3c::msg::out_of_range(
                    size_, static_cast<long long>(this->unwrap() - begin_)));
        }
        return this->unwrap();
    }

    ptr(element_type *begin, std::size_t size, element_type *ptr_,
        std::shared_ptr<bool> range_alive) noexcept
        : wrap<T *>(ptr_), begin_(begin), size_(size),
          range_alive_(range_alive) {}
    ptr(element_type *ptr_, std::shared_ptr<bool> ptr_alive) noexcept
        : ptr(ptr_, 1, ptr_, std::move(ptr_alive)) {}

  public:
    ptr(std::nullptr_t = nullptr) noexcept
        : wrap<T *>(nullptr), begin_(nullptr), size_(0), range_alive_() {}

    template <typename U>
    ptr(const ptr<U, ptr_type> &ref)
        : wrap<T *>(unwrap(ref)), begin_(ref.begin_), size_(ref.size_),
          range_alive_(ref.range_alive_) {}
    template <typename U>
    ptr &operator=(const ptr<U, ptr_type> &ref) {
        this->wrap<T *>::operator=(unwrap(ref));
        begin_ = ref.begin_;
        size_ = ref.size_;
        range_alive_ = ref.range_alive_;
        return *this;
    }

    friend class wrap<element_type>;
    friend class wrap_ref<element_type>;
    friend class shared_ptr<element_type>;
    template <typename, internal::ptr_type_enum>
    friend class ptr;
    template <typename, std::size_t>
    friend class array;

    wrap_auto<element_type> operator*() const {
        static std::string func_name = func_name_() + "::operator*()";
        return wrap_auto<element_type>(
            begin_, size_, ptr_unwrap(func_name.c_str()), range_alive_);
    }
    element_type *operator->() const {
        static std::string func_name = func_name_() + "::operator->()";
        return ptr_unwrap(func_name.c_str());
    }

    ptr &operator++() {
        ++this->unwrap();
        return *this;
    }
    ptr operator++(int) {
        ptr copy = *this;
        ++this->unwrap();
        return copy;
    }
    ptr &operator--() {
        --this->unwrap();
        return *this;
    }
    ptr operator--(int) {
        ptr copy = *this;
        --this->unwrap();
        return copy;
    }
    ptr &operator+=(std::ptrdiff_t n) {
        this->unwrap() += n;
        return *this;
    }
    ptr &operator-=(std::ptrdiff_t n) {
        this->unwrap() -= n;
        return *this;
    }
    ptr operator+(std::ptrdiff_t n) const {
        return ptr(begin_, size_, this->unwrap() + n, range_alive_);
    }
    ptr operator-(std::ptrdiff_t n) const {
        return ptr(begin_, size_, this->unwrap() - n, range_alive_);
    }
    std::ptrdiff_t operator-(const ptr &other) const {
        return this->unwrap() - other.ptr_;
    }
    element_type &operator[](std::ptrdiff_t n) const {
        static std::string func_name = func_name_() + "::operator[]()";
        return *(*this + n).ptr_unwrap(func_name.c_str());
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
template <typename T>
inline ptr<T> wrap_ref<T>::operator&() const noexcept {
    return ptr<T>(begin_, size_, ptr_, range_alive_);
}
template <typename T>
inline ptr<T> wrap_auto<T>::operator&() const noexcept {
    return &wrap_ref<T>(*this);
}

Y3C_NS_END
