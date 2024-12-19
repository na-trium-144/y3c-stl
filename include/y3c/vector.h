#pragma once
#include "y3c/terminate.h"
#include "y3c/wrap.h"
#include "y3c/typename.h"
#include <vector>
#include <memory>

namespace y3c {

template <typename T>
class vector {
    std::vector<T> base_;
    std::unique_ptr<internal::life> elems_life_;
    internal::life life_;

    void init_elems_life() {
        if (!base_.empty()) {
            elems_life_ = std::unique_ptr<internal::life>(
                new internal::life(&base_[0], &base_[0] + base_.size()));
        } else {
            elems_life_ = std::unique_ptr<internal::life>(
                new internal::life(nullptr, nullptr));
        }
    }
    void update_elems_life() {
        if (!base_.empty()) {
            elems_life_->update(&base_[0], &base_[0] + base_.size());
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

  public:
    /*!
     * \brief サイズ0のvectorを作成する
     */
    vector() : base_(), life_(&base_) { init_elems_life(); }
    /*!
     * \brief 新しい領域にコピー構築
     */
    vector(const vector &other) : base_(other.base_), life_(&base_) {
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
          elems_life_(std::move(other.elems_life_)), life_(&base_) {}
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
    vector(const std::vector<T> &other) : base_(other), life_(&base_) {
        init_elems_life();
    }
    /*!
     * \brief ムーブ構築
     */
    vector(const std::vector<T> &&other)
        : base_(std::move(other)), life_(&base_) {
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
    explicit vector(size_type count) : base_(count), life_(&base_) {
        init_elems_life();
    }
    /*!
     * \brief サイズと値を指定して初期化
     */
    vector(size_type count, const T &value)
        : base_(count, value), life_(&base_) {
        init_elems_life();
    }
    /*!
     * \brief イテレータで初期化
     */
    template <typename InputIt>
    vector(InputIt first, InputIt last) : base_(first, last), life_(&base_) {
        init_elems_life();
    }
    /*!
     * \brief std::initializer_listで初期化
     */
    vector(std::initializer_list<T> init) : base_(init), life_(&base_) {
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
     */
    iterator erase(const_iterator pos, internal::skip_trace_tag = {}) {
        static std::string func = type_name() + "::erase()";
        if (elems_life_ != pos.get_observer_()) {
            y3c::internal::terminate_wrong_iter(func);
        }
        pos.get_observer_().assert_ptr(unwrap(pos), func);
        std::size_t index = unwrap(pos) - &base_[0];
        base_.erase(base_.begin() + index);
        update_elems_life();
        return iterator(&base_[index], elems_life_->observer(), &iter_name());
    }
    /*!
     * \brief 要素の削除
     */
    iterator erase(const_iterator begin, const_iterator end,
                   internal::skip_trace_tag = {}) {
        static std::string func = type_name() + "::erase()";
        if (elems_life_ != begin.get_observer_() ||
            elems_life_ != end.get_observer_()) {
            y3c::internal::terminate_wrong_iter(func);
        }
        begin.get_observer_().assert_range(unwrap(begin), unwrap(end), func);
        std::size_t index_begin = unwrap(begin) - &base_[0];
        std::size_t index_end = unwrap(end) - &base_[0];
        base_.erase(base_.begin() + index_begin, base_.begin() + index_end);
        update_elems_life();
        return iterator(&base_[index_begin], elems_life_->observer(),
                        &iter_name());
    }
};


} // namespace y3c