#pragma once
#include "y3c/terminate.h"
#include <string>

// based on
// https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c/58331141#58331141

namespace y3c {
namespace internal {
namespace type_name {
template <typename T>
const char *wrapped_type_name() {
#if defined(__clang__) || defined(__GNUC__)
    return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
    return __FUNCSIG__;
#else
    return __func__;
#endif
}
template <typename T>
const std::string &wrapped_type_name_s() {
    static const std::string name = wrapped_type_name<T>();
    return name;
}

class probe_type;
const char *const probe_type_name_candidates[] = {
    "class y3c::internal::type_name::probe_type",
    "y3c::internal::type_name::probe_type",
    "probe_type",
};
inline const std::string &probe_type_name_used() {
    static std::string name;
    if (name.empty()) {
        for (const char *probe_type_name : probe_type_name_candidates) {
            if (wrapped_type_name_s<probe_type>().find(probe_type_name) !=
                std::string::npos) {
                name = probe_type_name;
                break;
            }
        }
        if (name.empty()) {
            internal::terminate_internal(
                "y3c::internal::type_name::probe_type_name_used()",
                "probe_type_name not found from signature: " +
                    wrapped_type_name_s<probe_type>());
        }
    }
    return name;
}

inline size_t prefix_size() {
    return wrapped_type_name_s<probe_type>().find(probe_type_name_used());
}
inline size_t suffix_size() {
    return wrapped_type_name_s<probe_type>().length() - prefix_size() -
           probe_type_name_used().length();
}
} // namespace type_name

template <typename T>
const std::string &get_type_name() {
    static std::string name;
    if (name.empty()) {
        const std::string &t_name = type_name::wrapped_type_name_s<T>();
        y3c_assert_internal(t_name.length() > type_name::prefix_size() +
                                                  type_name::suffix_size());
        name = t_name.substr(type_name::prefix_size(),
                             t_name.length() - type_name::prefix_size() -
                                 type_name::suffix_size());
        for (std::size_t i = 0;
             (i = name.find("class ", i)) != std::string::npos; i++) {
            if (i == 0 || (!std::isalnum(name[i - 1]) && name[i - 1] != '_')) {
                name = name.replace(i, 6, "");
            }
        }
        for (std::size_t i = 0;
             (i = name.find("struct ", i)) != std::string::npos; i++) {
            if (i == 0 || (!std::isalnum(name[i - 1]) && name[i - 1] != '_')) {
                name = name.replace(i, 7, "");
            }
        }
    }
    return name;
}
} // namespace internal
} // namespace y3c
