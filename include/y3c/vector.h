#pragma once
#include "y3c/terminate.h"
#include "y3c/wrap.h"
#include "y3c/typename.h"
#include "y3c/iterator.h"
#include <vector>
#include <memory>

namespace y3c {

template <typename T>
class vector;

template <typename T>
const std::vector<T> &unwrap(const vector<T> &wrapper) noexcept {
    return wrapper.base_;
}

/*!
 * \todo
 * * std::length_error
 *
 */
template <typename T>
class vector {
    std::vector<T> base_;
    std::unique_ptr<internal::life> elems_life_;
    internal::life life_;

    /*!
     * \brief ライフタイムを初期化
     */
    void init_elems_life() {
        if (!base_.empty()) {
            elems_life_ = std::unique_ptr<internal::life>(
                new internal::life(&base_[0], &base_[0] + base_.size()));
        } else {
            elems_life_ = std::unique_ptr<internal::life>(
                new internal::life(nullptr, nullptr));
        }
    }
    /*!
     * \brief 範囲が更新されていた場合その分だけライフタイムを初期化
     * \param invalidate_from 更新された範囲の先頭
     * (nullptrでない場合、これより後の範囲を追加で無効化する)
     */
    void update_elems_life(const void *invalidate_from = nullptr) {
        if (!base_.empty()) {
            elems_life_->update(&base_[0], &base_[0] + base_.size(),
                                invalidate_from);
        } else {
            elems_life_->update(nullptr, nullptr);
        }
    }

    const std::string &type_name() const {
        return internal::get_type_name<vector>();
    }
    const std::string &iter_name() const {
        static std::string name =
            internal::get_type_name<vector>() + "::iterator";
        return name;
    }

    std::size_t assert_iter(const internal::contiguous_iterator<const T> &pos,
                            const std::string &func,
                            internal::skip_trace_tag = {}) const {
        if (*elems_life_ != pos.get_observer_()) {
            y3c::internal::terminate_ub_wrong_iter(func);
        }
        pos.get_observer_().assert_iter(pos, func);
        if (base_.size() == 0) {
            return 0;
        } else {
            return y3c::internal::unwrap(pos) - &base_[0];
        }
    }
    std::size_t
    assert_iter_including_end(const internal::contiguous_iterator<const T> &pos,
                              const std::string &func,
                              internal::skip_trace_tag = {}) const {
        if (*elems_life_ != pos.get_observer_()) {
            y3c::internal::terminate_ub_wrong_iter(func);
        }
        pos.get_observer_().assert_iter_including_end(pos, func);
        if (base_.size() == 0) {
            return 0;
        } else {
            return y3c::internal::unwrap(pos) - &base_[0];
        }
    }

  public:
    /*!
     * \brief サイズ0のvectorを作成する
     */
    vector() : base_(), life_(this) { init_elems_life(); }
    /*!
     * \brief 新しい領域にコピー構築
     */
    vector(const vector &other) : base_(other.base_), life_(this) {
        init_elems_life();
    }
    /*!
     * \brief ムーブ構築
     *
     * ムーブ元の領域を自分のものとする。
     * ムーブ元を指していたイテレータは有効のまま
     *
     */
    vector(vector &&other)
        : base_(std::move(other.base_)),
          elems_life_(std::move(other.elems_life_)), life_(this) {}
    /*!
     * \brief すべての要素をコピー
     *
     * 既存のイテレータは無効になる
     *
     */
    vector &operator=(const vector &other) {
        this->base_ = other.base_;
        init_elems_life();
        return *this;
    }
    /*!
     * \brief すべての要素をムーブ
     *
     * ムーブ元を指していたイテレータは有効のまま
     *
     */
    vector &operator=(vector &&other) {
        if (this != std::addressof(other)) {
            this->base_ = std::move(other.base_);
            this->elems_life_ = std::move(other.elems_life_);
        }
        return *this;
    }
    ~vector() = default;

    friend const std::vector<T> &y3c::unwrap<>(const vector<T> &) noexcept;

    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = wrap_ref<T>;
    using const_reference = const_wrap_ref<T>;
    using pointer = ptr<T>;
    using const_pointer = const_ptr<T>;
    using iterator = internal::contiguous_iterator<T>;
    using const_iterator = internal::contiguous_iterator<const T>;

    /*!
     * \brief コピー構築
     */
    vector(const std::vector<T> &other) : base_(other), life_(this) {
        init_elems_life();
    }
    /*!
     * \brief ムーブ構築
     */
    vector(const std::vector<T> &&other)
        : base_(std::move(other)), life_(this) {
        init_elems_life();
    }
    /*!
     * \brief コピー代入
     *
     * 既存のイテレータは無効になる
     *
     */
    vector &operator=(const std::vector<T> &other) {
        this->base_ = other;
        init_elems_life();
        return *this;
    }
    /*!
     * \brief ムーブ代入
     *
     */
    vector &operator=(std::vector<T> &&other) {
        this->base_ = std::move(other);
        init_elems_life();
        return *this;
    }

    /*!
     * \brief サイズ指定して初期化
     */
    explicit vector(size_type count) : base_(count), life_(this) {
        init_elems_life();
    }
    /*!
     * \brief サイズと値を指定して初期化
     */
    vector(size_type count, const T &value) : base_(count, value), life_(this) {
        init_elems_life();
    }
    /*!
     * \brief イテレータで初期化
     */
    template <typename InputIt>
    vector(InputIt first, InputIt last) : base_(first, last), life_(this) {
        init_elems_life();
    }
    /*!
     * \brief std::initializer_listで初期化
     */
    vector(std::initializer_list<T> init) : base_(init), life_(this) {
        init_elems_life();
    }
    /*!
     * \brief initialization_listの代入
     *
     * 既存のイテレータは無効になる
     */
    vector &operator=(std::initializer_list<T> ilist) {
        base_ = ilist;
        init_elems_life();
        return *this;
    }

    /*!
     * \brief サイズと値を指定して要素を置き換える
     *
     * 既存のイテレータは無効になる
     *
     */
    void assign(size_type count, const T &value) {
        base_.assign(count, value);
        init_elems_life();
    }
    /*!
     * \brief イテレータからのコピーで要素を置き換える
     *
     * 既存のイテレータは無効になる
     *
     */
    template <typename InputIt>
    void assign(InputIt first, InputIt last) {
        base_.assign(first, last);
        init_elems_life();
    }
    /*!
     * \brief initializer_listで要素を置き換える
     *
     * 既存のイテレータは無効になる
     *
     */
    void assign(std::initializer_list<T> ilist) {
        base_.assign(ilist);
        init_elems_life();
    }
    /*!
     * \brief 要素のクリア
     *
     * 既存のイテレータは無効になる
     *
     */
    void clear() {
        base_.clear();
        init_elems_life();
    }

    /*!
     * \brief 領域の確保
     *
     * 再割り当てが発生した場合、既存のイテレータは無効になる
     *
     */
    void reserve(size_type new_cap) {
        base_.reserve(new_cap);
        update_elems_life();
    }
    /*!
     * \brief 容量の縮小
     *
     * 再割り当てが発生した場合、既存のイテレータは無効になる
     *
     */
    void shrink_to_fit() {
        base_.shrink_to_fit();
        update_elems_life();
    }

    /*!
     * \brief 要素の削除
     *
     * 削除した位置以降を指していたイテレータは無効になる
     *
     */
    iterator erase(const_iterator pos, internal::skip_trace_tag = {}) {
        static std::string func = type_name() + "::erase()";
        std::size_t index = assert_iter(pos, func);
        base_.erase(base_.begin() + index);
        update_elems_life(&base_[0] + index);
        return iterator(&base_[0] + index, elems_life_->observer(),
                        &iter_name());
    }
    /*!
     * \brief 要素の削除
     */
    iterator erase(const_iterator begin, const_iterator end,
                   internal::skip_trace_tag = {}) {
        static std::string func = type_name() + "::erase()";
        if (*elems_life_ != begin.get_observer_() ||
            *elems_life_ != end.get_observer_()) {
            y3c::internal::terminate_ub_wrong_iter(func);
        }
        begin.get_observer_().assert_range_iter(begin, end, func);
        std::size_t index_begin = y3c::internal::unwrap(begin) - &base_[0];
        std::size_t index_end = y3c::internal::unwrap(end) - &base_[0];
        base_.erase(base_.begin() + index_begin, base_.begin() + index_end);
        update_elems_life(&base_[0] + index_begin);
        return iterator(&base_[0] + index_begin, elems_life_->observer(),
                        &iter_name());
    }
    /*!
     * \brief 要素の追加
     *
     * 再割り当てが発生した場合、既存のイテレータは無効になる
     * そうでない場合、end()を指していたもののみ無効になる
     */
    void push_back(const T &value) {
        base_.push_back(value);
        update_elems_life();
    }
    /*!
     * \brief 要素の追加
     *
     * 再割り当てが発生した場合、既存のイテレータは無効になる
     * そうでない場合、end()を指していたもののみ無効になる
     */
    void push_back(T &&value) {
        base_.push_back(std::move(value));
        update_elems_life();
    }
    /*!
     * \brief 要素の追加
     *
     * 再割り当てが発生した場合、既存のイテレータは無効になる
     * そうでない場合、end()を指していたもののみ無効になる
     */
    template <typename... Args>
    reference emplace_back(Args &&...args) {
        base_.emplace_back(std::forward<Args>(args)...);
        update_elems_life();
        return back();
    }

    /*!
     * \brief 要素の挿入
     *
     * 再割り当てが発生した場合、既存のイテレータは無効になる
     * そうでない場合、挿入位置以降が無効になる
     */
    iterator insert(const_iterator pos, const T &value,
                    internal::skip_trace_tag = {}) {
        static std::string func = type_name() + "::insert()";
        std::size_t index = assert_iter_including_end(pos, func);
        base_.insert(base_.begin() + index, value);
        update_elems_life(&base_[0] + index);
        return iterator(&base_[0] + index, elems_life_->observer(),
                        &iter_name());
    }
    /*!
     * \brief 要素の挿入
     *
     * 再割り当てが発生した場合、既存のイテレータは無効になる
     * そうでない場合、挿入位置以降が無効になる
     */
    iterator insert(const_iterator pos, T &&value,
                    internal::skip_trace_tag = {}) {
        static std::string func = type_name() + "::insert()";
        std::size_t index = assert_iter_including_end(pos, func);
        base_.insert(base_.begin() + index, std::move(value));
        update_elems_life(&base_[0] + index);
        return iterator(&base_[0] + index, elems_life_->observer(),
                        &iter_name());
    }
    /*!
     * \brief 要素の挿入
     *
     * 再割り当てが発生した場合、既存のイテレータは無効になる
     * そうでない場合、挿入位置以降が無効になる
     */
    iterator insert(const_iterator pos, size_type count, const T &value,
                    internal::skip_trace_tag = {}) {
        static std::string func = type_name() + "::insert()";
        std::size_t index = assert_iter_including_end(pos, func);
        base_.insert(base_.begin() + index, count, std::move(value));
        update_elems_life(&base_[0] + index);
        return iterator(&base_[0] + index, elems_life_->observer(),
                        &iter_name());
    }
    /*!
     * \brief 要素の挿入
     *
     * 再割り当てが発生した場合、既存のイテレータは無効になる
     * そうでない場合、挿入位置以降が無効になる
     */
    template <typename InputIt,
              typename std::enable_if<
                  std::is_convertible<
                      typename std::iterator_traits<InputIt>::reference,
                      value_type>::value,
                  std::nullptr_t>::type = nullptr>
    iterator insert(const_iterator pos, InputIt first, InputIt last,
                    internal::skip_trace_tag = {}) {
        static std::string func = type_name() + "::insert()";
        std::size_t index = assert_iter_including_end(pos, func);
        base_.insert(base_.begin() + index, first, last);
        update_elems_life(&base_[0] + index);
        return iterator(&base_[0] + index, elems_life_->observer(),
                        &iter_name());
    }
    /*!
     * \brief 要素の挿入
     *
     * 再割り当てが発生した場合、既存のイテレータは無効になる
     * そうでない場合、挿入位置以降が無効になる
     */
    iterator insert(const_iterator pos, std::initializer_list<T> ilist,
                    internal::skip_trace_tag = {}) {
        static std::string func = type_name() + "::insert()";
        std::size_t index = assert_iter_including_end(pos, func);
        base_.insert(base_.begin() + index, ilist);
        update_elems_life(&base_[0] + index);
        return iterator(&base_[0] + index, elems_life_->observer(),
                        &iter_name());
    }
    /*!
     * \brief 要素の挿入
     *
     * 再割り当てが発生した場合、既存のイテレータは無効になる
     * そうでない場合、挿入位置以降が無効になる
     */
    template <typename... Args, typename = internal::skip_trace_tag>
    iterator emplace(const_iterator pos, Args &&...args) {
        static std::string func = type_name() + "::emplace()";
        std::size_t index = assert_iter_including_end(pos, func);
        base_.emplace(base_.begin() + index, std::forward<Args>(args)...);
        update_elems_life(&base_[0] + index);
        return iterator(&base_[0] + index, elems_life_->observer(),
                        &iter_name());
    }

    /*!
     * \brief サイズを変更
     *
     * 再割り当てが発生した場合、既存のイテレータは無効になる
     * そうでない場合、削除された要素とend()を指すもののみ無効になる
     */
    void resize(size_type count) {
        base_.resize(count);
        update_elems_life();
    }
    /*!
     * \brief サイズを変更
     *
     * 再割り当てが発生した場合、既存のイテレータは無効になる
     * そうでない場合、削除された要素とend()を指すもののみ無効になる
     */
    void resize(size_type count, const T &value) {
        base_.resize(count, value);
        update_elems_life();
    }

    /*!
     * \brief 末尾の要素を削除
     *
     * 最後の要素とend()を指すイテレータは無効になる
     *
     */
    void pop_back(internal::skip_trace_tag = {}) {
        if (base_.empty()) {
            static std::string func = type_name() + "::pop_back()";
            y3c::internal::terminate_ub_out_of_range(func, 0, -1);
        }
        base_.pop_back();
        update_elems_life();
    }

    reference at(size_type n, internal::skip_trace_tag = {}) {
        if (n >= base_.size()) {
            static std::string func = type_name() + "::at()";
            throw y3c::out_of_range(func, base_.size(),
                                    static_cast<std::ptrdiff_t>(n));
        }
        return reference(&this->base_[n], elems_life_->observer());
    }
    const_reference at(size_type n, internal::skip_trace_tag = {}) const {
        if (n >= base_.size()) {
            static std::string func = type_name() + "::at()";
            throw y3c::out_of_range(func, base_.size(),
                                    static_cast<std::ptrdiff_t>(n));
        }
        return const_reference(&this->base_[n], elems_life_->observer());
    }
    template <typename = internal::skip_trace_tag>
    reference operator[](size_type n) {
        if (n >= base_.size()) {
            static std::string func = type_name() + "::operator[]()";
            y3c::internal::terminate_ub_out_of_range(
                func, base_.size(), static_cast<std::ptrdiff_t>(n));
        }
        return reference(&this->base_[n], elems_life_->observer());
    }
    template <typename = internal::skip_trace_tag>
    const_reference operator[](size_type n) const {
        if (n >= base_.size()) {
            static std::string func = type_name() + "::operator[]()";
            y3c::internal::terminate_ub_out_of_range(
                func, base_.size(), static_cast<std::ptrdiff_t>(n));
        }
        return const_reference(&this->base_[n], elems_life_->observer());
    }
    reference front(internal::skip_trace_tag = {}) {
        if (base_.empty()) {
            static std::string func = type_name() + "::front()";
            y3c::internal::terminate_ub_out_of_range(func, 0, 0);
        }
        return reference(&base_.front(), elems_life_->observer());
    }
    const_reference front(internal::skip_trace_tag = {}) const {
        if (base_.empty()) {
            static std::string func = type_name() + "::front()";
            y3c::internal::terminate_ub_out_of_range(func, 0, 0);
        }
        return const_reference(&base_.front(), elems_life_->observer());
    }
    reference back(internal::skip_trace_tag = {}) {
        if (base_.empty()) {
            static std::string func = type_name() + "::back()";
            y3c::internal::terminate_ub_out_of_range(func, 0, -1);
        }
        return reference(&base_.back(), elems_life_->observer());
    }
    const_reference back(internal::skip_trace_tag = {}) const {
        if (base_.empty()) {
            static std::string func = type_name() + "::back()";
            y3c::internal::terminate_ub_out_of_range(func, 0, -1);
        }
        return const_reference(&base_.back(), elems_life_->observer());
    }

    pointer data() {
        if (base_.empty()) {
            return pointer(nullptr, elems_life_->observer());
        }
        return pointer(&this->base_[0], elems_life_->observer());
    }
    const_pointer data() const {
        if (base_.empty()) {
            return const_pointer(nullptr, elems_life_->observer());
        }
        return const_pointer(&this->base_[0], elems_life_->observer());
    }

    iterator begin() {
        if (base_.empty()) {
            return iterator(nullptr, elems_life_->observer(), &iter_name());
        }
        return iterator(&this->base_.front(), elems_life_->observer(),
                        &iter_name());
    }
    const_iterator begin() const {
        if (base_.empty()) {
            return const_iterator(nullptr, elems_life_->observer(),
                                  &iter_name());
        }
        return const_iterator(&this->base_.front(), elems_life_->observer(),
                              &iter_name());
    }
    const_iterator cbegin() const { return begin(); }
    iterator end() { return begin() + base_.size(); }
    const_iterator end() const { return begin() + base_.size(); }
    const_iterator cend() const { return begin() + base_.size(); }

    bool empty() const { return base_.empty(); }
    size_type size() const { return base_.size(); }
    size_type max_size() const { return base_.max_size(); }
    size_type capacity() const { return base_.capacity(); }

    void swap(vector &other) {
        base_.swap(other.base_);
        elems_life_.swap(other.elems_life_);
        update_elems_life(&base_[0] + base_.size());
        other.update_elems_life(&other.base_[0] + other.base_.size());
    }

    operator wrap<const vector &>() const noexcept {
        return wrap<const vector &>(this, life_.observer());
    }
    wrap<const vector *> operator&() const {
        return wrap<const vector *>(this, life_.observer());
    }
};

template <typename T>
void swap(vector<T> &lhs, vector<T> &rhs) {
    lhs.swap(rhs);
}

template <typename T>
bool operator==(const vector<T> &lhs, const vector<T> &rhs) {
    return unwrap(lhs) == unwrap(rhs);
}
template <typename T>
bool operator!=(const vector<T> &lhs, const vector<T> &rhs) {
    return unwrap(lhs) != unwrap(rhs);
}
template <typename T>
bool operator<(const vector<T> &lhs, const vector<T> &rhs) {
    return unwrap(lhs) < unwrap(rhs);
}
template <typename T>
bool operator<=(const vector<T> &lhs, const vector<T> &rhs) {
    return unwrap(lhs) <= unwrap(rhs);
}
template <typename T>
bool operator>(const vector<T> &lhs, const vector<T> &rhs) {
    return unwrap(lhs) > unwrap(rhs);
}
template <typename T>
bool operator>=(const vector<T> &lhs, const vector<T> &rhs) {
    return unwrap(lhs) >= unwrap(rhs);
}

} // namespace y3c