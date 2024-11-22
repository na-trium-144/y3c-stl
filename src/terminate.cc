#include "y3c/terminate.h"
#include "y3c/exception.h"
#include <exception>
#include <cpptrace/cpptrace.hpp>
#include <iostream>

namespace y3c {
namespace internal {

static bool throw_terminate = false;
void enable_throw_terminate() { throw_terminate = true; }

[[noreturn]] void terminate() {
    auto trace = cpptrace::generate_trace();
    while (!trace.frames.empty() &&
           (trace.frames.front().symbol.substr(0, 5) == "y3c::" ||
            trace.frames.front().symbol.find(" y3c::") != std::string::npos)) {
        trace.frames.erase(trace.frames.begin());
    }
    std::cerr << "y3c-stl: terminate called!!" << std::endl;
    trace.print_with_snippets(std::cerr);
    if (throw_terminate) {
        throw y3c::internal::exception_terminate();
    }
    std::terminate();
}

} // namespace internal
} // namespace y3c
