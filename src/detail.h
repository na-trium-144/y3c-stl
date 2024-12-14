#pragma once
#include "y3c/internal.h"
#include <cpptrace/basic.hpp>
#include <string>

Y3C_NS_BEGIN
namespace internal {

struct exception_detail {
    terminate_type type;
    const char *e_class;
    std::string func;
    std::string what;
    cpptrace::raw_trace raw_trace;

    exception_detail(terminate_type type, const char *e_class,
                     std::string &&func, std::string &&what,
                     cpptrace::raw_trace &&raw_trace);
};

} // namespace internal

Y3C_NS_END
