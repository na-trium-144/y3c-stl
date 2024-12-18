#pragma once
#include "y3c/terminate.h"
#include "y3c/wrap.h"
#include "y3c/typename.h"
#include <vector>
#include <memory>

namespace y3c {

template <typename T>
class vector {
    std::vector<T, internal::allocator<T>> base_;
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

    const std::string &type_name() const {
        return internal::get_type_name<vector>();
    }

  public:
    /*!
     * \brief サイズ0のvectorを作成する
     */
    vector() : base_(), life_(&base_) { init_elems_life(); }
    /*!
     * \brief 新しい領域にコピーを作成する
     */
    vector(const vector &other) : base_(other.base_), life_(&base_) {
        init_elems_life();
    }
    /*!
     * \brief ムーブ元の領域を自分のものとする
     * 
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
    // using iterator = internal::vector_iterator<T>;
    // using const_iterator = internal::vector_iterator<const T>;

    vector(const std::vector<T> &other) : base_(other), life_(&base_) {
        init_elems_life();
    }
    vector(const std::vector<T> &&other)
        : base_(std::move(other)), life_(&base_) {init_elems_life(); }
    vector &operator=(const std::vector<T> &other) {
        this->base_ = other;
        init_elems_life();
        return *this;
    }
    vector &operator=(std::vector<T> &&other) {
        this->base_ = std::move(other);
        init_elems_life();
        return *this;
    }

    explicit vector(size_type count)
        : base_(count), life_(&base_) { init_elems_life(); }
    vector(size_type count, const T &value)
        : base_(count, value), life_(&base_) {
        init_elems_life();
    }
    template <typename InputIt>
    vector(InputIt first, InputIt last)
        : base_(first, last), life_(&base_) {
        init_elems_life();
    }
    vector(std::initializer_list<T> init)
        : base_(init), life_(&base_) { init_elems_life(); }

    vector& operator=(std::initializer_list<T> ilist) {
        base_ = ilist;
        init_elems_life();
        return *this;
    }

    void assign(size_type count, const T &value) {
        base_.assign(count, value);
        init_elems_life();
    }
    template <typename InputIt>
    void assign(InputIt first, InputIt last) {
        base_.assign(first, last);
        init_elems_life();
    }
    void assign(std::initializer_list<T> ilist) {
        base_.assign(ilist);
        init_elems_life();
    }

    void clear() {
        base_.clear();
        init_elems_life();
    }

    void reserve(size_type new_cap) {
        if(base_.capacity() < new_cap){
            base_.reserve(new_cap);
            init_elems_life();
        }
    }

};


} // namespace y3c