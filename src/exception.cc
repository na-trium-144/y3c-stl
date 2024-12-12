#include "y3c/exception.h"

namespace y3c {
namespace exception_std {

out_of_range::out_of_range(const char *func, std::size_t size,
                           std::size_t index)
    : std::out_of_range(""),
      internal::exception_base(func, msg::out_of_range(size, index)) {}
out_of_range::out_of_range(const char *func, std::size_t size, long long index)
    : std::out_of_range(""),
      internal::exception_base(func, msg::out_of_range(size, index)) {}

} // namespace exception_std
} // namespace y3c