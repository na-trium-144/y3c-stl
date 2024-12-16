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
    struct state {
        bool alive_ = true;
    };
    std::shared_ptr<state> state_;

    void construct() { state_ = std::make_shared<state>(); }
    void destruct() {
        assert(state_);
        state_->alive_ = false;
    }

  public:
    life_state() = default;
    life_state(const life_state &) = default;
    life_state &operator=(const life_state &) = default;
    ~life_state() = default;

    bool empty() const { return state_ == nullptr; }
    bool dead() const { return !state_ || !state_->alive_; }

    // void swap(life_state &other) { this->state_.swap(other.state_); }

    friend class life;
};

/*!
 * \brief オブジェクトのライフタイム管理
 *
 * * オブジェクトの所有者はlifeまたはshared_ptr<life>を持つ。
 * * オブジェクトを参照する側はshared_ptr<life_state>を持ち、
 * lifeが生きているかどうかを取得できる
 *
 */
class life {
    life_state state_;

  public:
    life() : state_() { state_.construct(); }
    life(const life &) = delete;
    life &operator=(const life &) = delete;
    life(life &&) = default;
    life &operator=(life &&) = default;
    ~life() { state_.destruct(); }

    // void swap(life &other) { this->state_.swap(other.state_); }

    life_state state() const { return state_; }
};
} // namespace internal
Y3C_NS_END
