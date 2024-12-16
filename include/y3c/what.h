#pragma once
#ifdef Y3C_MESON
#include "y3c-config.h"
#else
#include "y3c/y3c-config.h"
#endif
#include <string>

Y3C_NS_BEGIN

namespace internal {

/*!
 * エラーメッセージの生成
 *
 */
namespace what {
Y3C_DLL std::string Y3C_CALL out_of_range(std::size_t size,
                                          std::ptrdiff_t index);
Y3C_DLL const char *Y3C_CALL access_nullptr();
Y3C_DLL const char *Y3C_CALL access_deleted();
} // namespace what
} // namespace internal
Y3C_NS_END