#include <y3c/shared_ptr.h>
#include <y3c/exception.h>
#include <doctest.h>

struct A {
    A(int val) : val(val) {}
    int val;
};

TEST_CASE("shared_ptr") {
    y3c::internal::enable_throw_terminate();

    {
        y3c::shared_ptr<A> a = std::make_shared<A>(42);
        CHECK_EQ((*a).val, 42);
        CHECK_EQ(a->val, 42);
        CHECK_EQ(a.get()->val, 42);
        CHECK_EQ(a.use_count(), 1L);
        CHECK(static_cast<bool>(a));
        CHECK(static_cast<bool>(y3c::unwrap(a)));

        a.reset();
        CHECK_THROWS_AS((*a).val, y3c::internal::exception_terminate);
        CHECK_THROWS_AS(a->val, y3c::internal::exception_terminate);
        CHECK_EQ(a.get(), nullptr);
        CHECK_EQ(a.use_count(), 0L);
        CHECK_FALSE(static_cast<bool>(a));
        CHECK_FALSE(static_cast<bool>(y3c::unwrap(a)));
    }
    {
        y3c::strict::shared_ptr<A> a2 = std::make_shared<A>(42);
        CHECK_EQ((*a2).val, 42);
        CHECK_EQ(a2->val, 42);
        CHECK_EQ(a2.get()->val, 42);
        CHECK_EQ(a2.use_count(), 1L);
        CHECK(static_cast<bool>(a2));
        CHECK(static_cast<bool>(y3c::unwrap(a2)));

        a2.reset();
        CHECK_THROWS_AS((*a2).val, y3c::internal::exception_terminate);
        CHECK_THROWS_AS(a2->val, y3c::internal::exception_terminate);
        CHECK_THROWS_AS(a2.get(), y3c::internal::exception_terminate);
        CHECK_EQ(a2.use_count(), 0L);
        CHECK_FALSE(static_cast<bool>(a2));
        CHECK_THROWS_AS(y3c::unwrap(a2), y3c::internal::exception_terminate);
    }
}
