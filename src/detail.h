#pragma once
#include "y3c/internal.h"
#include <cpptrace/basic.hpp>
#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>

Y3C_NS_BEGIN
namespace internal {

struct exception_detail {
    exception_type_enum type;
    const char *e_class;
    std::string func;
    std::string what;
    cpptrace::raw_trace raw_trace;

    exception_detail(exception_type_enum type, const char *e_class,
                     std::string &&func, std::string &&what,
                     cpptrace::raw_trace &&raw_trace);
};

/*!
 * exceptionの情報などを保存するクラス。
 * static変数のシングルトン。
 *
 * これが破棄される時、ただのbool変数であるinitializedがfalseになり、判別できるようにする。
 * その場合exception情報のアクセスは諦める
 *
 */
class global_storage {
    std::unordered_map<int, std::shared_ptr<exception_detail>> exceptions;
    int last_detail_id = 0;
    std::mutex m;
    bool initialized = true;

  public:
    global_storage();
    global_storage(const global_storage &) = delete;
    global_storage &operator=(const global_storage &) = delete;
    global_storage(global_storage &&) = delete;
    global_storage &operator=(global_storage &&) = delete;
    ~global_storage();

    bool alive() const noexcept { return initialized; }
    int add_exception(exception_type_enum type, const char *e_class,
                      std::string &&func, std::string &&what,
                      cpptrace::raw_trace &&raw_trace);
    void remove_exception(int detail_id);
    int copy_exception(int old_detail_id);

    template <typename T1, typename T2, typename T3>
    void foreach_exception(T1 &&foreach_f, T2 &&on_empty_f, T3 &&on_dead_f) {
        if (!alive()) {
            on_dead_f();
            return;
        }
        std::lock_guard<std::mutex> lock(m);
        if (!alive()) {
            on_dead_f();
            return;
        }
        if (exceptions.empty()) {
            on_empty_f();
            return;
        }
        for (const auto &e : exceptions) {
            foreach_f(e.second);
        }
    }
};

global_storage &get_global_storage();

/*!
 * できるだけ早いタイミングで初期化するために、グローバル変数初期化としてget_global_storage()を呼び出す。
 *
 * これ自体は使わない
 *
 */
extern global_storage *storage_early_init;

} // namespace internal

Y3C_NS_END
