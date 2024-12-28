#pragma once
#include "y3c/terminate.h"
#include "y3c/life.h"
#include "y3c/typename.h"
#include <memory>

namespace y3c {
template <typename base_type>
class wrap;
template <typename element_type>
element_type &unwrap(const wrap<element_type &> &wrapper,
                     internal::skip_trace_tag = {});

/*!
 * \brief 値型wrap: base_type型のデータと、このデータの生存状態を管理するクラス
 *
 * * キャストするか unwrap() することでbase_type型(の参照)に戻せる。
 * * `&` をつけるとbase_typeのポインタをラップした y3c::wrap<base_type*>
 * が得られる
 * * base_typeが E[N] の形の場合、
 *   * `[ ]` で要素アクセスできる (参照をラップした y3c::wrap<E&> が得られる)
 *   * E のポインタをラップした y3c::wrap<E*> にもキャストできる
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
    /*!
     * \brief デフォルト構築
     */
    wrap() : base_(), life_(&base_) {}
    /*!
     * \brief コピーコンストラクタ
     */
    wrap(const wrap &other) : base_(other.base_), life_(&base_) {}
    /*!
     * \brief ムーブコンストラクタ
     *
     * * このデータの生存状態はムーブされない。
     */
    wrap(wrap &&other) : base_(std::move(other.base_)), life_(&base_) {}
    /*!
     * \brief コピー代入
     */
    wrap &operator=(const wrap &other) {
        base_ = other.base_;
        return *this;
    }
    /*!
     * \brief ムーブ代入
     *
     * * このデータの生存状態はムーブされない。
     */
    wrap &operator=(wrap &&other) {
        base_ = std::move(other.base_);
        return *this;
    }
    ~wrap() = default;

    /*!
     * \brief コンストラクタ引数はそのままbase_typeのコンストラクタに渡される。
     */
    template <typename... Args>
    wrap(Args &&...args) : base_{std::forward<Args>(args)...}, life_(&base_) {}
    /*!
     * \brief 引数はそのままbase_typeのoperator=()に渡される。
     */
    template <typename V>
    wrap &operator=(V &&args) {
        base_ = std::forward<V>(args);
        return *this;
    }

    template <typename>
    friend class wrap;

    /*!
     * \brief base_type へのキャスト
     */
    operator base_type &() noexcept { return this->base_; }
    /*!
     * \brief const base_type へのキャスト
     */
    operator const base_type &() const noexcept { return this->base_; }

    /*!
     * \brief base_typeが E[N] の形の場合、要素への参照を返す
     *
     * * インデックスが範囲外の場合terminateする。
     *
     */
    template <typename T = base_type,
              typename E = typename std::remove_extent<base_type>::type,
              typename std::enable_if<std::is_array<T>::value,
                                      std::nullptr_t>::type = nullptr,
              typename = internal::skip_trace_tag>
    wrap<E &> operator[](std::ptrdiff_t i) {
        static std::string func = type_name() + "::operator[]()";
        return wrap<E &>(life_.observer().assert_ptr(&base_[i], func),
                         life_.observer());
    }
    /*!
     * \brief base_typeが E[N] の形の場合、要素への参照を返す(const)
     *
     * * インデックスが範囲外の場合terminateする。
     *
     */
    template <typename T = base_type,
              typename E = typename std::remove_extent<base_type>::type,
              typename std::enable_if<std::is_array<T>::value,
                                      std::nullptr_t>::type = nullptr,
              typename = internal::skip_trace_tag>
    wrap<const E &> operator[](std::ptrdiff_t i) const {
        static std::string func = type_name() + "::operator[]()";
        return wrap<const E &>(life_.observer().assert_ptr(&base_[i], func),
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
                                  std::nullptr_t>::type = nullptr>
base_type &unwrap(wrap<base_type> &wrapper) noexcept {
    return static_cast<base_type &>(wrapper);
}
template <typename base_type,
          typename std::enable_if<!std::is_pointer<base_type>::value &&
                                      !std::is_reference<base_type>::value,
                                  std::nullptr_t>::type = nullptr>
const base_type &unwrap(const wrap<base_type> &wrapper) noexcept {
    return static_cast<const base_type &>(wrapper);
}

/*!
 * \brief 参照型wrap: element_type型のデータへの参照を持つクラス
 *
 * * キャストするか unwrap() することでbase_type型の参照に戻せるが、
 * その際参照先が生きているかどうかのチェックが入る。
 * * `&` をつけるとelement_typeのポインタをラップした y3c::wrap<element_type*>
 * が得られる
 * * element_typeが E[N] の形の場合、
 *   * `[ ]` で要素アクセスできる (参照をラップした y3c::wrap<E&> が得られる)
 *   * E のポインタをラップした y3c::wrap<E*> にもキャストできる
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
     * \brief 値型wrapの参照(const)
     */
    template <typename T, typename E = element_type,
              typename std::enable_if<!std::is_pointer<T>::value &&
                                          !std::is_reference<T>::value &&
                                          std::is_const<E>::value,
                                      std::nullptr_t>::type = nullptr>
    wrap(const wrap<T> &wrapper)
        : ptr_(&wrapper.base_), observer_(wrapper.life_.observer()) {}

    /*!
     * \brief 参照型wrapのコピー: 同じものを参照する
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

    /*!
     * \brief コピーコンストラクタ: 同じものを参照する
     */
    wrap(const wrap &) noexcept = default;
    /*!
     * \brief コピー代入: 値を参照先へコピーする
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
     * \brief ムーブ代入
     *
     * (ムーブコンストラクタは無い)
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
    friend element_type &y3c::unwrap<element_type>(const wrap<element_type &> &,
                                                   internal::skip_trace_tag);

    /*!
     * \brief element_type へのキャスト
     *
     * * 参照先が生きていない場合terminateする。
     */
    template <typename = internal::skip_trace_tag>
    operator element_type &() {
        static std::string func2 = "cast from " + type_name() + " to reference";
        return *assert_ptr(func2);
    }

    /*!
     * \brief element_typeが E[N] の形の場合、要素への参照を返す
     *
     * * インデックスが範囲外の場合terminateする。
     *
     */
    template <typename T = element_type,
              typename E = typename std::remove_extent<element_type>::type,
              typename std::enable_if<std::is_array<T>::value,
                                      std::nullptr_t>::type = nullptr,
              typename = internal::skip_trace_tag>
    wrap<E &> operator[](std::ptrdiff_t i) {
        static std::string func = type_name() + "::operator[]()";
        return wrap<E &>(&assert_ptr(func)[0][i], observer_);
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

/*!
 * \brief ポインタ型wrap: element_type型のデータへのポインタと、
 * このポインタ自体の生存状態を管理するクラス
 *
 * * 他のy3cライブラリのクラスに `&` をつけることで、
 * このクラスでラップされたポインタが得られる。
 *   * 生ポインタなどから wrap<element_type*> へ変換することはできない。
 * * element_type* にキャストまたは unwrap()
 * することで生ポインタに変換できるが、
 * その場合参照先の生存チェックはされないので注意
 * * `&` をつけると y3c::wrap<element_type**> が得られる
 *
 * \sa ptr, const_ptr, ptr_const, const_ptr_const
 */
template <typename element_type>
class wrap<element_type *> {
    element_type *ptr_;
    internal::life_observer observer_;
    internal::life life_;

    const std::string &type_name() const {
        return internal::get_type_name<wrap>();
    }

    element_type *assert_ptr(const std::string &func,
                             internal::skip_trace_tag = {}) const {
        return observer_.assert_ptr(ptr_, func);
    }

  public:
    wrap(element_type *ptr, internal::life_observer observer) noexcept
        : ptr_(ptr), observer_(observer), life_(&ptr_) {}

    /*!
     * \brief 配列型wrapからのキャスト
     */
    template <typename T, std::size_t N,
              typename std::enable_if<!std::is_pointer<T>::value &&
                                          !std::is_reference<T>::value,
                                      std::nullptr_t>::type = nullptr>
    wrap(wrap<T[N]> &array)
        : ptr_(&array.base_[0]), observer_(array.life_.observer()),
          life_(&ptr_) {}
    /*!
     * \brief 配列型wrapからのキャスト(const)
     */
    template <typename T, std::size_t N, typename E = element_type,
              typename std::enable_if<!std::is_pointer<T>::value &&
                                          !std::is_reference<T>::value &&
                                          std::is_const<E>::value,
                                      std::nullptr_t>::type = nullptr>
    wrap(const wrap<T[N]> &array)
        : ptr_(&array.base_[0]), observer_(array.life_.observer()),
          life_(&ptr_) {}

    /*!
     * \brief 配列の参照型wrapからのキャスト
     */
    template <typename T, std::size_t N>
    wrap(const wrap<T (&)[N]> &array)
        : ptr_(&array.ptr_[0][0]), observer_(array.observer_), life_(&ptr_) {}

    /*!
     * \brief デフォルトコンストラクタ: nullptrを指す
     */
    wrap(std::nullptr_t = nullptr) noexcept
        : ptr_(nullptr), observer_(nullptr), life_(&ptr_) {}

    /*!
     * \brief ポインタのコピー
     */
    wrap(const wrap &other)
        : ptr_(other.ptr_), observer_(other.observer_), life_(&ptr_) {}
    /*!
     * \brief ポインタのコピー
     */
    wrap &operator=(const wrap &other) {
        ptr_ = other.ptr_;
        observer_ = other.observer_;
        return *this;
    }
    ~wrap() = default;

    /*!
     * \brief 別の型のポインタからの変換
     */
    template <typename T>
    wrap(const wrap<T *> &ref)
        : ptr_(ref.ptr_), observer_(ref.observer_), life_(&ptr_) {}

    template <typename T>
    friend class wrap;

    /*!
     * \brief 要素アクセス
     *
     * * 参照先が生きていない場合terminateする。
     * * 返り値は wrap<element_type&> でラップされた状態で返る。
     *   * 元のelement_type型に戻すにはさらにキャストするか unwrap() が必要。
     */
    template <typename E = element_type, typename = internal::skip_trace_tag>
    wrap<E &> operator*() const {
        static std::string func = type_name() + "::operator*()";
        return wrap<E &>(assert_ptr(func), observer_);
    }
    /*!
     * \brief メンバアクセス
     *
     * * 参照先が生きていない場合terminateする。
     * * 返り値は元のelement_type型の参照で返るので、
     * メンバアクセスは通常のポインタと同様に行える。
     */
    template <typename = internal::skip_trace_tag>
    element_type *operator->() const {
        static std::string func = type_name() + "::operator->()";
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
    /*!
     * \brief 要素アクセス
     *
     * * `ptr[n]` は `*(ptr + n)` と同じ。
     * * 参照先が生きていない場合terminateする。
     */
    template <typename E = element_type, typename = internal::skip_trace_tag>
    wrap<E &> operator[](std::ptrdiff_t n) const {
        static std::string func = type_name() + "::operator[]()";
        return wrap<E &>((*this + n).assert_ptr(func), observer_);
    }

    /*!
     * \brief 生ポインタへのキャスト
     */
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
    return static_cast<element_type *>(wrapper);
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

} // namespace y3c
