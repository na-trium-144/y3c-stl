#include "y3c/internal.h"
#include "./detail.h"

Y3C_NS_BEGIN

void link() noexcept {}

namespace internal {

[[noreturn]] void do_terminate_with(exception_type_enum type,
                                    const char *e_class, std::string &&func,
                                    std::string &&what) {
    get_global_storage().add_exception(type, e_class, std::move(func),
                                       std::move(what),
                                       cpptrace::generate_raw_trace());
    std::terminate();
}

exception_base::exception_base(const char *e_class, std::string &&func,
                               std::string &&what)
    : detail_id_(get_global_storage().add_exception(
          exception_type_enum::exception, e_class, std::move(func),
          std::move(what), cpptrace::generate_raw_trace())) {}
exception_base::exception_base(const exception_base &other)
    : detail_id_(get_global_storage().copy_exception(other.detail_id_)) {}
exception_base &exception_base::operator=(const exception_base &other) {
    if (this != &other) {
        detail_id_ = get_global_storage().copy_exception(other.detail_id_);
    }
    return *this;
}

exception_base::~exception_base() noexcept {
    get_global_storage().remove_exception(detail_id_);
}

bool throw_on_terminate = false;

exception_detail::exception_detail(exception_type_enum type,
                                   const char *e_class, std::string &&func,
                                   std::string &&what,
                                   cpptrace::raw_trace &&raw_trace)
    : type(type), e_class(e_class), func(std::move(func)),
      what(std::move(what)), raw_trace(std::move(raw_trace)) {}

global_storage::global_storage() {
    std::set_terminate(handle_final_terminate_message);
}

global_storage::~global_storage() {
    std::lock_guard<std::mutex> lock(m);
    initialized = false;
    exceptions.clear();
}

int global_storage::add_exception(exception_type_enum type, const char *e_class,
                                  std::string &&func, std::string &&what,
                                  cpptrace::raw_trace &&raw_trace) {
    if (!alive()) {
        return 0;
    }
    std::lock_guard<std::mutex> lock(m);
    if (!alive()) {
        return 0;
    }
    int detail_id = ++last_detail_id;
    exceptions.emplace(detail_id, std::make_shared<exception_detail>(
                                      type, e_class, std::move(func),
                                      std::move(what), std::move(raw_trace)));
    return detail_id;
}
int global_storage::copy_exception(int old_detail_id) {
    if (!alive()) {
        return 0;
    }
    std::lock_guard<std::mutex> lock(m);
    if (!alive()) {
        return 0;
    }
    int detail_id = ++last_detail_id;
    if (exceptions.count(old_detail_id)) {
        exceptions.emplace(detail_id, exceptions.at(old_detail_id));
    }
    return detail_id;
}
void global_storage::remove_exception(int detail_id) {
    if (!alive()) {
        return;
    }
    std::lock_guard<std::mutex> lock(m);
    if (!alive()) {
        return;
    }
    exceptions.erase(detail_id);
}

global_storage &get_global_storage() {
    static global_storage storage;
    return storage;
}

global_storage *storage_early_init = &get_global_storage();

} // namespace internal

Y3C_NS_END
