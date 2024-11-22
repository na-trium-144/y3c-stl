#pragma once

namespace y3c {
namespace internal {

[[noreturn]] void terminate();

/*!
 * 通常はterminate()はstd::terminate()を呼んで強制終了するが、
 * これを呼んでおくと代わりにthrowするようになる (主にテスト用)
 *
 */
void enable_throw_terminate();

} // namespace internal
} // namespace y3c
