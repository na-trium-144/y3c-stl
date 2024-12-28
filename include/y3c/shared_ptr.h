#pragma once
#include "y3c/terminate.h"
#include "y3c/wrap.h"
#include "y3c/typename.h"
#include <memory>

namespace y3c {

/*!
 * \brief 所有権を共有するスマートポインタ (std::shared_ptr)
 *
 * * キャストするか unwrap() することで std::shared_ptr<T>
 * (のconst参照)に戻せる。
 *   * std::shared_ptr<T>
 * のconstでない参照で取得して変更を加えることはできないようにしている。
 * * 初期化は std::make_shared<T> または y3c::make_shared<T> で行う。
 * 生ポインタ(newで初期化したものなど)を受け取ることはできない。
 * * std::shared_ptr と相互変換できるが、
 * y3cが管理するオブジェクトの寿命の情報を
 * std::shared_ptr は保持していないので、
 * y3c::shared_ptr を破棄し std::shared_ptr
 * が残っている場合でも参照先のオブジェクトは破棄されたと判断されてしまう。
 *
 * \sa [shared_ptr -
 * cpprefjp](https://cpprefjp.github.io/reference/memory/shared_ptr.html)
 */
template <typename T>
class shared_ptr {
    std::shared_ptr<T> base_;
    std::shared_ptr<internal::life> ptr_life_;
    internal::life life_;

    const std::string &type_name() const {
        return internal::get_type_name<shared_ptr>();
    }

  public:
    /*!
     * \brief デフォルトコンストラクタ: nullptrを指す
     */
    shared_ptr(std::nullptr_t = nullptr) noexcept
        : base_(nullptr), ptr_life_(nullptr), life_(this) {}
    /*!
     * \brief コピーコンストラクタ: リソースを共有する
     */
    shared_ptr(const shared_ptr &other)
        : base_(other.base_), ptr_life_(other.ptr_life_), life_(this) {}
    /*!
     * \brief コピー代入: リソースを共有する
     */
    shared_ptr &operator=(const shared_ptr &other) {
        if (this != std::addressof(other)) {
            base_ = other.base_;
            ptr_life_ = other.ptr_life_;
        }
        return *this;
    }
    /*!
     * \brief ムーブコンストラクタ: 所有権を移動する
     */
    shared_ptr(shared_ptr &&other)
        : base_(std::move(other.base_)), ptr_life_(std::move(other.ptr_life_)),
          life_(this) {}
    /*!
     * \brief ムーブ代入: 所有権を移動する
     */
    shared_ptr &operator=(shared_ptr &&other) {
        if (this != std::addressof(other)) {
            base_ = std::move(other.base_);
            ptr_life_ = std::move(other.ptr_life_);
        }
        return *this;
    }
    ~shared_ptr() = default;

    using element_type = T;

    /*!
     * \brief std::shared_ptrからのコピー
     */
    template <typename U>
    shared_ptr(const std::shared_ptr<U> &ptr)
        : base_(ptr), ptr_life_(std::make_shared<internal::life>(base_.get())),
          life_(this) {}
    /*!
     * \brief std::shared_ptrからのコピー
     */
    template <typename U>
    shared_ptr &operator=(const std::shared_ptr<U> &ptr) {
        base_ = ptr;
        ptr_life_ = std::make_shared<internal::life>(base_.get());
        return *this;
    }
    /*!
     * \brief std::shared_ptrからの所有権の移動
     */
    template <typename U>
    shared_ptr(std::shared_ptr<U> &&ptr)
        : base_(std::move(ptr)),
          ptr_life_(std::make_shared<internal::life>(base_.get())),
          life_(this) {}
    /*!
     * \brief std::shared_ptrからの所有権の移動
     */
    template <typename U>
    shared_ptr &operator=(std::shared_ptr<U> &&ptr) {
        base_ = std::move(ptr);
        ptr_life_ = std::make_shared<internal::life>(base_.get());
        return *this;
    }

    /*!
     * \brief 別の要素型のshared_ptrとリソースを共有する
     */
    template <typename U>
    shared_ptr(const shared_ptr<U> &other)
        : base_(other.base_), ptr_life_(other.ptr_life_), life_(this) {}
    /*!
     * \brief 別の要素型のshared_ptrとリソースを共有する
     */
    template <typename U>
    shared_ptr &operator=(const shared_ptr<U> &other) {
        base_ = other.base_;
        ptr_life_ = other.ptr_life_;
        return *this;
    }
    /*!
     * \brief 別の要素型のshared_ptrから所有権を移動
     */
    template <typename U>
    shared_ptr(shared_ptr<U> &&other)
        : base_(std::move(other.base_)), ptr_life_(std::move(other.ptr_life_)),
          life_(this) {}
    /*!
     * \brief 別の要素型のshared_ptrから所有権を移動
     */
    template <typename U>
    shared_ptr &operator=(shared_ptr<U> &&other) {
        base_ = std::move(other.base_);
        ptr_life_ = std::move(other.ptr_life_);
        return *this;
    }

    template <typename U>
    friend class shared_ptr;

    /*!
     * \brief 所有権を放棄
     *
     * * nullptr を代入するのと同じ。
     */
    void reset() noexcept {
        base_.reset();
        ptr_life_.reset();
    }
    /*!
     * \brief 所有権を入れ替える
     */
    void swap(shared_ptr &other) noexcept {
        base_.swap(other.base_);
        ptr_life_.swap(other.ptr_life_);
    }

    /*!
     * \brief ポインタを取得
     *
     * * 返り値は wrap<element_type*> でラップされたポインタ。
     *   * 生ポインタを取得するにはさらに element_type* にキャストするか
     * unwrap() が必要。
     */
    ptr<element_type> get() const noexcept {
        if (!base_) {
            return nullptr;
        }
        y3c_assert_internal(ptr_life_);
        return ptr<element_type>(base_.get(), ptr_life_->observer());
    }
    /*!
     * \brief 要素の間接参照
     *
     * * 参照先が生きていない場合terminateする。
     * * 返り値は wrap<element_type&> でラップされた状態で返る。
     *   * 元のelement_type型に戻すにはさらにキャストするか unwrap() が必要。
     */
    template <typename E = element_type, typename = internal::skip_trace_tag>
    y3c::wrap_ref<E> operator*() const {
        if (!base_) {
            static std::string func = type_name() + "::operator*()";
            y3c::internal::terminate_ub_access_nullptr(func);
        }
        y3c_assert_internal(ptr_life_);
        return y3c::wrap_ref<E>(base_.get(), ptr_life_->observer());
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
        if (!base_) {
            static std::string func = type_name() + "::operator->()";
            y3c::internal::terminate_ub_access_nullptr(func);
        }
        y3c_assert_internal(ptr_life_);
        return base_.get();
    }
    // template <typename U = T>
    // U &operator[](std::ptrdiff_t i) const {
    //     if (i < 0 || todo)
    //     return check_()[i];
    // }

    /*!
     * \brief 所有権を共有しているshared_ptrの数を取得
     *
     * * std::shared_ptr の use_count を返す。
     * std::shared_ptr と y3c::shared_ptr で共有されている場合、
     * y3c側のオブジェクト寿命管理とは無関係。
     */
    long use_count() const noexcept { return base_.use_count(); }
    /*!
     * \brief 有効なリソースを所有しているかどうかを判定
     *
     * `shared_ptr != nullptr` と同じ。
     */
    explicit operator bool() const noexcept { return static_cast<bool>(base_); }
    /*!
     * \brief 所有権ベースでのポインタ比較
     */
    bool owner_before(const shared_ptr &arg) const {
        return base_.owner_before(arg.base_);
    }

    /*!
     * \brief const std::shared_ptr へのキャスト
     */
    operator const std::shared_ptr<T> &() const noexcept { return base_; }

    operator wrap<const shared_ptr &>() const noexcept {
        return wrap<const shared_ptr &>(this, life_.observer());
    }
    wrap<const shared_ptr *> operator&() const {
        return wrap<const shared_ptr *>(this, life_.observer());
    }
};

template <typename T>
const std::shared_ptr<T> &unwrap(const shared_ptr<T> &wrapper) noexcept {
    return static_cast<const std::shared_ptr<T> &>(wrapper);
}

template <typename T>
void swap(shared_ptr<T> &lhs, shared_ptr<T> &rhs) noexcept {
    lhs.swap(rhs);
}

template <typename T, typename U>
bool operator==(const shared_ptr<T> &lhs, const shared_ptr<U> &rhs) noexcept {
    return unwrap(lhs) == unwrap(rhs);
}
template <typename T, typename U>
bool operator!=(const shared_ptr<T> &lhs, const shared_ptr<U> &rhs) noexcept {
    return unwrap(lhs) != unwrap(rhs);
}
template <typename T, typename U>
bool operator<(const shared_ptr<T> &lhs, const shared_ptr<U> &rhs) noexcept {
    return unwrap(lhs) < unwrap(rhs);
}
template <typename T, typename U>
bool operator<=(const shared_ptr<T> &lhs, const shared_ptr<U> &rhs) noexcept {
    return unwrap(lhs) <= unwrap(rhs);
}
template <typename T, typename U>
bool operator>(const shared_ptr<T> &lhs, const shared_ptr<U> &rhs) noexcept {
    return unwrap(lhs) > unwrap(rhs);
}
template <typename T, typename U>
bool operator>=(const shared_ptr<T> &lhs, const shared_ptr<U> &rhs) noexcept {
    return unwrap(lhs) >= unwrap(rhs);
}

template <class CharT, class Traits, typename T>
std::basic_ostream<CharT, Traits> &
operator<<(std::basic_ostream<CharT, Traits> &os, const shared_ptr<T> &p) {
    return os << unwrap(p);
}

/*!
 * \brief shared_ptrを構築する
 * \param args Tのコンストラクタに渡す引数
 * \sa [make_shared -
 * cpprefjp](https://cpprefjp.github.io/reference/memory/make_shared.html)
 */
template <typename T, typename... Args>
shared_ptr<T> make_shared(Args &&...args) {
    return shared_ptr<T>(std::make_shared<T>(std::forward<Args>(args)...));
}

} // namespace y3c
