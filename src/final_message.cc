#include "y3c/terminate.h"
#include <cpptrace/basic.hpp>
#include <rang.hpp>
#include <iostream>
#include <ostream>

namespace y3c {
namespace internal {
inline namespace Y3C_NS_ABI {

void strip_and_print_trace(std::ostream &stream, cpptrace::stacktrace &trace) {
    while (!trace.frames.empty() &&
           (trace.frames.front().symbol.empty() ||
            trace.frames.front().symbol.find("y3c::internal::" Y3C_NS_ABI_S
                                             "::skip_trace_tag") !=
                std::string::npos ||
            trace.frames.front().symbol.find(
                "y3c::internal::" Y3C_NS_ABI_S
                "::handle_final_terminate_message") != std::string::npos)) {
        trace.frames.erase(trace.frames.begin());
    }
    while (!trace.frames.empty() && trace.frames.back().symbol.empty()) {
        trace.frames.erase(trace.frames.end() - 1);
    }
    if (trace.frames.empty()) {
        stream << rang::style::dim << rang::style::italic
               << "(No stack trace available. You may need to re-compile with "
                  "debug symbols enabled.)"
               << rang::style::reset << std::endl;
    } else {
        trace.print_with_snippets(stream);
    }
}

void print_header(std::ostream &stream) {
    stream << rang::style::bold << rang::fg::red << "y3c-stl terminated";
    stream << rang::style::reset << ": ";
}
void print_what(std::ostream &stream, const char *func, const char *what,
                bool quote = false) {
    if (func || what) {
        stream << "  ";
        if (func) {
            stream << rang::style::italic << rang::style::dim << "at ";
            stream << rang::style::reset << rang::style::italic
                   << rang::fg::yellow << func;
            stream << rang::fg::reset << rang::style::italic << rang::style::dim
                   << ": ";
            stream << rang::style::reset;
        }
        if (what) {
            if (quote) {
                stream << rang::style::dim << '"';
            }
            stream << rang::style::reset << what;
            if (quote) {
                stream << rang::style::dim << '"';
            }
        }
        stream << rang::style::reset << std::endl;
    }
}
void print_maybe_inaccurate(std::ostream &stream) {
    stream << rang::style::italic << rang::style::dim
           << "The following stack trace may be inaccurate.";
    stream << rang::style::reset << std::endl;
}
void print_y3c_exception(std::ostream &stream, const terminate_detail &e) {
    switch (e.type) {
    case terminate_type::exception:
        if (e.e_class) {
            stream << rang::style::bold << "exception of type ";
            stream << rang::fg::cyan << e.e_class;
            stream << rang::fg::reset << " thrown";
        } else {
            stream << rang::style::bold << "unsupported type exception thrown";
        }
        break;
    // case terminate_type::terminate:
    //     stream << rang::style::bold << "terminate() called";
    //     break;
    case terminate_type::internal:
        stream << rang::style::bold << "internal error of y3c-stl itself";
        break;
    case terminate_type::ub_out_of_range:
    case terminate_type::ub_access_nullptr:
    case terminate_type::ub_access_deleted:
        stream << rang::style::bold << "undefined behavior detected";
        break;
    default:
        stream << rang::style::italic << rang::fg::red
               << "error: invalid terminate type " << static_cast<int>(e.type);
        break;
    }
    stream << rang::style::reset << std::endl;

    print_what(stream, e.func.c_str(), e.what.c_str());
}

void print_current_exception(std::ostream &stream, std::exception_ptr current,
                             skip_trace_tag = {}) {
    try {
        std::rethrow_exception(current);
    } catch (const std::exception &e) {
        stream << "exception of type ";
        stream << rang::fg::cyan << "std::exception";
        stream << rang::fg::reset << " thrown, but that's not from y3c-stl."
               << std::endl;
        print_what(stream, nullptr, e.what(), true);
    } catch (const std::string &e) {
        stream << "exception of type ";
        stream << rang::fg::cyan << "std::string";
        stream << rang::fg::reset << " thrown, but that's not from y3c-stl."
               << std::endl;
        print_what(stream, nullptr, e.c_str(), true);
    } catch (const char *e) {
        stream << "exception of type ";
        stream << rang::fg::cyan << "const char *";
        stream << rang::fg::reset << " thrown, but that's not from y3c-stl."
               << std::endl;
        print_what(stream, nullptr, e, true);
    } catch (...) {
        stream
            << "unsupported type exception thrown, but that's not from y3c-stl."
            << std::endl;
        print_what(stream, nullptr, nullptr, false);
    }
    print_maybe_inaccurate(stream);
    auto trace = cpptrace::generate_trace();
    strip_and_print_trace(stream, trace);
}
[[noreturn]] void handle_final_terminate_message() noexcept {
    auto &stream = std::cerr;
    rang::setControlMode(rang::control::Force);
    print_header(stream);
    auto current = std::current_exception();
    if (current) {
        if (!exception_base::exceptions.empty()) {
            for (auto &detail : exception_base::exceptions) {
                print_y3c_exception(stream, detail.second);
                auto trace = std::static_pointer_cast<cpptrace::raw_trace>(
                                 detail.second.raw_trace)
                                 ->resolve();
                strip_and_print_trace(stream, trace);
            }
        } else {
            print_current_exception(stream, current);
        }
    } else {
        stream << "terminate() called, but that's not from y3c-stl and "
                  "current_exception information is empty."
               << std::endl;
        print_maybe_inaccurate(stream);
        auto trace = cpptrace::generate_trace();
        strip_and_print_trace(stream, trace);
    }
    std::abort();
}

[[noreturn]] void do_terminate_with(terminate_detail &&detail) {
    auto &stream = std::cerr;
    rang::setControlMode(rang::control::Force);
    print_header(stream);
    print_y3c_exception(stream, detail);
    auto trace = std::static_pointer_cast<cpptrace::raw_trace>(detail.raw_trace)
                     ->resolve();
    strip_and_print_trace(stream, trace);
    std::abort();
}

} // namespace Y3C_NS_ABI
} // namespace internal
} // namespace y3c