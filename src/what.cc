#include "y3c/what.h"
#include <sstream>

namespace y3c {
namespace internal {
namespace what {
std::string ub_out_of_range(std::size_t size, std::ptrdiff_t index) {
    std::ostringstream ss;
    ss << "attempted to access index " << index
       << ", that is outside the bounds of size " << size << ".";
    return ss.str();
}
std::string ub_out_of_range(std::size_t size, std::ptrdiff_t begin,
                            std::ptrdiff_t end) {
    std::ostringstream ss;
    ss << "attempted to access index range from " << begin << " to " << end
       << ", that is invalid or outside the bounds of size " << size << ".";
    return ss.str();
}
const char *ub_access_deleted() {
    return "attempted to access the deleted value.";
}
const char *ub_access_nullptr() {
    return "attempted to access the value of nullptr.";
}
const char *ub_wrong_iter() {
    return "this iterator does not point inside this container.";
}
const char *ub_invalid_iter() { return "this iterator is invalidated."; }
const char *ub_iter_after_end() {
    return "iterated over the end() of container.";
}
const char *ub_iter_before_begin() {
    return "iterated back beyond the begin() of container.";
}
} // namespace what
} // namespace internal
} // namespace y3c
