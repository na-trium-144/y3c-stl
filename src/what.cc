#include "y3c/what.h"
#include <sstream>

Y3C_NS_BEGIN
namespace internal {
namespace what {
std::string out_of_range(std::size_t size, std::ptrdiff_t index) {
    std::ostringstream ss;
    ss << "attempted to access index " << index
       << ", that is outside the bounds of size " << size << ".";
    return ss.str();
}
const char *access_deleted() {
    return "attempted to access the deleted value.";
}
const char *access_nullptr() {
    return "attempted to access the value of nullptr.";
}
} // namespace what
} // namespace internal
Y3C_NS_END
