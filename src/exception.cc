#include "y3c/exception.h"
#include <cpptrace/cpptrace.hpp>
#include <iostream>
#include <ostream>
#include <sstream>

namespace y3c {
namespace internal {
static bool throw_terminate = false;
void enable_throw_terminate() { throw_terminate = true; }

static void print_trace(std::ostream &stream) {
    auto trace = cpptrace::generate_trace();
    while (!trace.frames.empty() &&
           (trace.frames.front().symbol.empty() ||
            trace.frames.front().symbol.substr(0, 5) == "y3c::" ||
            trace.frames.front().symbol.find(" y3c::") != std::string::npos)) {
        trace.frames.erase(trace.frames.begin());
    }
    while (!trace.frames.empty() &&
           (trace.frames.back().symbol.empty() ||
            trace.frames.back().symbol.substr(0, 5) == "y3c::" ||
            trace.frames.back().symbol.find(" y3c::") != std::string::npos)) {
        trace.frames.erase(trace.frames.end() - 1);
    }
    trace.print_with_snippets(stream);
    if (trace.frames.empty()) {
        stream << "  (You may need to re-compile with debug symbols enabled.)"
               << std::endl;
    }
}

[[noreturn]] void terminate(const char *func, const char *reason) {
    std::cerr << "y3c-stl: terminate() called!!" << std::endl;
    std::cerr << "  at " << func << ", " << reason << std::endl;
    print_trace(std::cerr);
    if (throw_terminate) {
        throw y3c::internal::exception_terminate();
    }
    std::terminate();
}
[[noreturn]] void undefined_behavior(const char *func, const char *reason) {
    std::cerr << "y3c-stl: undefined behavior occurred!!" << std::endl;
    std::cerr << "  at " << func << ", " << reason << std::endl;
    print_trace(std::cerr);
    if (throw_terminate) {
        throw y3c::internal::exception_undefined_behavior();
    }
    std::terminate();
}
[[noreturn]] void ub_out_of_range(const char *func, std::size_t size,
                                  std::size_t index) {
    std::ostringstream ss;
    ss << "tried to access index " << index << ", that is larger than size "
       << size << ".";
    undefined_behavior(func, ss.str().c_str());
}
[[noreturn]] void ub_deleted(const char *func) {
    undefined_behavior(func, "tried to access the deleted value.");
}
[[noreturn]] void ub_nullptr(const char *func) {
    undefined_behavior(func, "tried to access the value of nullptr.");
}

exception_base::exception_base(const char *func, std::ostringstream &&ss) {
    ss << "\n";
    print_trace(ss);
    message_ = std::string("throwed at ") + func + ", " + ss.str();
}
const char *exception_base::what() const noexcept { return message_.c_str(); }

} // namespace internal

namespace exception_std {
out_of_range::out_of_range(const char *func, std::size_t size,
                           std::size_t index)
    : std::out_of_range(""),
      internal::exception_base(func, std::ostringstream()
                                         << "tried to access index " << index
                                         << ", that is larger than size "
                                         << size << ".") {}

} // namespace exception_std
} // namespace y3c