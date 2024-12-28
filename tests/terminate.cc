#include <y3c/terminate.h>
#ifdef Y3C_DOCTEST_NESTED_HEADER
#include <doctest/doctest.h>
#else
#include <doctest.h>
#endif

TEST_CASE("terminate") {
    y3c::internal::throw_on_terminate = true;

    CHECK_THROWS_AS(y3c::internal::terminate_ub_out_of_range("", 0, 0),
                    y3c::internal::ub_out_of_range);
    CHECK_THROWS_AS(y3c::internal::terminate_ub_access_nullptr(""),
                    y3c::internal::ub_access_nullptr);
    CHECK_THROWS_AS(y3c::internal::terminate_ub_access_deleted(""),
                    y3c::internal::ub_access_deleted);
    CHECK_THROWS_AS(y3c::internal::terminate_ub_wrong_iter(""),
                    y3c::internal::ub_wrong_iter);
    CHECK_THROWS_AS(y3c::internal::terminate_ub_invalid_iter(""),
                    y3c::internal::ub_invalid_iter);
    CHECK_THROWS_AS(y3c::internal::terminate_ub_iter_after_end(""),
                    y3c::internal::ub_iter_after_end);
    CHECK_THROWS_AS(y3c::internal::terminate_ub_iter_before_begin(""),
                    y3c::internal::ub_iter_before_begin);
}
