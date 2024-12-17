#pragma once
#ifdef Y3C_MESON
#include "y3c-config.h"
#else
#include "y3c/y3c-config.h"
#endif
#include <memory>
#include <cassert>

Y3C_NS_BEGIN
namespace internal {
class life_state {
    bool alive_ = true;
    const bool is_allocated_ = false;
    const bool is_array_ = false;

    life_state() = default;
    explicit life_state(bool allocated, bool is_array)
        : is_allocated_(allocated), is_array_(is_array) {}

    static std::shared_ptr<life_state> allocate(bool is_array) {
        return std::shared_ptr<life_state>(new life_state(true, is_array));
    }

  public:
    life_state(const life_state &) = delete;
    life_state &operator=(const life_state &) = delete;
    ~life_state() = default;
    friend class life;
    friend class life_observer;
};

/*!
 * \brief ライフタイムの状態を観測するクラス
 *
 * オブジェクトを参照する側はlife_observerを受けとり、
 * empty() や dead() で状態を確認できる。
 *
 */
class life_observer {
    std::shared_ptr<life_state> state_;

    explicit life_observer(std::shared_ptr<life_state> state) : state_(state) {}

  public:
    /*!
     * オブジェクトを参照しない空のobserverになる
     *
     */
    explicit life_observer(std::nullptr_t) : state_(nullptr) {}
    life_observer(const life_observer &) = default;
    life_observer &operator=(const life_observer &) = default;
    ~life_observer() = default;

    bool empty() const { return state_ == nullptr; }
    bool dead() const { return !state_ || !state_->alive_; }

    static life_observer allocate(bool is_array) {
        return life_observer(life_state::allocate(is_array));
    }
    bool is_allocated() const { return state_ && state_->is_allocated_; }
    bool is_array() const { return state_ && state_->is_array_; }
    void deallocate() {
        assert(state_);
        assert(state_->is_allocated_);
        state_->alive_ = false;
    }

    // void swap(life_state &other) { this->state_.swap(other.state_); }

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
    life() : state_(new life_state()) {}
    life(const life &) = delete;
    life &operator=(const life &) = delete;
    ~life() { state_->alive_ = false; }

    // void swap(life &other) { this->state_.swap(other.state_); }

    life_observer observer() const { return life_observer(this->state_); }
};
} // namespace internal
Y3C_NS_END
