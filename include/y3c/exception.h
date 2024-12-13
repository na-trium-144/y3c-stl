#pragma once
#include <stdexcept>
#include <sstream>

namespace y3c {
namespace msg {
std::string out_of_range(std::size_t size, std::size_t index);
std::string out_of_range(std::size_t size, long long index);
const char *access_nullptr();
const char *access_deleted();
} // namespace msg

namespace internal {
class exception_base {
    std::string message_;

  protected:
    explicit exception_base(const char *func, const std::string &message);
    const char *what() const noexcept;
};

class exception_terminate {};
class exception_undefined_behavior {};

[[noreturn]] void terminate(const char *func, const std::string &reason);
[[noreturn]] void undefined_behavior(const char *func,
                                     const std::string &reason);

/*!
 * 通常はterminate()はstd::terminate()を呼んで強制終了するが、
 * これを呼んでおくと代わりにthrowするようになる (主にテスト用)
 *
 */
void enable_throw_terminate();

} // namespace internal

namespace exception_std {
/*!
 * std:: のexceptionをそれぞれ継承しており、
 *  y3c:: の各exceptionの間に継承関係はない
 * ユーザーがcatchするのは std:: のほうで、
 *  y3c:: のクラスのまま直接catchすることは想定していない
 *
 */
class exception final : public std::exception, internal::exception_base {
  public:
    const char *what() const noexcept override {
        return this->internal::exception_base::what();
    }
};
class logic_error final : public std::logic_error, internal::exception_base {
  public:
    const char *what() const noexcept override {
        return this->internal::exception_base::what();
    }
};
class out_of_range final : public std::out_of_range, internal::exception_base {
  public:
    out_of_range(const char *func, std::size_t size, std::size_t index);
    out_of_range(const char *func, std::size_t size, long long index);
    const char *what() const noexcept override {
        return this->internal::exception_base::what();
    }
};
} // namespace exception_std
} // namespace y3c
