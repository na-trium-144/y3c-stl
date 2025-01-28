#pragma once
#include "y3c/terminate.h"
#include "y3c/wrap.h"
#include "y3c/typename.h"
#include "y3c/iterator.h"
#include <string>

namespace y3c {

/*!
 * \brief 文字列 (std::basic_string)
 *
 * * キャストするか unwrap() することで std::basic_string<CharT>
 * (のconst参照)に戻せる。
 *   * std::string<CharT>
 * のconstでない参照で取得して変更を加えることはできないようにしている。
 *
 * \sa [basic_string -
 * cpprefjp](https://cpprefjp.github.io/reference/string/basic_string.html)
 * \sa string, wstring
 */

template <typename CharT, typename Traits = std::char_traits<CharT>>
class basic_string {
    std::basic_string<CharT, Traits> base_;
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

    const std::string &type_name() const {
        if (std::is_same<CharT, char>::value) {
            static std::string name = "y3c::string";
            return name;
        }
        if (std::is_same<CharT, wchar_t>::value) {
            static std::string name = "y3c::wstring";
            return name;
        }
        if (std::is_same<CharT, std::char_traits<CharT>>::value) {
            static std::string name =
                "y3c::basic_string<" + internal::get_type_name<CharT>() + ">";
            return name;
        }
        return internal::get_type_name<basic_string>();
    }
    const std::string &iter_name() const {
        static std::string name = type_name() + "::iterator";
        return name;
    }

    std::size_t
    assert_iter(const internal::contiguous_iterator<const CharT> &pos,
                const std::string &func, internal::skip_trace_tag = {}) const {
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
    std::size_t assert_iter_including_end(
        const internal::contiguous_iterator<const CharT> &pos,
        const std::string &func, internal::skip_trace_tag = {}) const {
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
     * \brief サイズ0のstringを作成する
     */
    basic_string() : base_(), life_(this) { init_elems_life(); }
    /*!
     * \brief 新しい領域にコピー構築
     */
    basic_string(const basic_string &other) : base_(other.base_), life_(this) {
        init_elems_life();
    }
    /*!
     * \brief ムーブ構築
     *
     * * ムーブ元の領域を自分のものとする。
     * * このコンテナとムーブ元の既存のイテレータは無効になる
     *
     */
    basic_string(basic_string &&other)
        : base_(std::move(other.base_)), elems_life_(), life_(this) {
        init_elems_life();
        other.init_elems_life();
    }
    /*!
     * \brief 文字列をコピー
     *
     * * このコンテナの既存のイテレータは無効になる
     *
     */
    basic_string &operator=(const basic_string &other) {
        this->base_ = other.base_;
        init_elems_life();
        return *this;
    }
    /*!
     * \brief 文字列をコピー
     * 
     * * このコンテナの既存のイテレータは無効になる
     *
     */
    basic_string &assign(const basic_string &other) {
        return *this = other;
    }
    /*!
     * \brief 文字列をムーブ
     *
     * * このコンテナとムーブ元の既存のイテレータは無効になる
     *
     */
    basic_string &operator=(basic_string &&other) {
        if (this != std::addressof(other)) {
            this->base_ = std::move(other.base_);
            init_elems_life();
            other.init_elems_life();
        }
        return *this;
    }
    /*!
     * \brief 文字列をムーブ
     *
     * * このコンテナとムーブ元の既存のイテレータは無効になる
     *
     */
    basic_string &assign(basic_string &&other) {
        return *this = std::move(other);
    }
    ~basic_string() = default;

    using traits_type = Traits;
    using value_type = CharT;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = wrap_ref<CharT>;
    using const_reference = const_wrap_ref<CharT>;
    using pointer = ptr<CharT>;
    using const_pointer = const_ptr<CharT>;
    using iterator = internal::contiguous_iterator<CharT>;
    using const_iterator = internal::contiguous_iterator<const CharT>;

    static const size_type npos = std::basic_string<CharT>::npos;

    /*!
     * \brief std::basic_stringからコピー構築
     */
    basic_string(const std::basic_string<CharT> &other)
        : base_(other), life_(this) {
        init_elems_life();
    }
    /*!
     * \brief std::basic_stringからのコピー代入
     *
     * * 既存のイテレータは無効になる
     *
     */
    basic_string &operator=(const std::basic_string<CharT> &other) {
        this->base_ = other;
        init_elems_life();
        return *this;
    }
    /*!
     * \brief std::basic_stringからのコピー代入
     *
     * * 既存のイテレータは無効になる
     *
     */
    basic_string &assign(const std::basic_string<CharT> &other) {
        return *this = other;
    }
    /*!
     * \brief std::basic_stringからムーブ構築
     */
    basic_string(const std::basic_string<CharT> &&other)
        : base_(std::move(other)), life_(this) {
        init_elems_life();
    }
    /*!
     * \brief std::basic_stringからのムーブ代入
     *
     * * 既存のイテレータは無効になる
     *
     */
    basic_string &operator=(std::basic_string<CharT> &&other) {
        this->base_ = std::move(other);
        init_elems_life();
        return *this;
    }
    /*!
     * \brief std::basic_stringからのムーブ代入
     *
     * * 既存のイテレータは無効になる
     *
     */
    basic_string &assign(std::basic_string<CharT> &&other) {
        return *this = std::move(other);
    }
    /*!
     * \brief stringの一部をコピー
     * 
     * * 既存のイテレータは無効になる
     * 
     */
    basic_string(const std::basic_string<CharT> &str, size_type pos,
                 size_type n = npos, internal::skip_trace_tag = {})
        : base_(), life_(this) {
        if (pos >= str.size()) {
            static std::string func = type_name() + "::constructor";
            throw y3c::out_of_range(func, str.size(),
                                    static_cast<std::ptrdiff_t>(pos));
        }
        base_.assign(str, pos, n);
        init_elems_life();
    }

    /*!
     * \brief stringの一部をコピー
     * 
     * * 既存のイテレータは無効になる
     * 
     */
    basic_string(const basic_string &str, size_type pos,
                 size_type n = npos, internal::skip_trace_tag = {})
        : basic_string(str.base_, pos, n) {}
    /*!
     * \brief stringの一部をコピー
     * 
     * * 既存のイテレータは無効になる
     * 
     */
    basic_string& assign(const std::basic_string<CharT> &str, size_type pos,
                 size_type n = npos, internal::skip_trace_tag = {}){
        if (pos >= str.size()) {
            static std::string func = type_name() + "::assign()";
            throw y3c::out_of_range(func, str.size(),
                                    static_cast<std::ptrdiff_t>(pos));
        }
        base_.assign(str, pos, n);
        init_elems_life();
        return *this;
    }
    /*!
     * \brief stringの一部をコピー
     * 
     * * 既存のイテレータは無効になる
     * 
     */
    basic_string& assign(const basic_string &str, size_type pos,
                 size_type n = npos, internal::skip_trace_tag = {}){
        return assign(str.base_, pos, n);
    }

    // basic_string(const CharT* s, size_type n): base_(s, n), life_(this)
    // {init_elems_life(); }
    // basic_string(const CharT* s): base_(s), life_(this)
    // {init_elems_life(); }
    // basic_string &operator=(const CharT* s)
    // basic_string &assign(const CharT* s)
    // basic_string &assign(const CharT* s, size_type n)

    /*!
     * \brief 文字の繰り返しで初期化
     */
    basic_string(size_type n, CharT c) : base_(n, c), life_(this) {
        init_elems_life();
    }
    /*!
     * \brief 文字を繰り返したものを代入
     * 
     * * 既存のイテレータは無効になる
     * 
     */
    basic_string& assign(size_type n, CharT c){
        base_.assign(n, c);
        init_elems_life();
        return *this;
    }
    /*!
     * \brief 1文字代入
     * 
     * * 既存のイテレータは無効になる
     * 
     */
    basic_string& operator=(CharT c){
        base_ = c;
        init_elems_life();
        return *this;
    }
    /*!
     * \brief 1文字代入
     * 
     * * 既存のイテレータは無効になる
     * 
     */
    basic_string& assign(CharT c){
        return *this = c;
    }

    /*!
     * \brief イテレータで初期化
     */
    template <typename InputIt>
    basic_string(InputIt first, InputIt last)
        : base_(first, last), life_(this) {
        init_elems_life();
    }
    /*!
     * \brief イテレータで代入
     * 
     * * 既存のイテレータは無効になる
     * 
     */
    template <typename InputIt>
    basic_string& assign(InputIt first, InputIt last){
        base_.assign(first, last);
        init_elems_life();
        return *this;
    }

    /*!
     * \brief std::initializer_listで初期化
     */
    basic_string(std::initializer_list<CharT> init) : base_(init), life_(this) {
        init_elems_life();
    }
    /*!
     * \brief std::initializer_listで代入
     * 
     * * 既存のイテレータは無効になる
     * 
     */
    basic_string &operator=(std::initializer_list<CharT> init){
        base_ = std::move(init);
        init_elems_life();
        return *this;
    }
    /*!
     * \brief std::initializer_listで代入
     * 
     * * 既存のイテレータは無効になる
     * 
     */
    basic_string &assign(std::initializer_list<CharT> init){
        return *this = init;
    }

    // todo: string_viewからの変換


};
} // namespace y3c
