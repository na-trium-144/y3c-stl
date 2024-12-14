#include "y3c/internal.h"
#include "./detail.h"

Y3C_NS_BEGIN

void link() noexcept {}

namespace internal {

exception_base::exception_base(const char *e_class, std::string &&func,
                               std::string &&what, skip_trace_tag)
    : detail(std::make_shared<exception_detail>(
          terminate_type::exception, e_class, std::move(func), std::move(what),
          cpptrace::generate_raw_trace())) {}

const char *exception_base::what() const noexcept {
    return std::static_pointer_cast<exception_detail>(this->detail)
        ->what.c_str();
}

// グローバル変数初期化のタイミングでset_terminateを呼び、そのついでにfalseで初期化
bool throw_on_terminate =
    (std::set_terminate(handle_final_terminate_message), false);

exception_detail::exception_detail(terminate_type type, const char *e_class,
                                   std::string &&func, std::string &&what,
                                   cpptrace::raw_trace &&raw_trace)
    : type(type), e_class(e_class), func(std::move(func)),
      what(std::move(what)), raw_trace(std::move(raw_trace)) {}

} // namespace internal

Y3C_NS_END
