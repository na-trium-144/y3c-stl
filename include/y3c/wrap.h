#pragma once
#include "y3c/terminate.h"
#include "y3c/life.h"
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
template <typename T, std::size_t N>
class array;

template <typename T>
T &unwrap(wrap<T> &wrapper) noexcept;
template <typename T>
const T &unwrap(const wrap<T> &wrapper) noexcept;
template <typename T>
T &unwrap(const wrap_ref<T> &wrapper, internal::skip_trace_tag = {});

/*!
 * \brief T型のデータ(base_)と、このコンテナの生存状態(alive_)を管理するクラス
 * y3cの各コンテナ型のベース
 *
 * * alive_は wrap_ref や ptr を生成したときに渡され、
 * wrapが破棄される時にfalseにする
 * * `operator&` で y3c::ptr<T> が得られる
 * * y3c::wrap_ref<T> にキャストできる
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
    internal::life life_;

  protected:
    internal::life_observer life_observer() const { return life_.observer(); }

    base_type &unwrap() noexcept { return base_; }
    const base_type &unwrap() const noexcept { return base_; }

  public:
    wrap() : base_(), life_() {}
    wrap(const wrap &other) : base_(other.base_), life_() {}
    wrap(wrap &&other) : base_(std::move(other.base_)), life_() {}
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
    wrap(Args &&...args) : base_(std::forward<Args>(args)...), life_() {}
    template <typename Args>
    wrap &operator=(Args &&args) {
        base_ = std::forward<Args>(args);
        return *this;
    }
    wrap(const base_type &base) : base_(base), life_() {}
    wrap(base_type &&base) : base_(std::move(base)), life_() {}
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
 * \brief T型のデータへのポインタと、その生存状態を持つクラス
 *
 * * `T&`へのキャスト時と、`y3c::unwrap()`
 * 時に参照先が生きているかなどのチェックを行う。
 * キャストとy3c::unwrapでチェックが入るのは (今のところ) wrap_ref のみ。
 * * y3c::wrap<remove_const_t<T>> の左辺値からキャストできる
 * * y3c::wrap_ref<T> から y3c::const_wrap_ref<T> にキャストできる
 * * `operator&` で y3c::ptr<T> が得られる
 *
 * \sa const_wrap_ref
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
    internal::life_observer range_observer_;

  protected:
    element_type *ptr_unwrap(const char *func,
                             internal::skip_trace_tag = {}) const {
        // array<T, 0> の参照の場合 ptr_ = nullptr, alive = arrayの寿命
        // になる場合があるが、 その場合はnullptrアクセスエラーとしない
        if (range_observer_.empty()) {
            y3c::internal::terminate_ub_access_nullptr(func);
        }
        if (range_observer_.dead()) {
            y3c::internal::terminate_ub_access_deleted(func);
        }
        if (ptr_ - begin_ < 0 ||
            ptr_ - begin_ >= static_cast<std::ptrdiff_t>(size_)) {
            y3c::internal::terminate_ub_out_of_range(func, size_,
                                                     ptr_ - begin_);
        }
        return ptr_;
    }

    wrap_ref(element_type *begin, std::size_t size, element_type *ptr,
             internal::life_observer range_observer) noexcept
        : begin_(begin), size_(size), ptr_(ptr),
          range_observer_(range_observer) {}
    wrap_ref(element_type *ptr, internal::life_observer ptr_observer) noexcept
        : wrap_ref(ptr, 1, ptr, ptr_observer) {}

    wrap_ref(std::nullptr_t = nullptr) noexcept
        : begin_(nullptr), size_(0), ptr_(nullptr), range_observer_(nullptr) {}

  public:
    template <typename U>
    wrap_ref(wrap<U> &wrapper)
        : begin_(&wrapper.base_), size_(1), ptr_(&wrapper.base_),
          range_observer_(wrapper.life_observer()) {}

    template <typename U>
    wrap_ref(const wrap_ref<U> &ref) noexcept
        : begin_(ref.begin_), size_(ref.size_), ptr_(ref.ptr_),
          range_observer_(ref.range_observer_) {}

    template <typename Args, typename = internal::skip_trace_tag>
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
    template <typename = internal::skip_trace_tag>
    wrap_ref &operator=(const wrap_ref &other) {
        *ptr_unwrap("y3c::wrap_ref::operator=()") =
            *other.ptr_unwrap("cast from y3c::wrap_ref to reference");
        return *this;
    }
    template <typename = internal::skip_trace_tag>
    wrap_ref &operator=(wrap_ref &&other) {
        T &val = *other.ptr_unwrap("cast from y3c::wrap_ref to reference");
        if (this != std::addressof(other)) {
            *ptr_unwrap("y3c::wrap_ref::operator=()") = std::move(val);
        }
        return *this;
    }
    ~wrap_ref() = default;

    friend class wrap_auto<element_type>;
    friend element_type &
    y3c::unwrap<element_type>(const wrap_ref<element_type> &,
                              internal::skip_trace_tag);

    template <typename = internal::skip_trace_tag>
    operator element_type &() {
        return *ptr_unwrap("cast from y3c::wrap_ref to reference");
    }

    ptr<element_type> operator&() const noexcept;
};

template <typename T>
using const_wrap_ref = wrap_ref<const T>;

template <typename T>
T &unwrap(const wrap_ref<T> &wrapper, internal::skip_trace_tag) {
    return *wrapper.ptr_unwrap("y3c::unwrap()");
}

/*!
 * 値の参照を返す関数が、y3c::wrap_ref<T> を返す代わりにこれを返すことで、
 * ユーザーがそれをさらに明示的に y3c::wrap_ref<T>
 * にキャストすれば元の参照を返すが、
 * autoで受け取るなどwrap_refにならなかった場合は参照ではなく値をコピーしたものとしてふるまう
 *
 * * `operator&` で y3c::ptr<T> が得られる
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
    internal::life_observer range_observer_;

    void clear_ref() {
        begin_ = &this->unwrap();
        size_ = 1;
        ptr_ = &this->unwrap();
        range_observer_ = this->life_observer();
    }

    wrap_auto(element_type *begin, std::size_t size, element_type *ptr,
              internal::life_observer range_observer) noexcept
        : wrap<base_type>(*ptr), begin_(begin), size_(size), ptr_(ptr),
          range_observer_(range_observer) {}
    wrap_auto(element_type *ptr, internal::life_observer ptr_observer) noexcept
        : wrap_auto(ptr, 1, ptr, ptr_observer) {}

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

    ptr<element_type> operator&() const noexcept;
};
template <typename T>
template <typename U>
wrap_ref<T>::wrap_ref(const wrap_auto<U> &auto_ref) noexcept
    : wrap_ref(auto_ref.begin_, auto_ref.size_, auto_ref.ptr_,
               auto_ref.range_observer_) {}

template <typename T>
T &unwrap(const wrap_auto<T> &wrapper, internal::skip_trace_tag = {}) {
    return unwrap(wrap_ref<T>(wrapper));
}

/*!
 * \brief 生ポインタ T* のラッパー
 *
 * * `operator*`, `operator->`, `operator[]`
 * 時にnullptrチェックと範囲外アクセスチェックをする
 * 使用時にnullptrでないかと範囲外でないかのチェックを行う。
 * * また `operator*`, `operator[]` が返す参照はラップ済み (y3c::wrap_auto)
 * * `T*` にキャストできるがその場合チェックされないので注意
 * * y3c::ptr<T> から y3c::const_ptr<T> にキャストできる
 *
 * \sa y3c::const_ptr, y3c::ptr_const, y3c::const_ptr_const
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
    internal::life_observer range_observer_;

    element_type *ptr_unwrap(const char *func,
                             internal::skip_trace_tag = {}) const {
        // array<T, 0> の参照の場合 ptr_ = nullptr, alive = arrayの寿命
        // になる場合があるが、 その場合はnullptrアクセスエラーとしない
        if (range_observer_.empty()) {
            y3c::internal::terminate_ub_access_nullptr(func);
        }
        if (range_observer_.dead()) {
            y3c::internal::terminate_ub_access_deleted(func);
        }
        if (this->unwrap() - begin_ < 0 ||
            this->unwrap() - begin_ >= static_cast<std::ptrdiff_t>(size_)) {
            y3c::internal::terminate_ub_out_of_range(func, size_,
                                                     this->unwrap() - begin_);
        }
        return this->unwrap();
    }

    ptr(element_type *begin, std::size_t size, element_type *ptr_,
        internal::life_observer range_observer) noexcept
        : wrap<T *>(ptr_), begin_(begin), size_(size),
          range_observer_(range_observer) {}
    ptr(element_type *ptr_, internal::life_observer ptr_observer) noexcept
        : ptr(ptr_, 1, ptr_, ptr_observer) {}

  public:
    ptr(std::nullptr_t = nullptr) noexcept
        : wrap<T *>(nullptr), begin_(nullptr), size_(0),
          range_observer_(nullptr) {}

    template <typename U>
    ptr(const ptr<U, ptr_type> &ref)
        : wrap<T *>(unwrap(ref)), begin_(ref.begin_), size_(ref.size_),
          range_observer_(ref.range_observer_) {}
    template <typename U>
    ptr &operator=(const ptr<U, ptr_type> &ref) {
        this->wrap<T *>::operator=(unwrap(ref));
        begin_ = ref.begin_;
        size_ = ref.size_;
        range_observer_ = ref.range_observer_;
        return *this;
    }

    friend class wrap<element_type>;
    friend class wrap_ref<element_type>;
    friend class shared_ptr<element_type>;
    template <typename, internal::ptr_type_enum>
    friend class ptr;
    template <typename, std::size_t>
    friend class array;

    template <typename = internal::skip_trace_tag>
    wrap_auto<element_type> operator*() const {
        static std::string func_name = func_name_() + "::operator*()";
        return wrap_auto<element_type>(
            begin_, size_, ptr_unwrap(func_name.c_str()), range_observer_);
    }
    template <typename = internal::skip_trace_tag>
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
        return ptr(begin_, size_, this->unwrap() + n, range_observer_);
    }
    ptr operator-(std::ptrdiff_t n) const {
        return ptr(begin_, size_, this->unwrap() - n, range_observer_);
    }
    std::ptrdiff_t operator-(const ptr &other) const {
        return this->unwrap() - other.ptr_;
    }
    template <typename = internal::skip_trace_tag>
    wrap_auto<element_type> operator[](std::ptrdiff_t n) const {
        static std::string func_name = func_name_() + "::operator[]()";
        return wrap_auto<element_type>(
            begin_, size_, (*this + n).ptr_unwrap(func_name.c_str()),
            range_observer_);
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
    return ptr<T>(&base_, life_observer());
}
template <typename T>
inline ptr<const T> wrap<T>::operator&() const {
    return ptr<const T>(&base_, life_observer());
}
template <typename T>
inline ptr<T> wrap_ref<T>::operator&() const noexcept {
    return ptr<T>(begin_, size_, ptr_, range_observer_);
}
template <typename T>
inline ptr<T> wrap_auto<T>::operator&() const noexcept {
    return &wrap_ref<T>(*this);
}

Y3C_NS_END
