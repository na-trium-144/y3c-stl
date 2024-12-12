#include "y3c/exception.h"

namespace y3c {
namespace msg {
std::string out_of_range(std::size_t size, long long index) {
    std::ostringstream ss;
    ss << "attempted to access index " << index
       << ", that is outside the bounds of size " << size << ".";
    return ss.str();
}
std::string out_of_range(std::size_t size, std::size_t index) {
    std::ostringstream ss;
    ss << "attempted to access index " << index
       << ", that is outside the bounds of size " << size << ".";
    return ss.str();
}
std::string access_deleted() {
    return "attempted to access the deleted value.";
}
std::string access_nullptr() {
    return "attempted to access the value of nullptr.";
}
} // namespace msg
} // namespace y3c