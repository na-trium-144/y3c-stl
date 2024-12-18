#pragma once
#include <string>

// based on
// https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c/58331141#58331141

namespace y3c {
namespace internal {
namespace type_name {
template <typename T>
const std::string &wrapped_type_name() {
#ifdef __clang__
    static const std::string name = __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
    static const std::string name = __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
    static const std::string name = __FUNCSIG__;
#endif
    return name;
}

class probe_type;
const char *const probe_type_name = "y3c::internal::type_name::probe_type";
const char *const probe_type_name_elaborated =
    "class y3c::internal::type_name::probe_type";
inline const std::string &probe_type_name_used() {
    static const std::string name =
        wrapped_type_name<probe_type>().find(probe_type_name_elaborated) !=
                std::string::npos
            ? probe_type_name_elaborated
            : probe_type_name;
    return name;
}

inline size_t prefix_size() {
    return wrapped_type_name<probe_type>().find(probe_type_name_used());
}
inline size_t suffix_size() {
    return wrapped_type_name<probe_type>().length() - prefix_size() -
           probe_type_name_used().length();
}
} // namespace type_name

template <typename T>
const std::string &get_type_name() {
    static const std::string type_name = type_name::wrapped_type_name<T>();
    static const std::string name =
        type_name.substr(type_name::prefix_size(),
                         type_name.length() - type_name::prefix_size() -
                             type_name::suffix_size());
    return name;
}
} // namespace internal
} // namespace y3c
