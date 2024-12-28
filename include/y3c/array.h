#pragma once
#include "y3c/terminate.h"
#include "y3c/wrap.h"
#include "y3c/typename.h"
#include "y3c/iterator.h"
#include <array>

namespace y3c {

/*!
 * \brief 固定長配列 (std::array)
 *
 * * キャストするか unwrap() することで std::array<T, N> (の参照)に戻せる。
 * * std::arrayと違って集成体初期化はできない
 * (できるようにする必要はあるのか?)
 * 
 * \sa [array - cpprefjp](https://cpprefjp.github.io/reference/array/array.html)
 */
template <typename T, std::size_t N>
class array {
    std::array<T, N> base_;
    internal::life elems_life_;
    internal::life life_;

    const std::string &type_name() const {
        return internal::get_type_name<array>();
    }
    const std::string &iter_name() const {
        static std::string name =
            internal::get_type_name<array>() + "::iterator";
        return name;
    }

  public:
    /*!
     * \brief デフォルトコンストラクタ: 各要素をデフォルト値で初期化
     */
    array()
        : base_(), elems_life_(N == 0 ? nullptr : &base_[0],
                               N == 0 ? nullptr : &base_[0] + N),
          life_(this) {}
    /*!
     * \brief コピーコンストラクタ
     */
    array(const array &other)
        : base_(other.base_), elems_life_(N == 0 ? nullptr : &base_[0],
                                          N == 0 ? nullptr : &base_[0] + N),
          life_(this) {}
    /*!
     * \brief コピー代入
     */
    array &operator=(const array &other) {
        this->base_ = other.base_;
        return *this;
    }
    /*!
     * \brief ムーブコンストラクタ
     */
    array(array &&other)
        : base_(std::move(other.base_)),
          elems_life_(N == 0 ? nullptr : &base_[0],
                      N == 0 ? nullptr : &base_[0] + N),
          life_(this) {}
    /*!
     * \brief ムーブ代入
     */
    array &operator=(array &&other) {
        if (this != std::addressof(other)) {
            this->base_ = std::move(other.base_);
        }
        return *this;
    }

    /*!
     * \brief 要素で初期化
     *
     * `y3c::array<int, 3>{1, 2, 3}` のような初期化をできるようにする
     *
     */
    template <typename... Args,
              typename std::enable_if<(sizeof...(Args) == N),
                                      std::nullptr_t>::type = nullptr>
    array(Args &&...args)
        : array(std::array<T, N>{std::forward<Args>(args)...}) {}
    /*!
     * \brief std::arrayからのコピー
     */
    array(const std::array<T, N> &elems)
        : base_(elems), elems_life_(N == 0 ? nullptr : &base_[0],
                                    N == 0 ? nullptr : &base_[0] + N),
          life_(this) {}
    /*!
     * \brief std::arrayからのムーブ
     */
    array(std::array<T, N> &&elems)
        : base_(std::move(elems)),
          elems_life_(N == 0 ? nullptr : &base_[0],
                      N == 0 ? nullptr : &base_[0] + N),
          life_(this) {}
    /*!
     * \brief std::arrayからのコピー
     */
    array &operator=(const std::array<T, N> &elems) {
        this->base_ = elems;
        return *this;
    }
    /*!
     * \brief std::arrayからのムーブ
     */
    array &operator=(std::array<T, N> &&elems) {
        this->base_ = std::move(elems);
        return *this;
    }

    using reference = wrap_ref<T>;
    using const_reference = const_wrap_ref<T>;
    using iterator = internal::contiguous_iterator<T>;
    using const_iterator = internal::contiguous_iterator<const T>;
    // using reverse_iterator = std::reverse_iterator<iterator>;
    // using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = ptr<T>;
    using const_pointer = const_ptr<T>;
    using value_type = T;

    /*!
     * \brief 要素アクセス
     *
     * * インデックスが範囲外の場合、 out_of_range を投げる。
     *
     */
    reference at(size_type n, internal::skip_trace_tag = {}) {
        if (n >= N) {
            static std::string func = type_name() + "::at()";
            throw y3c::out_of_range(func, N, static_cast<std::ptrdiff_t>(n));
        }
        return reference(&this->base_[n], this->elems_life_.observer());
    }
    /*!
     * \brief 要素アクセス(const)
     *
     * * インデックスが範囲外の場合、 out_of_range を投げる。
     *
     */
    const_reference at(size_type n, internal::skip_trace_tag = {}) const {
        if (n >= N) {
            static std::string func = type_name() + "::at()";
            throw y3c::out_of_range(func, N, static_cast<std::ptrdiff_t>(n));
        }
        return const_reference(&this->base_[n], this->elems_life_.observer());
    }
    /*!
     * \brief 要素アクセス
     *
     * * インデックスが範囲外の場合terminateする。
     *
     */
    template <typename = internal::skip_trace_tag>
    reference operator[](size_type n) {
        if (n >= N) {
            static std::string func = type_name() + "::operator[]()";
            y3c::internal::terminate_ub_out_of_range(
                func, N, static_cast<std::ptrdiff_t>(n));
        }
        return reference(&this->base_[n], this->elems_life_.observer());
    }
    /*!
     * \brief 要素アクセス(const)
     *
     * * インデックスが範囲外の場合terminateする。
     *
     */
    template <typename = internal::skip_trace_tag>
    const_reference operator[](size_type n) const {
        if (n >= N) {
            static std::string func = type_name() + "::operator[]()";
            y3c::internal::terminate_ub_out_of_range(
                func, N, static_cast<std::ptrdiff_t>(n));
        }
        return const_reference(&this->base_[n], this->elems_life_.observer());
    }

    /*!
     * \brief 先頭の要素へのアクセス
     *
     * * サイズが0の場合terminateする。
     *
     */
    reference front(internal::skip_trace_tag = {}) {
        if (N == 0) {
            static std::string func = type_name() + "::front()";
            y3c::internal::terminate_ub_out_of_range(func, N, 0);
        }
        return reference(&this->base_.front(), this->elems_life_.observer());
    }
    /*!
     * \brief 先頭の要素へのアクセス(const)
     *
     * * サイズが0の場合terminateする。
     *
     */
    const_reference front(internal::skip_trace_tag = {}) const {
        if (N == 0) {
            static std::string func = type_name() + "::front()";
            y3c::internal::terminate_ub_out_of_range(func, N, 0);
        }
        return const_reference(&this->base_.front(),
                               this->elems_life_.observer());
    }
    /*!
     * \brief 末尾の要素へのアクセス
     *
     * * サイズが0の場合terminateする。
     *
     */
    reference back(internal::skip_trace_tag = {}) {
        if (N == 0) {
            static std::string func = type_name() + "::back()";
            y3c::internal::terminate_ub_out_of_range(func, N, -1);
        }
        return reference(&this->base_.back(), this->elems_life_.observer());
    }
    /*!
     * \brief 末尾の要素へのアクセス
     *
     * * サイズが0の場合terminateする。
     *
     */
    const_reference back(internal::skip_trace_tag = {}) const {
        if (N == 0) {
            static std::string func = type_name() + "::back()";
            y3c::internal::terminate_ub_out_of_range(func, N, -1);
        }
        return const_reference(&this->base_.back(),
                               this->elems_life_.observer());
    }

    /*!
     * \brief 先頭要素へのポインタを取得
     *
     * * サイズが0の場合無効なポインタを返す。
     *
     */
    pointer data() {
        if (N == 0) {
            return pointer(nullptr, this->elems_life_.observer());
        }
        return pointer(&this->base_[0], this->elems_life_.observer());
    }
    /*!
     * \brief 先頭要素へのconstポインタを取得
     *
     * * サイズが0の場合無効なポインタを返す。
     *
     */
    const_pointer data() const {
        if (N == 0) {
            return const_pointer(nullptr, this->elems_life_.observer());
        }
        return const_pointer(&this->base_[0], this->elems_life_.observer());
    }

    /*!
     * \brief 先頭要素を指すイテレータを取得
     *
     * * サイズが0の場合無効なイテレータを返す。
     *
     */
    iterator begin() {
        if (N == 0) {
            return iterator(nullptr, this->elems_life_.observer(),
                            &iter_name());
        }
        return iterator(&this->base_.front(), this->elems_life_.observer(),
                        &iter_name());
    }
    /*!
     * \brief 先頭要素を指すconstイテレータを取得
     *
     * * サイズが0の場合無効なイテレータを返す。
     *
     */
    const_iterator begin() const {
        if (N == 0) {
            return const_iterator(nullptr, this->elems_life_.observer(),
                                  &iter_name());
        }
        return const_iterator(&this->base_.front(),
                              this->elems_life_.observer(), &iter_name());
    }
    /*!
     * \brief 先頭要素を指すconstイテレータを取得
     *
     * * サイズが0の場合無効なイテレータを返す。
     *
     */
    const_iterator cbegin() const { return begin(); }
    /*!
     * \brief 末尾要素を指すイテレータを取得
     *
     * * サイズが0の場合無効なイテレータを返す。
     *
     */
    iterator end() { return begin() + N; }
    /*!
     * \brief 末尾要素を指すconstイテレータを取得
     *
     * * サイズが0の場合無効なイテレータを返す。
     *
     */
    const_iterator end() const { return begin() + N; }
    /*!
     * \brief 末尾要素を指すconstイテレータを取得
     *
     * * サイズが0の場合無効なイテレータを返す。
     *
     */
    const_iterator cend() const { return begin() + N; }

    /*!
     * \brief sizeが0かどうかを返す
     * \return N == 0
     */
    bool empty() const noexcept { return N == 0; }
    /*!
     * \brief 配列のサイズを取得
     * \return N
     */
    size_type size() const noexcept { return N; }
    /*!
     * \brief 配列の最大サイズを取得
     * \return N
     */
    size_type max_size() const noexcept { return N; }

    /*!
     * \brief コンテナを要素で埋める
     */
    void fill(const T &value) { this->base_.fill(value); }
    /*!
     * \brief 別のarrayと要素を入れ替える
     */
    void swap(array &other) { this->base_.swap(other); }

    /*!
     * \brief std::array へのキャスト
     */
    operator std::array<T, N>&() noexcept {
        return base_;
    }
    /*!
     * \brief const std::array へのキャスト
     */
    operator const std::array<T, N>&() const noexcept {
        return base_;
    }
    
    operator wrap<array &>() noexcept {
        return wrap<array &>(this, life_.observer());
    }
    operator wrap<const array &>() const noexcept {
        return wrap<const array &>(this, life_.observer());
    }
    wrap<array *> operator&() { return wrap<array *>(this, life_.observer()); }
    wrap<const array *> operator&() const {
        return wrap<const array *>(this, life_.observer());
    }
};

template <typename T, std::size_t N>
std::array<T, N> &unwrap(array<T, N> &wrapper) noexcept {
    return static_cast<std::array<T, N> &>(wrapper);
}
template <typename T, std::size_t N>
const std::array<T, N> &unwrap(const array<T, N> &wrapper) noexcept {
    return static_cast<const std::array<T, N> &>(wrapper);
}

template <typename T, std::size_t N>
void swap(array<T, N> &lhs, array<T, N> &rhs) {
    lhs.swap(rhs);
}

template <typename T, std::size_t N>
bool operator==(const array<T, N> &lhs, const array<T, N> &rhs) {
    return unwrap(lhs) == unwrap(rhs);
}
template <typename T, std::size_t N>
bool operator!=(const array<T, N> &lhs, const array<T, N> &rhs) {
    return unwrap(lhs) != unwrap(rhs);
}
template <typename T, std::size_t N>
bool operator<(const array<T, N> &lhs, const array<T, N> &rhs) {
    return unwrap(lhs) < unwrap(rhs);
}
template <typename T, std::size_t N>
bool operator<=(const array<T, N> &lhs, const array<T, N> &rhs) {
    return unwrap(lhs) <= unwrap(rhs);
}
template <typename T, std::size_t N>
bool operator>(const array<T, N> &lhs, const array<T, N> &rhs) {
    return unwrap(lhs) > unwrap(rhs);
}
template <typename T, std::size_t N>
bool operator>=(const array<T, N> &lhs, const array<T, N> &rhs) {
    return unwrap(lhs) >= unwrap(rhs);
}

} // namespace y3c
