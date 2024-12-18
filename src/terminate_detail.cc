#include "y3c/terminate.h"
#include <cpptrace/basic.hpp>

namespace y3c {
namespace internal {
inline namespace Y3C_NS_ABI {

void link() noexcept {}


terminate_detail::terminate_detail(terminate_type type, const char *e_class,
                                   std::string &&func, std::string &&what,
                                   skip_trace_tag)
    : type(type), e_class(e_class), func(std::move(func)),
      what(std::move(what)), raw_trace(std::make_shared<cpptrace::raw_trace>(
                                 cpptrace::generate_raw_trace())) {}

// グローバル変数初期化のタイミングでset_terminateを呼び、そのついでにfalseで初期化
bool throw_on_terminate =
    (std::set_terminate(handle_final_terminate_message), false);

std::atomic<int> exception_base::last_exception_id;
std::unordered_map<int, terminate_detail> exception_base::exceptions;

} // namespace Y3C_NS_ABI
} // namespace internal
} // namespace y3c
