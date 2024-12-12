#include <y3c/wrap.h>
#include <y3c/exception.h>
#include <doctest.h>

struct A {
    A(int val) : val(val) {}
    int val;
};
struct B : A {
    B(int val) : A(val) {}
};

TEST_CASE("wrap") {
    y3c::internal::enable_throw_terminate();

    y3c::ptr<A> p;
    {
        y3c::wrap<A> a{42};
        CHECK_EQ(unwrap(a).val, 42);

        p = &a;
        CHECK_EQ(p->val, 42);

        y3c::wrap_ref<A> r = a;
        CHECK_EQ(unwrap(r).val, 42);

        a = 100;
        CHECK_EQ(unwrap(a).val, 100);
        CHECK_EQ(p->val, 100);
        CHECK_EQ(unwrap(r).val, 100);

        y3c::wrap<A> b = 200;
        a = b;
        CHECK_EQ(unwrap(a).val, 200);
        CHECK_EQ(p->val, 200);
        CHECK_EQ(unwrap(r).val, 200);
    }

    CHECK_THROWS_AS(p->val, y3c::internal::exception_undefined_behavior);
}
