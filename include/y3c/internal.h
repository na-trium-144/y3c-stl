#pragma once
#ifdef Y3C_MESON
#include "y3c-config.h"
#else
#include "y3c/y3c-config.h"
#endif
#include <stdexcept>
#include <string>
#include <memory>

Y3C_NS_BEGIN

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
Y3C_DLL void Y3C_CALL link() noexcept;

/*!
 * エラーメッセージの生成
 *
 */
namespace msg {
Y3C_DLL std::string Y3C_CALL out_of_range(std::size_t size, long long index);
inline std::string Y3C_CALL out_of_range(std::size_t size, std::size_t index) {
    return out_of_range(size, static_cast<long long>(index));
}
Y3C_DLL const char *Y3C_CALL access_nullptr();
Y3C_DLL const char *Y3C_CALL access_deleted();
} // namespace msg

namespace internal {
enum class exception_type_enum {
    exception,
    undefined_behavior,
    terminate,
};

/*!
 * これが std::set_terminate() に自動的に登録され、
 * 例外がcatchされずterminateした時にメッセージを出してくれる。
 *
 */
[[noreturn]] Y3C_DLL void Y3C_CALL handle_final_terminate_message() noexcept;
/*!
 * 例外を表示して強制終了する
 * 
 * 内部ではstd::terminate()ではなくstd::abort()を呼んでいる
 * 
 */
[[noreturn]] Y3C_DLL void Y3C_CALL do_terminate_with(exception_type_enum type,
                                                     const char *e_class,
                                                     std::string &&func,
                                                     std::string &&what);

/*!
 * y3cの例外クラスのベース。
 *
 * スタックトレースや例外の詳細をコンストラクタでy3c内部のグローバル変数に保存し、
 * what() は通常の例外と同様短いメッセージを返す。
 *
 */
struct Y3C_DLL exception_base {
    explicit exception_base(const char *e_class, std::string &&func,
                            std::string &&what);
    std::shared_ptr<void> detail;
    const char *what() const noexcept;
};

/*!
 * 通常は terminate(), undefined_behavior() で
 * std::terminate()を呼んで強制終了するが、
 * これがtrueの場合代わりにthrowするようになる
 * (主にテスト用)
 *
 * 投げた例外がdll境界を越えるとたまにめんどくさいことになるので、
 * 例外はinline関数の中で投げる
 *
 */
extern Y3C_DLL bool throw_on_terminate;

class exception_terminate {};
class exception_undefined_behavior {};

[[noreturn]] inline void terminate(std::string func, std::string what) {
    if (throw_on_terminate) {
        throw exception_terminate();
    } else {
        do_terminate_with(exception_type_enum::terminate, nullptr,
                          std::move(func), std::move(what));
    }
}
[[noreturn]] inline void undefined_behavior(std::string func,
                                            std::string what) {
    if (throw_on_terminate) {
        throw exception_undefined_behavior();
    } else {
        do_terminate_with(exception_type_enum::undefined_behavior, nullptr,
                          std::move(func), std::move(what));
    }
}

} // namespace internal

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
    out_of_range(std::string func, std::size_t size, long long index)
        : std::out_of_range(""),
          internal::exception_base("y3c::out_of_range", std::move(func),
                                   msg::out_of_range(size, index)) {}
    out_of_range(std::string func, std::size_t size, std::size_t index)
        : out_of_range(std::move(func), size, static_cast<long long>(index)) {}

    const char *what() const noexcept override {
        return internal::exception_base::what();
    }
};

Y3C_NS_END
