#pragma once
#include "y3c/terminate.h"
#include "y3c/life.h"
#include "y3c/wrap.h"
#include <memory>

namespace y3c {
namespace internal {
template <typename element_type>
class contiguous_iterator;

template <typename element_type>
element_type *unwrap(const contiguous_iterator<element_type> &wrapper) noexcept;

/*!
 * \brief array, vector など各種コンテナのイテレータ
 *
 * * `operator*`, `operator->`, `operator[]`
 * 時にnullptrチェックと範囲外アクセスチェックをする
 * 使用時にnullptrでないかと範囲外でないかのチェックを行う。
 * * また `operator*`, `operator[]` が返す参照はラップ済み (y3c::wrap_auto)
 * * `operator&` は未実装 (要るのか?)
 *
 */
template <typename element_type>
class contiguous_iterator {
    element_type *ptr_;
    internal::life_observer observer_;
    std::shared_ptr<internal::life_validator> validator_;
    // internal::life life_;

    const std::string *type_name_;
    const std::string &type_name() const {
        // return internal::get_type_name<contiguous_iterator>();
        return *type_name_;
    }

    element_type *assert_iter(const std::string &func,
                              internal::skip_trace_tag = {}) const {
        return observer_.assert_iter(*this, func);
    }
    void update_iter(const std::string &func,
                     internal::skip_trace_tag = {}) const {
        validator_->ptr_ = this->ptr_;
        if (ptr_ > observer_.end()) {
            internal::terminate_ub_iter_after_end(func);
        }
        if (ptr_ < observer_.begin()) {
            internal::terminate_ub_iter_before_begin(func);
        }
    }

  public:
    contiguous_iterator(element_type *ptr, internal::life_observer observer,
                        const std::string *type_name) noexcept
        : ptr_(ptr), observer_(observer),
          validator_(observer.push_validator(
              std::make_shared<life_validator>(ptr /* always valid */))),
          /*life_(&ptr),*/ type_name_(type_name) {}
    contiguous_iterator(element_type *ptr, internal::life_observer observer,
                        const std::string *type_name, const std::string &func,
                        internal::skip_trace_tag = {}) noexcept
        : ptr_(ptr), observer_(observer),
          validator_(observer.push_validator(
              std::make_shared<life_validator>(ptr /* always valid */))),
          /*life_(&ptr),*/ type_name_(type_name) {
        update_iter(func);
    }

    template <typename T, typename std::enable_if<
                              std::is_same<const T, element_type>::value,
                              std::nullptr_t>::type = nullptr>
    contiguous_iterator(const contiguous_iterator<T> &other)
        : ptr_(other.ptr_), observer_(other.observer_),
          validator_(observer_.push_validator(std::make_shared<life_validator>(
              *other.validator_))), /*life_(&ptr_),*/
          type_name_(other.type_name_) {}

    contiguous_iterator(const contiguous_iterator &other)
        : ptr_(other.ptr_), observer_(other.observer_),
          validator_(observer_.push_validator(std::make_shared<life_validator>(
              *other.validator_))), /*life_(&ptr_),*/
          type_name_(other.type_name_) {}
    contiguous_iterator &operator=(const contiguous_iterator &other) {
        ptr_ = other.ptr_;
        observer_ = other.observer_;
        *validator_ = observer_.push_validator(
            std::make_shared<life_validator>(*other.validator_));
        type_name_ = other.type_name_;
        return *this;
    }
    ~contiguous_iterator() = default;

    friend class life_observer;
    const life_observer &get_observer_() const { return this->observer_; }
    friend element_type *y3c::internal::unwrap<>(
        const contiguous_iterator<element_type> &wrapper) noexcept;

    using difference_type = std::ptrdiff_t;
    using value_type = element_type;
    using pointer = element_type *;
    using reference = wrap_auto<element_type>;
    using iterator_category = std::random_access_iterator_tag;

    template <typename = internal::skip_trace_tag>
    wrap_auto<element_type> operator*() const {
        std::string func = type_name() + "::operator*()";
        return wrap_auto<element_type>(assert_iter(func), observer_);
    }
    template <typename = internal::skip_trace_tag>
    element_type *operator->() const {
        std::string func = type_name() + "::operator->()";
        return assert_iter(func);
    }

    template <typename = internal::skip_trace_tag>
    contiguous_iterator &operator++() {
        ++this->ptr_;
        update_iter(type_name() + "::operator++()");
        return *this;
    }
    template <typename = internal::skip_trace_tag>
    contiguous_iterator operator++(int) {
        contiguous_iterator copy = *this;
        ++this->ptr_;
        update_iter(type_name() + "::operator++()");
        return copy;
    }
    template <typename = internal::skip_trace_tag>
    contiguous_iterator &operator--() {
        --this->ptr_;
        update_iter(type_name() + "::operator--()");
        return *this;
    }
    template <typename = internal::skip_trace_tag>
    contiguous_iterator operator--(int) {
        contiguous_iterator copy = *this;
        --this->ptr_;
        update_iter(type_name() + "::operator--()");
        return copy;
    }
    template <typename = internal::skip_trace_tag>
    contiguous_iterator &operator+=(std::ptrdiff_t n) {
        this->ptr_ += n;
        update_iter(type_name() + "::operator+=()");
        return *this;
    }
    template <typename = internal::skip_trace_tag>
    contiguous_iterator &operator-=(std::ptrdiff_t n) {
        this->ptr_ -= n;
        update_iter(type_name() + "::operator-=()");
        return *this;
    }
    template <typename = internal::skip_trace_tag>
    contiguous_iterator operator+(std::ptrdiff_t n) const {
        return contiguous_iterator(this->ptr_ + n, this->observer_, type_name_,
                                   type_name() + "::operator+()");
    }
    template <typename = internal::skip_trace_tag>
    contiguous_iterator operator-(std::ptrdiff_t n) const {
        return contiguous_iterator(this->ptr_ - n, this->observer_, type_name_,
                                   type_name() + "::operator-()");
    }

    std::ptrdiff_t operator-(const contiguous_iterator &other) const {
        return ptr_ - other.ptr_;
    }
    template <typename = internal::skip_trace_tag>
    wrap_auto<element_type> operator[](std::ptrdiff_t n) const {
        static std::string func = type_name() + "::operator[]()";
        return wrap_auto<element_type>((*this + n).assert_iter(func),
                                       observer_);
    }

    contiguous_iterator *operator&() = delete;
    const contiguous_iterator *operator&() const = delete;

    friend bool operator==(const contiguous_iterator &lhs,
                           const contiguous_iterator &rhs) {
        return lhs.ptr_ == rhs.ptr_;
    }
    friend bool operator!=(const contiguous_iterator &lhs,
                           const contiguous_iterator &rhs) {
        return !(lhs == rhs);
    }
    friend bool operator<(const contiguous_iterator &lhs,
                          const contiguous_iterator &rhs) {
        return lhs.ptr_ < rhs.ptr_;
    }
    friend bool operator>(const contiguous_iterator &lhs,
                          const contiguous_iterator &rhs) {
        return lhs.ptr_ > rhs.ptr_;
    }
    friend bool operator<=(const contiguous_iterator &lhs,
                           const contiguous_iterator &rhs) {
        return lhs.ptr_ <= rhs.ptr_;
    }
    friend bool operator>=(const contiguous_iterator &lhs,
                           const contiguous_iterator &rhs) {
        return lhs.ptr_ >= rhs.ptr_;
    }
};

template <typename element_type>
element_type *
unwrap(const contiguous_iterator<element_type> &wrapper) noexcept {
    return wrapper.ptr_;
}

} // namespace internal

template <typename element_type>
element_type *
unwrap(const internal::contiguous_iterator<element_type> &wrapper) noexcept {
    return internal::unwrap(wrapper);
}

} // namespace y3c
