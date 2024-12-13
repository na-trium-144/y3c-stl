#include "y3c/exception.h"

Y3C_NS_BEGIN
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
const char *access_deleted() {
    return "attempted to access the deleted value.";
}
const char *access_nullptr() {
    return "attempted to access the value of nullptr.";
}
} // namespace msg
Y3C_NS_END
