#pragma once
#ifdef Y3C_MESON
#include "y3c-config.h"
#else
#include "y3c/y3c-config.h"
#endif
#include <string>

namespace y3c {
namespace internal {
inline namespace Y3C_NS_ABI {

/*!
 * エラーメッセージの生成
 *
 */
namespace what {
Y3C_DLL std::string Y3C_CALL ub_out_of_range(std::size_t size,
                                             std::ptrdiff_t index);
Y3C_DLL std::string Y3C_CALL ub_out_of_range(std::size_t size,
                                             std::ptrdiff_t begin,
                                             std::ptrdiff_t end);
Y3C_DLL const char *Y3C_CALL ub_access_nullptr();
Y3C_DLL const char *Y3C_CALL ub_access_deleted();
Y3C_DLL const char *Y3C_CALL ub_wrong_iter();
Y3C_DLL const char *Y3C_CALL ub_invalid_iter();
Y3C_DLL const char *Y3C_CALL ub_iter_after_end();
Y3C_DLL const char *Y3C_CALL ub_iter_before_begin();
} // namespace what
} // namespace Y3C_NS_ABI
} // namespace internal
} // namespace y3c
