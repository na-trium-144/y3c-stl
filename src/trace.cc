#include "y3c/exception.h"
#include <rang.hpp>
#include <cpptrace/cpptrace.hpp>
#include <iostream>
#include <ostream>
#include <string>

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

[[noreturn]] void terminate(const char *func, const std::string &reason) {
    std::cerr << rang::style::bold << rang::fg::red << "y3c-stl warn";
    std::cerr << rang::style::reset << ": ";
    std::cerr << rang::style::bold << "terminate() detected!!";
    std::cerr << rang::style::reset << std::endl;
    std::cerr << rang::style::italic << rang::style::dim << "  at ";
    std::cerr << rang::style::reset << rang::style::italic << rang::fg::yellow
              << func;
    std::cerr << rang::style::reset << rang::style::italic << rang::style::dim
              << ", ";
    std::cerr << rang::style::reset << reason << std::endl;
    print_trace(std::cerr);
    if (throw_terminate) {
        throw y3c::internal::exception_terminate();
    }
    std::terminate();
}
[[noreturn]] void undefined_behavior(const char *func,
                                     const std::string &reason) {
    std::cerr << rang::style::bold << rang::fg::red << "y3c-stl warn";
    std::cerr << rang::style::reset << ": ";
    std::cerr << rang::style::bold << "undefined behavior detected!!";
    std::cerr << rang::style::reset << std::endl;
    std::cerr << rang::style::italic << rang::style::dim << "  at ";
    std::cerr << rang::style::reset << rang::style::italic << rang::fg::yellow
              << func;
    std::cerr << rang::style::reset << rang::style::italic << rang::style::dim
              << ", ";
    std::cerr << rang::style::reset << reason << std::endl;
    print_trace(std::cerr);
    if (throw_terminate) {
        throw y3c::internal::exception_undefined_behavior();
    }
    std::terminate();
}

exception_base::exception_base(const char *func, const std::string &message) {
    rang::setControlMode(rang::control::Force);
    std::ostringstream ss;
    ss << rang::style::italic << rang::style::dim << "at ";
    ss << rang::style::reset << rang::style::italic << rang::fg::yellow << func;
    ss << rang::style::reset << rang::style::italic << rang::style::dim << ", ";
    ss << rang::style::reset << message << "\n";
    print_trace(ss);
    message_ = ss.str();
}
const char *exception_base::what() const noexcept { return message_.c_str(); }

} // namespace internal

} // namespace y3c