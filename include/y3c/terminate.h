#pragma once
#ifdef Y3C_MESON
#include "y3c-config.h"
#else
#include "y3c/y3c-config.h"
#endif
#include "y3c/what.h"
#include <stdexcept>
#include <string>
#include <memory>
#include <unordered_map>
#include <atomic>

namespace y3c {
namespace internal {
inline namespace Y3C_NS_ABI {

Y3C_DLL void Y3C_CALL link() noexcept;

/*!
 * y3c::内部の関数がスタックトレースに表示されないようにするために、
 * 関数の引数型やテンプレートにこれを含めると、スタックトレースからそのフレームを除外してくれる
 * (`skip_trace_tag = {}`, `template <typename = skip_trace_tag>` など)
 *
 * handle_final_terminate_message() はtag無しでも除外する例外。
 *
 */
struct skip_trace_tag {};

enum class terminate_type {
    exception,
    terminate,
    internal,
    ub_out_of_range,
    ub_access_nullptr,
    ub_access_deleted,
    ub_wrong_iter,
    ub_invalid_iter,
    ub_iter_after_end,
    ub_iter_before_begin,
};

struct terminate_detail {
    terminate_type type;
    const char *e_class;
    std::string func;
    std::string what;
    std::shared_ptr<void> raw_trace;

    Y3C_DLL terminate_detail(terminate_type type, const char *e_class,
                             std::string &&func, std::string &&what,
                             skip_trace_tag = {});
    terminate_detail(terminate_type type, std::string &&func,
                     std::string &&what, skip_trace_tag = {})
        : terminate_detail(type, nullptr, std::move(func), std::move(what)) {}
};

/*!
 * これが std::set_terminate() に自動的に登録され、
 * 例外がcatchされずterminateした時にメッセージを出してくれる。
 *
 */
[[noreturn]] Y3C_DLL void Y3C_CALL handle_final_terminate_message() noexcept;
/*!
 * \brief 例外を表示して強制終了する
 *
 * 内部ではstd::terminate()ではなくstd::abort()を呼んでいる
 *
 */
[[noreturn]] Y3C_DLL void Y3C_CALL do_terminate_with(terminate_detail &&detail);

/*!
 * \brief y3cの例外クラスのベース。
 *
 * スタックトレースや例外の詳細をコンストラクタでstatic変数に保存し、
 * what() は通常の例外と同様短いメッセージを返す。
 *
 */
class exception_base {
    int id;

  protected:
    std::string what;

  public:
    static Y3C_DLL std::atomic<int> last_exception_id;
    static Y3C_DLL std::unordered_map<int, terminate_detail> exceptions;

    exception_base(const char *e_class, std::string &&func, std::string &&what,
                   skip_trace_tag = {})
        : id(++last_exception_id), what(what) {
        exceptions.emplace(id,
                           terminate_detail(terminate_type::exception, e_class,
                                            std::move(func), std::move(what)));
    }
    exception_base(const exception_base &) = delete;
    exception_base &operator=(const exception_base &) = delete;
    exception_base(exception_base &&other)
        : id(other.id), what(std::move(other.what)) {
        other.id = -1;
    }
    exception_base &operator=(exception_base &&) = delete;
    ~exception_base() { exceptions.erase(id); }
};

/*!
 * 通常は terminate(), undefined_behavior() で
 * std::terminate()を呼んで強制終了するが、
 * これがtrueの場合代わりにthrowするようになる
 * (主にテスト用)
 *
 * 投げた例外がdll境界を越えるとMacOSでめんどくさいことになるので、
 * 例外はinline関数の中で投げる
 *
 */
extern Y3C_DLL bool throw_on_terminate;

#define define_terminate_func(ub_name)                                         \
    class ub_name {};                                                          \
    template <typename... Args, typename = skip_trace_tag>                     \
    [[noreturn]] void terminate_##ub_name(std::string func, Args &&...args) {  \
        if (throw_on_terminate) {                                              \
            throw ub_name();                                                   \
        }                                                                      \
        do_terminate_with({terminate_type::ub_name, std::move(func),           \
                           what::ub_name(std::forward<Args>(args)...)});       \
    }

define_terminate_func(ub_out_of_range);
define_terminate_func(ub_access_nullptr);
define_terminate_func(ub_access_deleted);
define_terminate_func(ub_wrong_iter);
define_terminate_func(ub_invalid_iter);
define_terminate_func(ub_iter_after_end);
define_terminate_func(ub_iter_before_begin);

#undef define_terminate_func

[[noreturn]] inline void terminate_internal(std::string func, std::string what,
                                            skip_trace_tag = {}) {
    do_terminate_with(
        {terminate_type::internal, std::move(func), std::move(what)});
}

#define y3c_assert_internal(cond)                                              \
    if (!(cond)) {                                                             \
        y3c::internal::terminate_internal(__func__,                            \
                                          "asserion '" #cond "' failed");      \
    }

} // namespace Y3C_NS_ABI
} // namespace internal

/*!
 * 実際のところなにもしない。
 *
 * y3c-stlはほとんどがテンプレートクラスであるため、
 * 使い方によってはy3cライブラリにincludeやリンクしてもy3cの初期化がされない場合がある。
 *
 * y3cライブラリのテンプレートでない関数を何かしら1つ呼ぶことで
 * 確実にリンクされ初期化させることができるので、
 * こんなものを用意してみた。
 *
 */
inline void link() { internal::link(); }

/*!
 * std:: のexceptionをそれぞれ継承しており、
 * y3c:: の各exceptionの間に継承関係はない
 *
 * ユーザーがcatchするのは std:: のほうで、
 * y3c:: のクラスのまま直接catchすることは想定していない
 * (catchできるけど...)
 *
 */
class out_of_range final : public std::out_of_range,
                           public internal::exception_base {
  public:
    out_of_range(std::string func, std::size_t size, std::ptrdiff_t index,
                 internal::skip_trace_tag = {})
        : std::out_of_range(""),
          internal::exception_base(
              "y3c::out_of_range", std::move(func),
              internal::what::ub_out_of_range(size, index)) {}

    const char *what() const noexcept override {
        return this->internal::exception_base::what.c_str();
    }
};

} // namespace y3c
