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
    std::unique_ptr<internal::life> elems_iter_life_;
    std::unique_ptr<internal::life> elems_str_life_; // null終端を含むためelems_lifeより要素数が1多い
    internal::life life_;

    /*!
     * \brief ライフタイムを初期化
     */
    void init_elems_life() {
        if (!base_.empty()) {
            elems_iter_life_ = std::unique_ptr<internal::life>(
                new internal::life(&base_[0], &base_[0] + base_.size()));
            elems_str_life_ = std::unique_ptr<internal::life>(
                new internal::life(&base_[0], &base_[0] + base_.size() + 1));
        } else {
            elems_iter_life_ = std::unique_ptr<internal::life>(
                new internal::life(nullptr, nullptr));
            elems_str_life_ = std::unique_ptr<internal::life>(
                new internal::life(&base_[0], &base_[0] + 1));
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
        if (*elems_iter_life_ != pos.get_observer_()) {
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
        if (*elems_iter_life_ != pos.get_observer_()) {
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
        : base_(std::move(other.base_)), life_(this) {
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


    /*!
     * \brief 要素アクセス
     *
     * * インデックスが範囲外の場合、 out_of_range を投げる。
     *
     */
    reference at(size_type n, internal::skip_trace_tag = {}) {
        if (n >= base_.size()) {
            static std::string func = type_name() + "::at()";
            throw y3c::out_of_range(func, base_.size(),
                                    static_cast<std::ptrdiff_t>(n));
        }
        return reference(&this->base_[n], elems_str_life_->observer());
    }
    /*!
     * \brief 要素アクセス(const)
     *
     * * インデックスが範囲外の場合、 out_of_range を投げる。
     *
     */
    const_reference at(size_type n, internal::skip_trace_tag = {}) const {
        if (n >= base_.size()) {
            static std::string func = type_name() + "::at()";
            throw y3c::out_of_range(func, base_.size(),
                                    static_cast<std::ptrdiff_t>(n));
        }
        return const_reference(&this->base_[n], elems_str_life_->observer());
    }
    /*!
     * \brief 要素アクセス
     *
     * * インデックスが `0 <= n <= size()` の外の場合terminateする。
     * null終端へのアクセスは許可されている
     *
     */
    template <typename = internal::skip_trace_tag>
    reference operator[](size_type n) {
        if (n > base_.size()) {
            static std::string func = type_name() + "::operator[]()";
            y3c::internal::terminate_ub_out_of_range(
                func, base_.size(), static_cast<std::ptrdiff_t>(n));
        }
        return reference(&this->base_[n], elems_str_life_->observer());
    }
    /*!
     * \brief 要素アクセス(const)
     *
     * * インデックスが `0 <= n <= size()` の外の場合terminateする。
     * null終端へのアクセスは許可されている
     *
     */
    template <typename = internal::skip_trace_tag>
    const_reference operator[](size_type n) const {
        if (n > base_.size()) {
            static std::string func = type_name() + "::operator[]()";
            y3c::internal::terminate_ub_out_of_range(
                func, base_.size(), static_cast<std::ptrdiff_t>(n));
        }
        return const_reference(&this->base_[n], elems_str_life_->observer());
    }
    /*!
     * \brief 先頭の要素へのアクセス
     *
     * * サイズが0の場合terminateする。
     *
     */
    reference front(internal::skip_trace_tag = {}) {
        if (base_.empty()) {
            static std::string func = type_name() + "::front()";
            y3c::internal::terminate_ub_out_of_range(func, 0, 0);
        }
        return reference(&base_.front(), elems_str_life_->observer());
    }
    /*!
     * \brief 先頭の要素へのアクセス(const)
     *
     * * サイズが0の場合terminateする。
     *
     */
    const_reference front(internal::skip_trace_tag = {}) const {
        if (base_.empty()) {
            static std::string func = type_name() + "::front()";
            y3c::internal::terminate_ub_out_of_range(func, 0, 0);
        }
        return const_reference(&base_.front(), elems_str_life_->observer());
    }
    /*!
     * \brief 末尾の要素へのアクセス
     *
     * * サイズが0の場合terminateする。
     *
     */
    reference back(internal::skip_trace_tag = {}) {
        if (base_.empty()) {
            static std::string func = type_name() + "::back()";
            y3c::internal::terminate_ub_out_of_range(func, 0, -1);
        }
        return reference(&base_.back(), elems_str_life_->observer());
    }
    /*!
     * \brief 末尾の要素へのアクセス(const)
     *
     * * サイズが0の場合terminateする。
     *
     */
    const_reference back(internal::skip_trace_tag = {}) const {
        if (base_.empty()) {
            static std::string func = type_name() + "::back()";
            y3c::internal::terminate_ub_out_of_range(func, 0, -1);
        }
        return const_reference(&base_.back(), elems_str_life_->observer());
    }

    /*!
     * \brief 先頭要素へのポインタを取得
     *
     * * c_str()と同じnull終端された文字列へのポインタを返す。
     *
     */
    pointer data() {
        return pointer(&this->base_[0], elems_str_life_->observer());
    }
    /*!
     * \brief 先頭要素へのconstポインタを取得
     *
     * * c_str()と同じnull終端された文字列へのポインタを返す。
     *
     */
    const_pointer data() const {
        return const_pointer(&this->base_[0], elems_str_life_->observer());
    }
    /*!
     * \brief null終端された文字列へのconstポインタを取得
     */
    const_pointer c_str() const {
        return const_pointer(&this->base_[0], elems_str_life_->observer());
    }

    /*!
     * \brief 先頭要素を指すイテレータを取得
     *
     * * サイズが0の場合無効なイテレータを返す。
     *
     */
    iterator begin() {
        if (base_.empty()) {
            return iterator(nullptr, elems_iter_life_->observer(), &iter_name());
        }
        return iterator(&this->base_.front(), elems_iter_life_->observer(),
                        &iter_name());
    }
    /*!
     * \brief 先頭要素を指すconstイテレータを取得
     *
     * * サイズが0の場合無効なイテレータを返す。
     *
     */
    const_iterator begin() const {
        if (base_.empty()) {
            return const_iterator(nullptr, elems_iter_life_->observer(),
                                  &iter_name());
        }
        return const_iterator(&this->base_.front(), elems_iter_life_->observer(),
                              &iter_name());
    }
    /*!
     * \brief 先頭要素を指すconstイテレータを取得
     *
     * * サイズが0の場合無効なイテレータを返す。
     *
     */
    const_iterator cbegin() const { return begin(); }
    /*!
     * \brief 末尾要素を指すイテレータを取得
     *
     * * サイズが0の場合無効なイテレータを返す。
     *
     */
    iterator end() { return begin() + base_.size(); }
    /*!
     * \brief 末尾要素を指すconstイテレータを取得
     *
     * * サイズが0の場合無効なイテレータを返す。
     *
     */
    const_iterator end() const { return begin() + base_.size(); }
    /*!
     * \brief 末尾要素を指すconstイテレータを取得
     *
     * * サイズが0の場合無効なイテレータを返す。
     *
     */
    const_iterator cend() const { return begin() + base_.size(); }

    /*!
     * \brief sizeが0かどうかを返す
     */
    bool empty() const { return base_.empty(); }
    /*!
     * \brief 配列のサイズを取得
     */
    size_type size() const { return base_.size(); }
    /*!
     * \brief 配列のサイズを取得
     */
    size_type length() const { return base_.length(); }
    /*!
     * \brief 配列の最大サイズを取得
     */
    size_type max_size() const { return base_.max_size(); }
    /*!
     * \brief 現在のメモリ確保済みのサイズを取得
     */
    size_type capacity() const { return base_.capacity(); }

    /*!
     * \brief 要素のクリア
     * 
     * * 既存のイテレータは無効になる
     */
    void clear() {
        base_.clear();
        init_elems_life();
    }
    /*!
     * \brief 文字を挿入する
     * \param index 挿入する位置
     * \param count 挿入する個数
     * \param ch 挿入する文字
     * 
     * * indexが範囲外の場合 out_of_range を投げる。
     * * 既存のイテレータは無効になる
     */
    basic_string &insert(size_type index, size_type count, CharT ch, internal::skip_trace_tag = {}) {
        if(index > base_.size()){
            static std::string func = type_name() + "::insert()";
            throw y3c::out_of_range(func, base_.size(),
                                    static_cast<std::ptrdiff_t>(index));
        }
        base_.insert(index, count, ch);
        init_elems_life();
        return *this;
    }
    // insert(size_type index, const CharT*s);
    // insert(size_type index, const CharT*s, size_type count);
    /*!
     * \brief 文字列を挿入する
     * \param index 挿入する位置
     * \param str 挿入する文字列
     * 
     * * indexが範囲外の場合 out_of_range を投げる。
     * * 既存のイテレータは無効になる
     */
    basic_string &insert(size_type index, const std::basic_string<CharT> &str, internal::skip_trace_tag = {}) {
        if(index > base_.size()){
            static std::string func = type_name() + "::insert()";
            throw y3c::out_of_range(func, base_.size(),
                                    static_cast<std::ptrdiff_t>(index));
        }
        base_.insert(index, str);
        init_elems_life();
        return *this;
    }
    /*!
     * \brief 文字列を挿入する
     * \param index 挿入する位置
     * \param str 挿入する文字列
     * 
     * * indexが範囲外の場合 out_of_range を投げる。
     * * 既存のイテレータは無効になる
     */
    basic_string &insert(size_type index, const basic_string &str, internal::skip_trace_tag = {}) {
        return insert(index, str.base_);
    }
    /*!
     * \brief 文字列を挿入する
     * \param index 挿入する位置
     * \param str 挿入する文字列
     * \param s_index 挿入する文字列の開始位置
     * \param count 挿入する文字数
     * 
     * * indexまたはs_indexが範囲外の場合 out_of_range を投げる。
     * * 既存のイテレータは無効になる
     */
    basic_string &insert(size_type index, const std::basic_string<CharT> &str, size_type s_index, size_type count = npos, internal::skip_trace_tag = {}) {
        static std::string func = type_name() + "::insert()";
        if(index > base_.size()){
            throw y3c::out_of_range(func, base_.size(),
                                    static_cast<std::ptrdiff_t>(index));
        }
        if (s_index >= str.size()) {
            throw y3c::out_of_range(func, str.size(),
                                    static_cast<std::ptrdiff_t>(s_index));
        }
        base_.insert(index, str, s_index, count);
        init_elems_life();
        return *this;
    }
    /*!
     * \brief 文字列を挿入する
     * \param index 挿入する位置
     * \param str 挿入する文字列
     * \param s_index 挿入する文字列の開始位置
     * \param count 挿入する文字数
     * 
     * * indexまたはs_indexが範囲外の場合 out_of_range を投げる。
     * * 既存のイテレータは無効になる
     */
    basic_string &insert(size_type index, const basic_string &str, size_type s_index, size_type count = npos, internal::skip_trace_tag = {}) {
        return insert(index, str.base_, s_index, count);
    }
    /*!
     * \brief 文字を挿入する
     * \param pos 挿入する位置を指すイテレータ
     * \param ch 挿入する文字
     * 
     * * 指定した位置が無効であったりこのstringのものでない場合terminateする。
     * * 既存のイテレータは無効になる
     */
    iterator insert(const_iterator pos, CharT ch, internal::skip_trace_tag = {}) {
        static std::string func = type_name() + "::insert()";
        std::size_t index = assert_iter_including_end(pos, func);
        base_.insert(base_.begin() + index, ch);
        init_elems_life();
        return iterator(&base_[0] + index, elems_iter_life_->observer(),
                        &iter_name());
    }
    /*!
     * \brief 文字を挿入する
     * \param pos 挿入する位置を指すイテレータ
     * \param count 挿入する個数
     * \param ch 挿入する文字
     * 
     * * 指定した位置が無効であったりこのstringのものでない場合terminateする。
     * * 既存のイテレータは無効になる
     */
    iterator insert(const_iterator pos, size_type count, CharT ch, internal::skip_trace_tag = {}) {
        static std::string func = type_name() + "::insert()";
        std::size_t index = assert_iter_including_end(pos, func);
        base_.insert(base_.begin() + index, count, ch);
        init_elems_life();
        return iterator(&base_[0] + index, elems_iter_life_->observer(),
                        &iter_name());
    }
    /*!
     * \brief イテレーターから要素を挿入する
     * \param pos 挿入する位置を指すイテレータ
     * \param first, last 挿入する範囲
     * 
     * * 指定した位置が無効であったりこのstringのものでない場合terminateする。
     * * 既存のイテレータは無効になる
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
        return iterator(&base_[0] + index, elems_iter_life_->observer(),
                        &iter_name());
    }
    /*!
     * \brief std::initializer_listから要素を挿入する
     * \param pos 挿入する位置を指すイテレータ
     * \param ilist 挿入する要素
     * 
     * * 指定した位置が無効であったりこのstringのものでない場合terminateする。
     * * 既存のイテレータは無効になる
     */
    iterator insert(const_iterator pos, std::initializer_list<CharT> ilist,
                    internal::skip_trace_tag = {}) {
        static std::string func = type_name() + "::insert()";
        std::size_t index = assert_iter_including_end(pos, func);
        base_.insert(base_.begin() + index, ilist);
        update_elems_life(&base_[0] + index);
        return iterator(&base_[0] + index, elems_iter_life_->observer(),
                        &iter_name());
    }


};
} // namespace y3c
