#pragma once
#include "y3c/terminate.h"
#include <memory>

namespace y3c {
namespace internal {
class life_state {
    void *begin_, *end_;

  public:
    life_state(void *begin, void *end) : begin_(begin), end_(end) {}
    life_state(const life_state &) = delete;
    life_state &operator=(const life_state &) = delete;
    life_state(life_state &&) = delete;
    life_state &operator=(life_state &&) = delete;
    ~life_state() = default;
    void destroy() { begin_ = end_ = nullptr; }
    bool alive() const { return begin_ && end_; }
    bool in_range(const void *ptr) const { return begin_ <= ptr && ptr < end_; }
    template <typename T>
    std::size_t size() const {
        return static_cast<T *>(end_) - static_cast<T *>(begin_);
    }
    template <typename T>
    std::ptrdiff_t index_of(T *ptr) const {
        return ptr - static_cast<T *>(begin_);
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
    life(void *begin, void *end) : state_(new life_state(begin, end)) {}
    template <typename T>
    life(T *begin) : life(begin, begin + 1) {}
    life(const life &) = delete;
    life &operator=(const life &) = delete;
    life(life &&) = delete;
    life &operator=(life &&) = delete;
    ~life() { state_->destroy(); }

    life_observer observer() const { return life_observer(this->state_); }
};

/*!
 * \brief 領域の割り当てと削除を管理するクラス
 * 
 */
template <typename T>
class allocator {
    std::allocator<T> alloc_;
    std::shared_ptr<std::unordered_map<void *, std::size_t>> size_map_;

  public:
    using value_type = T;
    using pointer = T *;
    using const_pointer = const T *;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    allocator()
        : alloc_(),
          size_map_(
              std::make_shared<std::unordered_map<void *, std::size_t>>()) {}
    allocator(const allocator &other) = default;
    allocator &operator=(const allocator &other) = default;
    ~allocator() = default;

    template <typename U>
    allocator(const allocator<U> &other)
        : alloc_(other.alloc_), size_map_(other.size_map_) {}

    using is_always_equal = std::false_type;
    friend bool operator==(const allocator<T> &lhs, const allocator<T> &rhs) {
        return lhs.alloc_ == rhs.alloc_ && lhs.size_map_ == rhs.size_map_;
    }
    friend bool operator!=(const allocator<T> &lhs, const allocator<T> &rhs) {
        return !(lhs == rhs);
    }

    pointer allocate(size_type n) {
        pointer ptr = alloc_.allocate(n);
        size_map_->emplace(ptr, n);
        return ptr;
    }
    void deallocate(pointer ptr, size_type n) {
        alloc_.deallocate(ptr, n);
        size_map_->erase(ptr);
    }
    size_type max_size() const noexcept { return alloc_.max_size(); }

    allocator select_on_container_copy_construction() const {
        return allocator();
    }
    using propagate_on_container_copy_assignment = std::false_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
};

} // namespace internal
} // namespace y3c
