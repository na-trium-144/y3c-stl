#include "y3c/terminate.h"
#include <cpptrace/basic.hpp>

Y3C_NS_BEGIN

void link() noexcept {}

namespace internal {

terminate_detail::terminate_detail(terminate_type type, const char *e_class, std::string &&func,
                               std::string &&what, skip_trace_tag)
    : type(type), e_class(e_class), func(std::move(func)),
      what(std::move(what)), raw_trace(std::make_shared<cpptrace::raw_trace>(cpptrace::generate_raw_trace())) {}

// グローバル変数初期化のタイミングでset_terminateを呼び、そのついでにfalseで初期化
bool throw_on_terminate =
    (std::set_terminate(handle_final_terminate_message), false);

} // namespace internal

Y3C_NS_END
