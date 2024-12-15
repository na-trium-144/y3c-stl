#include <y3c/shared_ptr.h>
#include <y3c/internal.h>
#ifdef Y3C_DOCTEST_NESTED_HEADER
#include <doctest/doctest.h>
#else
#include <doctest.h>
#endif

struct A {
    A() = default;
    A(int val) : val(val) {}
    A(const A &) = default;
    A &operator=(const A &) = default;
    A(A &&other) : val(other.val) { other.val = -1; }
    A &operator=(A &&other) {
        val = other.val;
        other.val = -1;
        return *this;
    }
    int val = -1;
};
struct B : A {
    B(int val) : A(val) {}
};

TEST_CASE_TEMPLATE("shared_ptr ctor", P, y3c::shared_ptr<A>, y3c::shared_ptr<B>,
                   std::shared_ptr<A>, std::shared_ptr<B>) {
    using element_type = typename P::element_type; // A or B
    P a = std::make_shared<element_type>(100);
    y3c::shared_ptr<A> *p;
    SUBCASE("copy") {
        SUBCASE("ctor") { p = new y3c::shared_ptr<A>(a); }
        SUBCASE("assign") {
            p = new y3c::shared_ptr<A>();
            *p = a;
        }
        CHECK_EQ(unwrap(*p),
                 std::shared_ptr<A>(std::shared_ptr<element_type>(a)));
        CHECK_EQ(unwrap(*p)->val, 100);
    }
    SUBCASE("move") {
        SUBCASE("ctor") { p = new y3c::shared_ptr<A>(std::move(a)); }
        SUBCASE("assign") {
            p = new y3c::shared_ptr<A>();
            *p = std::move(a);
        }
        CHECK(!a);
        CHECK_EQ(unwrap(*p)->val, 100);
    }
    delete p;
}

TEST_CASE("shared_ptr") {
    y3c::internal::throw_on_terminate = true;

    SUBCASE("void") {
        auto a = y3c::make_shared<A>(100);
        y3c::shared_ptr<void> p = a;
        CHECK_EQ(unwrap(p), unwrap(a));
        CHECK_EQ(p, a);
    }
    SUBCASE("null") {
        y3c::shared_ptr<A> *p;
        SUBCASE("default") { p = new y3c::shared_ptr<A>(); }
        SUBCASE("nullptr") { p = new y3c::shared_ptr<A>(nullptr); }
        SUBCASE("assign nullptr") {
            p = new y3c::shared_ptr<A>(y3c::make_shared<A>());
            *p = nullptr;
        }
        SUBCASE("reset") {
            p = new y3c::shared_ptr<A>(y3c::make_shared<A>());
            p->reset();
        }
        CHECK(!*p);
        CHECK_EQ(unwrap(*p), nullptr);
        CHECK_THROWS_AS((*p)->val, y3c::internal::ub_access_nullptr);
        delete p;
    }
    SUBCASE("") {
        auto a = std::make_shared<A>(100);
        y3c::shared_ptr<A> p(a);
        y3c::ptr<A> rp(p.get());

        CHECK_EQ(unwrap(p), a);
        CHECK_EQ(unwrap(p)->val, 100);

        CHECK_EQ(p->val, 100);
        CHECK_EQ(unwrap(p.get()), a.get());
        CHECK_EQ(rp->val, 100);

        a.reset();

        SUBCASE("lifetime") {
            SUBCASE("reset") {
                p.reset();
                CHECK(!p);
                CHECK_EQ(unwrap(p), nullptr);
            }
            SUBCASE("assign") {
                p = std::make_shared<A>(200);
                CHECK_EQ(unwrap(p)->val, 200);
            }
            CHECK_THROWS_AS(rp->val, y3c::internal::ub_access_deleted);
        }
        SUBCASE("swap") {
            SUBCASE("wrapped") {
                auto p2 = y3c::make_shared<A>(200);
                swap(p, p2);
                CHECK_EQ(unwrap(p2)->val, 100);
                CHECK_EQ(unwrap(p)->val, 200);
                CHECK_EQ(rp->val, 100);

                p2.reset();
                CHECK_THROWS_AS(rp->val, y3c::internal::ub_access_deleted);
            }
            // todo: this doesn't work
            // SUBCASE("base") {
            //     auto p2 = std::make_shared<A>(200);
            //     swap(unwrap(p), p2);
            //     CHECK_EQ(p2->val, 100);
            //     CHECK_EQ(unwrap(p)->val, 200);
            //     CHECK_EQ(rp->val, 100);

            //     p2.reset();
            //     CHECK_THROWS_AS(rp->val, y3c::internal::ub_access_deleted);
            // }
        }
        SUBCASE("use_count") {
            CHECK_EQ(p.use_count(), 1);
            y3c::shared_ptr<A> p2 = p;
            CHECK_EQ(p.use_count(), 2);
            p2.reset();
            CHECK_EQ(p.use_count(), 1);
            p.reset();
            CHECK_EQ(p.use_count(), 0);
        }
    }
}
