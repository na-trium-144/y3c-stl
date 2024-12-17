#pragma once
#include "y3c/terminate.h"
#include "y3c/life.h"
#include <memory>
#include <cassert>

Y3C_NS_BEGIN
template <typename base_type>
class wrap;

template <typename base_type,
          typename std::enable_if<!std::is_pointer<base_type>::value &&
                                      !std::is_reference<base_type>::value,
                                  std::nullptr_t>::type = nullptr>
base_type &unwrap(wrap<base_type> &wrapper) noexcept;
template <typename base_type,
          typename std::enable_if<!std::is_pointer<base_type>::value &&
                                      !std::is_reference<base_type>::value,
                                  std::nullptr_t>::type = nullptr>
const base_type &unwrap(const wrap<base_type> &wrapper) noexcept;
template <typename element_type>
element_type &unwrap(const wrap<element_type &> &wrapper,
                     internal::skip_trace_tag = {});
template <typename element_type>
element_type *unwrap(const wrap<element_type *> &wrapper) noexcept;

/*!
 * \brief 値型wrap: base_type型のデータと、このデータの生存状態を管理するクラス
 *
 * * `operator&` で y3c::wrap<T*> が得られる
 * * y3c::wrap<T&> にキャストできる
 *
 */
template <typename base_type>
class wrap {
    static_assert(!std::is_void<base_type>::value,
                  "y3c::wrap cannot have void value");
    static_assert(!std::is_const<base_type>::value,
                  "y3c::wrap cannot have const value");

  protected:
    base_type base_;
    internal::life life_;

  public:
    wrap() : base_(), life_(&base_) {}
    wrap(const wrap &other) : base_(other.base_), life_(&base_) {}
    wrap(wrap &&other) : base_(std::move(other.base_)), life_(&base_) {}
    wrap &operator=(const wrap &other) {
        base_ = other.base_;
        return *this;
    }
    wrap &operator=(wrap &&other) {
        base_ = std::move(other.base_);
        return *this;
    }
    ~wrap() = default;

    template <typename... Args>
    wrap(Args &&...args) : base_(std::forward<Args>(args)...), life_(&base_) {}
    template <typename V>
    wrap &operator=(V &&args) {
        base_ = std::forward<V>(args);
        return *this;
    }

    template <typename>
    friend class wrap;
    template <typename T,
              typename std::enable_if<!std::is_pointer<T>::value &&
                                          !std::is_reference<T>::value,
                                      std::nullptr_t>::type>
    friend T &y3c::unwrap(wrap<T> &) noexcept;
    template <typename T,
              typename std::enable_if<!std::is_pointer<T>::value &&
                                          !std::is_reference<T>::value,
                                      std::nullptr_t>::type>
    friend const T &y3c::unwrap(const wrap<T> &) noexcept;

    operator base_type &() noexcept { return this->base_; }
    operator const base_type &() const noexcept { return this->base_; }

    wrap<base_type *> operator&() {
        return wrap<base_type *>(&base_, life_.observer());
    }
    wrap<const base_type *> operator&() const {
        return wrap<const base_type *>(&base_, life_.observer());
    }
};
template <typename base_type,
          typename std::enable_if<!std::is_pointer<base_type>::value &&
                                      !std::is_reference<base_type>::value,
                                  std::nullptr_t>::type>
base_type &unwrap(wrap<base_type> &wrapper) noexcept {
    return wrapper.base_;
}
template <typename base_type,
          typename std::enable_if<!std::is_pointer<base_type>::value &&
                                      !std::is_reference<base_type>::value,
                                  std::nullptr_t>::type>
const base_type &unwrap(const wrap<base_type> &wrapper) noexcept {
    return wrapper.base_;
}

/*!
 * 値の参照を返す関数が、y3c::wrap<T&> を返す代わりにこれを返すことで、
 * ユーザーがそれをさらに明示的に y3c::wrap<T&>
 * にキャストすれば元の参照を返すが、
 * autoで受け取るなどwrap<T&>にならなかった場合は参照ではなく値をコピーしたものとしてふるまう
 *
 * * `operator&` で y3c::wrap<T*> が得られる
 *
 * \todo
 * autoで受け取ったあとしばらく値を変更せずにあとでrefに変換した場合も元の値を参照することになるが、
 * それは直感的ではない
 *
 * ↑ しばらくしてから の定義ってなんだ?
 *
 */
template <typename element_type>
class wrap_auto : public wrap<typename std::remove_const<element_type>::type> {
    using base_type = typename std::remove_const<element_type>::type;
    element_type *ptr_;
    internal::life_observer observer_;

    void clear_ref() {
        ptr_ = &this->base_;
        observer_ = this->life_.observer();
    }

  public:
    wrap_auto(element_type *ptr, internal::life_observer observer) noexcept
        : wrap<base_type>(*ptr), ptr_(ptr), observer_(observer) {}
    wrap_auto(const wrap_auto &) = default;
    wrap_auto(wrap_auto &&) = default;
    ~wrap_auto() = default;

    template <typename T>
    friend class wrap;

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
        if (this != std::addressof(other)) {
            this->wrap<base_type>::operator=(other);
        }
        return *this;
    }
    wrap_auto &operator=(wrap_auto &&other) {
        clear_ref();
        if (this != std::addressof(other)) {
            this->wrap<base_type>::operator=(other);
        }
        return *this;
    }

    wrap<element_type *> operator&() const noexcept {
        return wrap<element_type *>(ptr_, observer_);
    }
};

/*!
 * \brief 参照型wrap: element_type型のデータへの参照を持つクラス
 *
 * * `T&`へのキャスト時と、`y3c::unwrap()`
 * 時に参照先が生きているかなどのチェックを行う。
 * キャストとy3c::unwrapでチェックが入るのは (今のところ) wrap_ref のみ。
 * * y3c::wrap<remove_const_t<T>> の左辺値からキャストできる
 * * `operator&` で y3c::wrap<T*> が得られる
 *
 * \sa wrap_ref, const_wrap_ref
 */
template <typename element_type>
class wrap<element_type &> {
    static_assert(
        !std::is_void<typename std::remove_const<element_type>::type>::value,
        "y3c::wrap cannot have reference to void");

    element_type *ptr_;
    internal::life_observer observer_;

    element_type *assert_ptr(const char *func,
                             internal::skip_trace_tag = {}) const {
        return observer_.assert_ptr(ptr_, func);
    }

  public:
    wrap(element_type *ptr, internal::life_observer observer) noexcept
        : ptr_(ptr), observer_(observer) {}

    /*!
     * \brief 値型wrapの参照
     */
    template <typename T,
              typename std::enable_if<!std::is_pointer<T>::value &&
                                          !std::is_reference<T>::value,
                                      std::nullptr_t>::type = nullptr>
    wrap(wrap<T> &wrapper)
        : ptr_(&wrapper.base_), observer_(wrapper.life_.observer()) {}

    /*!
     * \brief 値型wrapの参照
     */
    template <typename T, typename E = element_type,
              typename std::enable_if<!std::is_pointer<T>::value &&
                                          !std::is_reference<T>::value &&
                                          std::is_const<E>::value,
                                      std::nullptr_t>::type = nullptr>
    wrap(const wrap<T> &wrapper)
        : ptr_(&wrapper.base_), observer_(wrapper.life_.observer()) {}

    /*!
     * \brief 参照型wrapのコピー
     */
    template <typename T>
    wrap(const wrap<T &> &ref) noexcept
        : ptr_(ref.ptr_), observer_(ref.observer_) {}

    /*!
     * \brief 参照先への代入
     */
    template <typename V, typename = internal::skip_trace_tag>
    wrap &operator=(V &&args) {
        *assert_ptr("y3c::wrap_ref::operator=()") = std::forward<V>(args);
        return *this;
    }

    template <typename T>
    wrap(const wrap_auto<T> &auto_ref) noexcept
        : wrap(auto_ref.ptr_, auto_ref.observer_) {}

    /*!
     * コピー構築の場合は参照としてコピーする
     */
    wrap(const wrap &) noexcept = default;
    /*!
     * コピー代入しようとした場合は値のコピーにする
     */
    template <typename = internal::skip_trace_tag>
    wrap &operator=(const wrap &other) {
        *assert_ptr("y3c::wrap_ref::operator=()") =
            *other.assert_ptr("cast from y3c::wrap to reference");
        return *this;
    }
    /*!
     * ムーブ代入
     * (ムーブ構築は無い)
     */
    template <typename = internal::skip_trace_tag>
    wrap &operator=(wrap &&other) {
        if (this != std::addressof(other)) {
            *assert_ptr("y3c::wrap_ref::operator=()") = std::move(
                *other.assert_ptr("cast from y3c::wrap to reference"));
        }
        return *this;
    }
    ~wrap() = default;

    template <typename T>
    friend class wrap;
    friend class wrap_auto<element_type>;
    friend element_type &y3c::unwrap(const wrap<element_type &> &,
                                     internal::skip_trace_tag);

    template <typename = internal::skip_trace_tag>
    operator element_type &() {
        return *assert_ptr("cast from y3c::wrap_ref to reference");
    }

    wrap<element_type *> operator&() const noexcept {
        return wrap<element_type *>(ptr_, observer_);
    }
};

template <typename element_type>
element_type &unwrap(const wrap<element_type &> &wrapper,
                     internal::skip_trace_tag) {
    return *wrapper.assert_ptr("y3c::unwrap()");
}

template <typename element_type>
element_type &unwrap(const wrap_auto<element_type> &wrapper,
                     internal::skip_trace_tag = {}) {
    return unwrap(wrap<element_type &>(wrapper));
}

/*!
 * \brief ポインタ型wrap: element_type型のデータへのポインタと、
 * このポインタ自体の生存状態を管理するクラス
 *
 * * `operator*`, `operator->`, `operator[]`
 * 時にnullptrチェックと範囲外アクセスチェックをする
 * 使用時にnullptrでないかと範囲外でないかのチェックを行う。
 * * また `operator*`, `operator[]` が返す参照はラップ済み (y3c::wrap_auto)
 * * `T*` にキャストできるがその場合チェックされないので注意
 * * `operator&` で y3c::wrap<T**> が得られる
 *
 * \sa ptr, const_ptr, ptr_const, const_ptr_const
 */
template <typename element_type>
class wrap<element_type *> {
  protected:
    element_type *ptr_;
    internal::life_observer observer_;
    internal::life life_;

    element_type *assert_ptr(const char *func,
                             internal::skip_trace_tag = {}) const {
        return observer_.assert_ptr(ptr_, func);
    }

  public:
    wrap(element_type *ptr, internal::life_observer observer) noexcept
        : ptr_(ptr), observer_(observer), life_(&ptr_) {}

    wrap(std::nullptr_t = nullptr) noexcept
        : ptr_(nullptr), observer_(nullptr), life_(&ptr_) {}

    wrap(const wrap &other)
        : ptr_(other.ptr_), observer_(other.observer_), life_(&ptr_) {}
    wrap &operator=(const wrap &other) {
        ptr_ = other.ptr_;
        observer_ = other.observer_;
        return *this;
    }
    ~wrap() = default;

    template <typename T>
    wrap(const wrap<T *> &ref)
        : ptr_(ref.ptr_), observer_(ref.observer_), life_(&ptr_) {}
    template <typename T>
    wrap &operator=(const wrap<T> &ref) {
        ptr_ = ref.ptr_;
        observer_ = ref.observer_;
        return *this;
    }

    template <typename T>
    friend class wrap;
    template <typename T>
    friend class wrap_auto;
    friend element_type *
    y3c::unwrap(const wrap<element_type *> &wrapper) noexcept;

    template <typename = internal::skip_trace_tag>
    wrap_auto<element_type> operator*() const {
        return wrap_auto<element_type>(assert_ptr("y3c::wrap::operator*()"),
                                       observer_);
    }
    template <typename = internal::skip_trace_tag>
    element_type *operator->() const {
        return assert_ptr("y3c::wrap::operator->()");
    }

    wrap &operator++() {
        ++ptr_;
        return *this;
    }
    wrap operator++(int) {
        wrap copy = *this;
        ++ptr_;
        return copy;
    }
    wrap &operator--() {
        --ptr_;
        return *this;
    }
    wrap operator--(int) {
        wrap copy = *this;
        --ptr_;
        return copy;
    }
    wrap &operator+=(std::ptrdiff_t n) {
        ptr_ += n;
        return *this;
    }
    wrap &operator-=(std::ptrdiff_t n) {
        ptr_ -= n;
        return *this;
    }
    wrap operator+(std::ptrdiff_t n) const { return wrap(ptr_ + n, observer_); }
    wrap operator-(std::ptrdiff_t n) const { return wrap(ptr_ - n, observer_); }
    std::ptrdiff_t operator-(const wrap &other) const {
        return ptr_ - other.ptr_;
    }
    template <typename = internal::skip_trace_tag>
    wrap_auto<element_type> operator[](std::ptrdiff_t n) const {
        return wrap_auto<element_type>(
            (*this + n).assert_ptr("y3c::wrap::operator[]()"), observer_);
    }

    operator element_type *() const noexcept { return ptr_; }

    wrap<element_type **> operator&() {
        return wrap<element_type **>(&ptr_, life_.observer());
    }
    wrap<element_type *const *> operator&() const {
        return wrap<element_type *const *>(&ptr_, life_.observer());
    }
};
template <typename element_type>
element_type *unwrap(const wrap<element_type *> &wrapper) noexcept {
    return wrapper.ptr_;
}

template <typename element_type>
using wrap_ref = wrap<element_type &>;
template <typename element_type>
using const_wrap_ref = wrap<const element_type &>;

template <typename element_type>
using ptr = wrap<element_type *>;
template <typename element_type>
using const_ptr = wrap<const element_type *>;
template <typename element_type>
using ptr_const = const wrap<element_type *>;
template <typename element_type>
using const_ptr_const = const wrap<const element_type *>;

Y3C_NS_END
