#pragma once
#include "y3c/terminate.h"
#include <memory>
#include <vector>

namespace y3c {
namespace internal {
template <typename element_type>
class contiguous_iterator;

struct life_validator {
    bool valid_;
    const void *ptr_;

    explicit life_validator(const void *ptr) : valid_(true), ptr_(ptr) {}
};
class life_state {
    bool alive_;
    const void *begin_, *end_;
    std::vector<std::shared_ptr<life_validator>> validators_;

  public:
    life_state(const void *begin, const void *end)
        : alive_(true), begin_(begin), end_(end) {}
    life_state(const life_state &) = delete;
    life_state &operator=(const life_state &) = delete;
    life_state(life_state &&) = delete;
    life_state &operator=(life_state &&) = delete;
    ~life_state() = default;

    void destroy() {
        alive_ = false;
        for (const auto &validator : validators_) {
            validator->valid_ = false;
        }
        validators_.clear();
    }
    const void *begin() const { return begin_; }
    const void *end() const { return end_; }
    void push_validator(const std::shared_ptr<life_validator> &v) {
        validators_.push_back(v);
    }
    void update_range(const void *new_begin, const void *new_end,
                      const void *invalidate_from = nullptr) {
        bool end_changed = end_ != new_end;
        for (auto it = validators_.begin(); it != validators_.end();) {
            auto validator = *it;
            if (validator->ptr_ < begin_ || validator->ptr_ > end_ ||
                validator->ptr_ < new_begin || validator->ptr_ > new_end ||
                (end_changed && validator->ptr_ == end_) ||
                (end_changed && validator->ptr_ == new_end) ||
                (invalidate_from != nullptr &&
                 validator->ptr_ >= invalidate_from)) {
                validator->valid_ = false;
                it = validators_.erase(it);
            } else {
                ++it;
            }
        }
        begin_ = new_begin;
        end_ = new_end;
    }
    bool alive() const { return alive_; }
    bool in_range(const void *ptr) const { return begin_ <= ptr && ptr < end_; }
    bool in_range(const void *begin, const void *end) const {
        return this->begin_ <= begin && begin <= end && end <= this->end_;
    }
    template <typename T>
    std::size_t size() const {
        return static_cast<const T *>(end_) - static_cast<const T *>(begin_);
    }
    template <typename T>
    std::ptrdiff_t index_of(T *ptr) const {
        return ptr - static_cast<const T *>(begin_);
    }
};

/*!
 * \brief ライフタイムの状態を観測するクラス
 *
 * オブジェクトを参照する側はlife_observerを受けとり、
 * empty(), alive(), in_range() で状態を確認できる。
 *
 */
class life_observer {
    std::shared_ptr<life_state> state_;

    explicit life_observer(const std::shared_ptr<life_state> &state)
        : state_(state) {}

  public:
    /*!
     * オブジェクトを参照しない空のobserverになる
     *
     */
    explicit life_observer(std::nullptr_t) : state_(nullptr) {}
    life_observer(const life_observer &) = default;
    life_observer &operator=(const life_observer &) = default;
    ~life_observer() = default;

    const std::shared_ptr<life_validator> &
    push_validator(const std::shared_ptr<life_validator> &v) const {
        state_->push_validator(v);
        return v;
    }
    const void *begin() const { return state_->begin(); }
    const void *end() const { return state_->end(); }
    template <typename element_type>
    element_type *assert_ptr(element_type *ptr, const std::string &func,
                             internal::skip_trace_tag = {}) const {
        // array<T, 0> の参照の場合 ptr_ = nullptr, alive = arrayの寿命
        // になる場合があるが、 その場合はnullptrアクセスエラーとしない
        if (!state_) {
            y3c::internal::terminate_ub_access_nullptr(func);
        }
        if (!state_->alive()) {
            y3c::internal::terminate_ub_access_deleted(func);
        }
        if (!state_->in_range(ptr)) {
            y3c::internal::terminate_ub_out_of_range(
                func, state_->size<element_type>(), state_->index_of(ptr));
        }
        return ptr;
    }
    template <typename element_type>
    element_type *assert_iter(const contiguous_iterator<element_type> &iter,
                              const std::string &func,
                              internal::skip_trace_tag = {}) const {
        if (!state_) {
            y3c::internal::terminate_ub_access_nullptr(func);
        }
        if (!state_->alive()) {
            y3c::internal::terminate_ub_access_deleted(func);
        }
        y3c_assert_internal(iter.validator_);
        if (!iter.validator_->valid_) {
            y3c::internal::terminate_ub_invalid_iter(func);
        }
        if (!state_->in_range(iter.ptr_)) {
            y3c::internal::terminate_ub_out_of_range(
                func, state_->size<element_type>(),
                state_->index_of(iter.ptr_));
        }
        return iter.ptr_;
    }
    template <typename element_type>
    void assert_range_iter(const contiguous_iterator<element_type> &begin,
                           const contiguous_iterator<element_type> &end,
                           const std::string &func,
                           internal::skip_trace_tag = {}) const {
        if (!state_) {
            y3c::internal::terminate_ub_access_nullptr(func);
        }
        if (!state_->alive()) {
            y3c::internal::terminate_ub_access_deleted(func);
        }
        y3c_assert_internal(begin.validator_);
        if (!begin.validator_->valid_) {
            y3c::internal::terminate_ub_invalid_iter(func);
        }
        y3c_assert_internal(end.validator_);
        if (!end.validator_->valid_) {
            y3c::internal::terminate_ub_invalid_iter(func);
        }
        if (!state_->in_range(begin.ptr_, end.ptr_)) {
            y3c::internal::terminate_ub_out_of_range(
                func, state_->size<element_type>(), state_->index_of(begin),
                state_->index_of(end));
        }
    }

    friend class life;
};

/*!
 * \brief オブジェクトのライフタイムを管理するクラス
 *
 * オブジェクトの所有者はlifeまたはshared_ptr<life>を持つ。
 *
 */
class life {
    std::shared_ptr<life_state> state_;

  public:
    explicit life(const void *begin, const void *end)
        : state_(new life_state(begin, end)) {}
    template <typename T>
    explicit life(T *begin) : life(begin, begin + 1) {}
    life(const life &) = delete;
    life &operator=(const life &) = delete;
    life(life &&) = delete;
    life &operator=(life &&) = delete;
    ~life() { state_->destroy(); }

    life_observer observer() const { return life_observer(this->state_); }
    /*!
     * 新しい範囲が以前の範囲と被っていれば範囲を更新し(observerは有効のまま)、
     * まったく異なる範囲であればリセットする(以前のobserverは無効になる)
     * \param invalidate_from 更新された範囲の先頭
     * (nullptrでない場合、これより後の範囲を追加で無効化する)
     */
    void update(const void *begin, const void *end,
                const void *invalidate_from = nullptr) {
        if ((begin <= state_->begin() && state_->begin() < end) ||
            (begin < state_->end() && state_->end() <= end)) {
            state_->update_range(begin, end);
        } else {
            state_->destroy();
            state_ = std::shared_ptr<life_state>(new life_state(begin, end));
        }
    }

    bool operator==(const life_observer &obs) const {
        return this->state_ == obs.state_;
    }
    bool operator!=(const life_observer &obs) const {
        return this->state_ != obs.state_;
    }
};
} // namespace internal
} // namespace y3c
