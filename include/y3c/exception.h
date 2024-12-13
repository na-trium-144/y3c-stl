#pragma once
#ifdef Y3C_MESON
#include "y3c-config.h"
#else
#include "y3c/y3c-config.h"
#endif
#include <stdexcept>
#include <sstream>

Y3C_NS_BEGIN
namespace msg {
Y3C_DLL std::string Y3C_CALL out_of_range(std::size_t size, std::size_t index);
Y3C_DLL std::string Y3C_CALL out_of_range(std::size_t size, long long index);
Y3C_DLL std::string Y3C_CALL access_nullptr();
Y3C_DLL std::string Y3C_CALL access_deleted();
} // namespace msg

namespace internal {
class Y3C_DLL exception_base {
    std::string message_;

  protected:
    explicit exception_base(const char *func, const std::string &message);
    const char *what() const noexcept;
};

class Y3C_DLL exception_terminate {};
class Y3C_DLL exception_undefined_behavior {};

[[noreturn]] Y3C_DLL void Y3C_CALL terminate(const char *func, const std::string &reason);
[[noreturn]] Y3C_DLL void Y3C_CALL undefined_behavior(const char *func, const std::string &reason);

/*!
 * 通常はterminate()はstd::terminate()を呼んで強制終了するが、
 * これを呼んでおくと代わりにthrowするようになる (主にテスト用)
 *
 */
Y3C_DLL void Y3C_CALL enable_throw_terminate();

} // namespace internal

namespace exception_std {
/*!
 * std:: のexceptionをそれぞれ継承しており、
 *  y3c:: の各exceptionの間に継承関係はない
 * ユーザーがcatchするのは std:: のほうで、
 *  y3c:: のクラスのまま直接catchすることは想定していない
 *
 */
class Y3C_DLL exception final : public std::exception, internal::exception_base {
  public:
    const char *what() const noexcept override {
        return this->internal::exception_base::what();
    }
};
class Y3C_DLL logic_error final : public std::logic_error, internal::exception_base {
  public:
    const char *what() const noexcept override {
        return this->internal::exception_base::what();
    }
};
class Y3C_DLL out_of_range final : public std::out_of_range, internal::exception_base {
  public:
    out_of_range(const char *func, std::size_t size, std::size_t index);
    out_of_range(const char *func, std::size_t size, long long index);
    const char *what() const noexcept override {
        return this->internal::exception_base::what();
    }
};
} // namespace exception_std
Y3C_NS_END
