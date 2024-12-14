#include <y3c/wrap.h>
#include <y3c/internal.h>
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

TEST_CASE("wrap") {
    y3c::internal::throw_on_terminate = true;

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

    {
        y3c::wrap<int> i = 1;
        y3c::ptr<int> p2 = &i;
        y3c::wrap_ref<int> r2 = i;
        bool e1 = *p2 == 1;
        CHECK(e1);
        CHECK_EQ(p2, p2);
        CHECK_EQ(p2, &r2);
        CHECK(p2);
        bool e2 = p2 == &unwrap(i);
        CHECK(e2);
        y3c::ptr<int> p3 = p2 + 1;
        CHECK_LT(p2, p3);
        p2++;
        CHECK_EQ(p2, p3);
        CHECK_THROWS_AS(*p2, y3c::internal::exception_undefined_behavior);
    }
    {
        y3c::wrap<int> a = 1;
        y3c::wrap<int> b = 2;
        y3c::wrap_ref<int> r = a;
        r = b; // 参照のセットではなく、値のコピー
        CHECK_EQ(unwrap(r), 2);
        CHECK_EQ(unwrap(a), 2);

        y3c::wrap_ref<int> r2 = r; // 同じものを参照する
        r2 = 3;
        CHECK_EQ(unwrap(r), 3);

        y3c::wrap_ref<int> r3 = std::move(r2); // ムーブはコピーと同じ
        r3 = 4;
        CHECK_EQ(unwrap(r), 4);
    }

    {
        y3c::wrap<int> a = 1;
        y3c::ptr<int> p = &a;
        auto r1 = *p; // wrap_auto
        r1 = 2;       // 参照切れる
        CHECK_EQ(unwrap(a), 1);
        CHECK_EQ(unwrap(*p), 1);
        CHECK_EQ(unwrap(r1), 2);

        y3c::wrap_ref<int> r3 = r1; // もうaの参照は持っていないが、r1を参照する
        r3 = 4;
        CHECK_EQ(unwrap(a), 1);
        CHECK_EQ(unwrap(*p), 1);
        CHECK_EQ(unwrap(r1), 4);
        CHECK_EQ(unwrap(r3), 4);

        y3c::wrap_ref<int> r2 = *p; // wrap_ref
        r2 = 3;                     // 参照のまま
        CHECK_EQ(unwrap(a), 3);
        CHECK_EQ(unwrap(*p), 3);
        CHECK_EQ(unwrap(r2), 3);
    }
}
