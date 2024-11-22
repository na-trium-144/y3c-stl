#pragma once
#include <stdexcept>

namespace y3c {
namespace internal {
class exception_base {};

class exception_terminate {};

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
    exception();
    ~exception() = default;
    const char *what() const noexcept override;
};
class logic_error final : public std::logic_error, internal::exception_base {
  public:
    logic_error();
    ~logic_error() = default;
    const char *what() const noexcept override;
};
class out_of_range final : public std::out_of_range, internal::exception_base {
  public:
    out_of_range() = default;
    ~out_of_range() = default;
    const char *what() const noexcept override;
};
} // namespace exception_std
} // namespace y3c
