#include <y3c/shared_ptr.h>
#include <y3c/exception.h>
#ifdef Y3C_DOCTEST_NESTED_HEADER
#include <doctest/doctest.h>
#else
#include <doctest.h>
#endif

struct A {
    A(int val) : val(val) {}
    int val;
};
struct B : A {
    B(int val) : A(val) {}
};

TEST_CASE("shared_ptr") {
    y3c::internal::enable_throw_terminate();

    y3c::ptr<A> p;
    {
        y3c::shared_ptr<A> a = std::make_shared<A>(42);
        CHECK_EQ(unwrap(*a).val, 42);
        CHECK_EQ(a->val, 42);
        CHECK_EQ(a.get()->val, 42);
        CHECK_EQ(a.use_count(), 1L);
        CHECK(static_cast<bool>(a));
        CHECK(static_cast<bool>(y3c::unwrap(a)));

        p = a.get();
        CHECK_EQ(p->val, 42);

        y3c::wrap_ref<A> r = *a;
        CHECK_EQ(unwrap(r).val, 42);

        a.reset();
        CHECK_THROWS_AS(unwrap(*a).val,
                        y3c::internal::exception_undefined_behavior);
        CHECK_THROWS_AS(a->val, y3c::internal::exception_undefined_behavior);
        CHECK_EQ(a.get(), nullptr);
        CHECK_EQ(a.use_count(), 0L);
        CHECK_FALSE(static_cast<bool>(a));
        CHECK_FALSE(static_cast<bool>(y3c::unwrap(a)));

        CHECK_THROWS_AS(p->val, y3c::internal::exception_undefined_behavior);
        CHECK_THROWS_AS(unwrap(r).val, y3c::internal::exception_undefined_behavior);

        p = a.get();
        CHECK_THROWS_AS(p->val, y3c::internal::exception_undefined_behavior);
    }

    y3c::shared_ptr<A> a1 = new A(42);
    y3c::ptr<A> p1 = a1.get();
    a1 = new A(42);
    CHECK_THROWS_AS(p1->val, y3c::internal::exception_undefined_behavior);
    p1 = a1.get();

    y3c::shared_ptr<A> a2 = new B(42);
    y3c::ptr<A> p2 = a2.get();
    a2 = new B(42);
    CHECK_THROWS_AS(p2->val, y3c::internal::exception_undefined_behavior);
    p2 = a2.get();

    y3c::shared_ptr<A> a3 = a2;
    y3c::ptr<A> p3 = a3.get();
    CHECK_EQ(p3->val, 42);
    a2.reset();
    CHECK_EQ(p3->val, 42);
    a3 = a1;
    CHECK_THROWS_AS(p3->val, y3c::internal::exception_undefined_behavior);
    p3 = a3.get();

    a1.reset();
    CHECK_EQ(p1->val, 42);
    CHECK_EQ(p3->val, 42);
    a3.reset();
    CHECK_THROWS_AS(p1->val, y3c::internal::exception_undefined_behavior);
    CHECK_THROWS_AS(p3->val, y3c::internal::exception_undefined_behavior);

    y3c::shared_ptr<B> b1 = new B(42);
    y3c::shared_ptr<A> a4 = b1;
    a4 = b1;
    y3c::shared_ptr<const A> ca1 = std::make_shared<const A>(42);
    ca1 = std::make_shared<const A>(42);
    y3c::shared_ptr<const A> ca2 = std::make_shared<A>(42);
    ca2 = std::make_shared<A>(42);
    y3c::shared_ptr<const A> ca3 = a1;
    ca3 = a1;
    y3c::shared_ptr<const A> ca4 = std::make_shared<const B>(42);
    ca4 = std::make_shared<const B>(42);
    y3c::shared_ptr<const A> ca5 = std::make_shared<B>(42);
    ca5 = std::make_shared<B>(42);
    y3c::shared_ptr<const A> ca6 = b1;
    ca6 = b1;


}
