#include "y3c/what.h"
#include <sstream>

namespace y3c {
namespace internal {
inline namespace Y3C_NS_ABI {
namespace what {
std::string out_of_range(std::size_t size, std::ptrdiff_t index) {
    std::ostringstream ss;
    ss << "attempted to access index " << index
       << ", that is outside the bounds of size " << size << ".";
    return ss.str();
}
std::string out_of_range(std::size_t size, std::ptrdiff_t begin,
                         std::ptrdiff_t end) {
    std::ostringstream ss;
    ss << "attempted to access index range from " << begin << " to " << end
       << ", that is outside the bounds of size " << size << ".";
    return ss.str();
}
const char *access_deleted() {
    return "attempted to access the deleted value.";
}
const char *access_nullptr() {
    return "attempted to access the value of nullptr.";
}
const char *wrong_iter() {
    return "this iterator does not point inside this container.";
}
const char *invalid_iter() { return "this iterator is invalidated."; }
} // namespace what
} // namespace Y3C_NS_ABI
} // namespace internal
} // namespace y3c
