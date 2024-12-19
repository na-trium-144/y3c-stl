#pragma once
#include "y3c/terminate.h"
#include "y3c/life.h"
#include "y3c/typename.h"
#include <memory>

namespace y3c {
template <typename base_type>
class wrap;
template <typename base_type>
class wrap_auto;

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
 * * Tが E[N] の形の場合、
 *   * `operator[]` で y3c::wrap<E&> が得られる
 *   * `y3c::wrap<E*> にキャストできる
 *
 */
template <typename base_type>
class wrap {
    static_assert(!std::is_void<base_type>::value,
                  "y3c::wrap cannot have void value");
    static_assert(!std::is_const<base_type>::value,
                  "y3c::wrap cannot have const value");

    const std::string &type_name() const {
        return internal::get_type_name<wrap>();
    }

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
    wrap(Args &&...args) : base_{std::forward<Args>(args)...}, life_(&base_) {}
    template <typename V>
    wrap &operator=(V &&args) {
        base_ = std::forward<V>(args);
        return *this;
    }

    template <typename>
    friend class wrap;
    friend base_type &
    y3c::unwrap<base_type, nullptr>(wrap<base_type> &) noexcept;
    friend const base_type &
    y3c::unwrap<base_type, nullptr>(const wrap<base_type> &) noexcept;

    operator base_type &() noexcept { return this->base_; }
    operator const base_type &() const noexcept { return this->base_; }

    template <typename T = base_type,
              typename E = typename std::remove_extent<base_type>::type,
              typename std::enable_if<std::is_array<T>::value,
                                      std::nullptr_t>::type = nullptr,
              typename = internal::skip_trace_tag>
    wrap_auto<E> operator[](std::ptrdiff_t i) {
        static std::string func = type_name() + "::operator[]()";
        return wrap_auto<E>(life_.observer().assert_ptr(&base_[i], func),
                            life_.observer());
    }
    template <typename T = base_type,
              typename E = typename std::remove_extent<base_type>::type,
              typename std::enable_if<std::is_array<T>::value,
                                      std::nullptr_t>::type = nullptr,
              typename = internal::skip_trace_tag>
    wrap_auto<const E> operator[](std::ptrdiff_t i) const {
        static std::string func = type_name() + "::operator[]()";
        return wrap_auto<const E>(life_.observer().assert_ptr(&base_[i], func),
                                  life_.observer());
    }

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
 * * Tが E[N] の形の場合、
 *   * `operator[]` で y3c::wrap<E&> が得られる
 *   * `y3c::wrap<E*> にキャストできる
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

    const std::string &type_name() const {
        return internal::get_type_name<wrap>();
    }

    element_type *assert_ptr(const std::string &func,
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
        static std::string func = type_name() + "::operator=()";
        *assert_ptr(func) = std::forward<V>(args);
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
        static std::string func = type_name() + "::operator=()";
        static std::string func2 =
            "cast from " + other.type_name() + " to reference";
        *assert_ptr(func) = *other.assert_ptr(func2);
        return *this;
    }
    /*!
     * ムーブ代入
     * (ムーブ構築は無い)
     */
    template <typename = internal::skip_trace_tag>
    wrap &operator=(wrap &&other) {
        if (this != std::addressof(other)) {
            static std::string func = type_name() + "::operator=()";
            static std::string func2 =
                "cast from " + other.type_name() + " to reference";
            *assert_ptr(func) = std::move(*other.assert_ptr(func2));
        }
        return *this;
    }
    ~wrap() = default;

    template <typename T>
    friend class wrap;
    friend class wrap_auto<element_type>;
    friend element_type &y3c::unwrap<element_type>(const wrap<element_type &> &,
                                                   internal::skip_trace_tag);

    template <typename = internal::skip_trace_tag>
    operator element_type &() {
        static std::string func2 = "cast from " + type_name() + " to reference";
        return *assert_ptr(func2);
    }

    template <typename T = element_type,
              typename E = typename std::remove_extent<element_type>::type,
              typename std::enable_if<std::is_array<T>::value,
                                      std::nullptr_t>::type = nullptr,
              typename = internal::skip_trace_tag>
    wrap_auto<E> operator[](std::ptrdiff_t i) {
        static std::string func = type_name() + "::operator[]()";
        return wrap_auto<E>(&assert_ptr(func)[0][i], observer_);
    }

    wrap<element_type *> operator&() const noexcept {
        return wrap<element_type *>(ptr_, observer_);
    }
};

template <typename element_type>
element_type &unwrap(const wrap<element_type &> &wrapper,
                     internal::skip_trace_tag) {
    static std::string func = "y3c::unwrap(" + wrapper.type_name() + ")";
    return *wrapper.assert_ptr(func);
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

    virtual const std::string &type_name() const {
        return internal::get_type_name<wrap>();
    }

    element_type *assert_ptr(const std::string &func,
                             internal::skip_trace_tag = {}) const {
        return observer_.assert_ptr(ptr_, func);
    }

  public:
    wrap(element_type *ptr, internal::life_observer observer) noexcept
        : ptr_(ptr), observer_(observer), life_(&ptr_) {}

    template <typename T, std::size_t N,
              typename std::enable_if<!std::is_pointer<T>::value &&
                                          !std::is_reference<T>::value,
                                      std::nullptr_t>::type = nullptr>
    wrap(wrap<T[N]> &array)
        : ptr_(&array.base_[0]), observer_(array.life_.observer()),
          life_(&ptr_) {}
    template <typename T, std::size_t N, typename E = element_type,
              typename std::enable_if<!std::is_pointer<T>::value &&
                                          !std::is_reference<T>::value &&
                                          std::is_const<E>::value,
                                      std::nullptr_t>::type = nullptr>
    wrap(const wrap<T[N]> &array)
        : ptr_(&array.base_[0]), observer_(array.life_.observer()),
          life_(&ptr_) {}

    template <typename T, std::size_t N>
    wrap(const wrap<T (&)[N]> &array)
        : ptr_(&array.ptr_[0][0]), observer_(array.observer_), life_(&ptr_) {}

    wrap(std::nullptr_t = nullptr) noexcept
        : ptr_(nullptr), observer_(nullptr), life_(&ptr_) {}

    wrap(const wrap &other)
        : ptr_(other.ptr_), observer_(other.observer_), life_(&ptr_) {}
    wrap &operator=(const wrap &other) {
        ptr_ = other.ptr_;
        observer_ = other.observer_;
        return *this;
    }
    virtual ~wrap() = default;

    template <typename T>
    wrap(const wrap<T *> &ref)
        : ptr_(ref.ptr_), observer_(ref.observer_), life_(&ptr_) {}

    template <typename T>
    friend class wrap;
    template <typename T>
    friend class wrap_auto;
    friend element_type *
    y3c::unwrap<element_type>(const wrap<element_type *> &wrapper) noexcept;

    template <typename = internal::skip_trace_tag>
    wrap_auto<element_type> operator*() const {
        std::string func = type_name() + "::operator*()";
        return wrap_auto<element_type>(assert_ptr(func), observer_);
    }
    template <typename = internal::skip_trace_tag>
    element_type *operator->() const {
        std::string func = type_name() + "::operator->()";
        return assert_ptr(func);
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
        std::string func = type_name() + "::operator[]()";
        return wrap_auto<element_type>((*this + n).assert_ptr(func), observer_);
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

namespace internal {
template <typename element_type>
class contiguous_iterator : public wrap<element_type *> {

    const std::string *type_name_;
    const std::string &type_name() const override {
        // return internal::get_type_name<contiguous_iterator>();
        return *type_name_;
    }

  public:
    contiguous_iterator(element_type *ptr_, internal::life_observer observer,
                        const std::string *type_name) noexcept
        : wrap<element_type *>(ptr_, observer), type_name_(type_name) {}

    template <typename T, typename std::enable_if<
                              std::is_same<const T, element_type>::value,
                              std::nullptr_t>::type = nullptr>
    contiguous_iterator(const contiguous_iterator<T> &other)
        : wrap<element_type *>(other.ptr_, other.observer_),
          type_name_(other.type_name_) {}

    contiguous_iterator(const contiguous_iterator &) = default;
    contiguous_iterator &operator=(const contiguous_iterator &) = default;
    ~contiguous_iterator() override = default;

    const life_observer &get_observer_() const { return this->observer_; }

    contiguous_iterator &operator++() {
        ++this->ptr_;
        return *this;
    }
    contiguous_iterator operator++(int) {
        contiguous_iterator copy = *this;
        ++this->ptr_;
        return copy;
    }
    contiguous_iterator &operator--() {
        --this->ptr_;
        return *this;
    }
    contiguous_iterator operator--(int) {
        contiguous_iterator copy = *this;
        --this->ptr_;
        return copy;
    }
    contiguous_iterator &operator+=(std::ptrdiff_t n) {
        this->ptr_ += n;
        return *this;
    }
    contiguous_iterator &operator-=(std::ptrdiff_t n) {
        this->ptr_ -= n;
        return *this;
    }
    contiguous_iterator operator+(std::ptrdiff_t n) const {
        return contiguous_iterator(this->ptr_ + n, this->observer_, type_name_);
    }
    contiguous_iterator operator-(std::ptrdiff_t n) const {
        return contiguous_iterator(this->ptr_ - n, this->observer_, type_name_);
    }
};
} // namespace internal


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

} // namespace y3c
