#include <y3c/array.h>

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

TEST_CASE_TEMPLATE("array ctor", P, y3c::array<A, 2>, std::array<A, 2>) {
    P base{100, 200};
    y3c::array<A, 2> *a;
    SUBCASE("copy") {
        SUBCASE("ctor") { a = new y3c::array<A, 2>(base); }
        SUBCASE("assign") {
            a = new y3c::array<A, 2>();
            *a = base;
        }
        CHECK_EQ(static_cast<A &>(base[0]).val, 100);
        CHECK_EQ(static_cast<A &>(base[1]).val, 200);
        CHECK_EQ(unwrap(*a)[0].val, 100);
        CHECK_EQ(unwrap(*a)[1].val, 200);
    }
    SUBCASE("move") {
        SUBCASE("ctor") { a = new y3c::array<A, 2>(std::move(base)); }
        SUBCASE("assign") {
            a = new y3c::array<A, 2>();
            *a = std::move(base);
        }
        CHECK_EQ(static_cast<A &>(base[0]).val, -1);
        CHECK_EQ(static_cast<A &>(base[1]).val, -1);
        CHECK_EQ(unwrap(*a)[0].val, 100);
        CHECK_EQ(unwrap(*a)[1].val, 200);
    }
    delete a;
}

TEST_CASE("array") {
    y3c::internal::throw_on_terminate = true;

    SUBCASE("default ctor") {
        y3c::array<A, 2> a;
        CHECK_EQ(unwrap(a)[0].val, -1);
        CHECK_EQ(unwrap(a)[1].val, -1);
    }
    SUBCASE("") {
        y3c::array<A, 2> a{100, 200};
        y3c::array<A, 0> e;

        CHECK_EQ(unwrap(a)[0].val, 100);
        CHECK_EQ(unwrap(a)[1].val, 200);

        CHECK_EQ(unwrap(a.at(0)).val, 100);
        CHECK_EQ(unwrap(a.at(1)).val, 200);
        CHECK_THROWS_AS(unwrap(a.at(2)).val, y3c::out_of_range);
        CHECK_THROWS_AS(unwrap(e.at(0)).val, y3c::out_of_range);

        CHECK_EQ(unwrap(a[0]).val, 100);
        CHECK_EQ(unwrap(a[1]).val, 200);
        CHECK_THROWS_AS(unwrap(a[2]).val, y3c::internal::ub_out_of_range);
        CHECK_THROWS_AS(unwrap(e[0]).val, y3c::internal::ub_out_of_range);

        CHECK_EQ(&unwrap(a.front()), &unwrap(a)[0]);
        CHECK_EQ(&unwrap(a.back()), &unwrap(a)[1]);
        CHECK_THROWS_AS(unwrap(e.front()), y3c::internal::ub_out_of_range);
        CHECK_THROWS_AS(unwrap(e.back()), y3c::internal::ub_out_of_range);

        CHECK_EQ(&unwrap(*a.data()), &unwrap(a)[0]);
        CHECK_EQ(&unwrap(*(a.data() + 1)), &unwrap(a)[1]);
        CHECK_THROWS_AS(unwrap(*(a.data() + 2)),
                        y3c::internal::ub_out_of_range);
        CHECK_THROWS_AS(unwrap(*(a.data() - 1)),
                        y3c::internal::ub_out_of_range);
        CHECK_THROWS_AS(unwrap(*(e.data())), y3c::internal::ub_out_of_range);

        CHECK_EQ(&unwrap(*a.begin()), unwrap(a).begin());
        CHECK_EQ(&unwrap(*a.cbegin()), unwrap(a).cbegin());
        CHECK_THROWS_AS(*a.end(), y3c::internal::ub_out_of_range);
        CHECK_THROWS_AS(*a.cend(), y3c::internal::ub_out_of_range);
        CHECK_EQ(unwrap(a.end()), unwrap(a).begin() + 2);
        CHECK_EQ(unwrap(a.cend()), unwrap(a).cbegin() + 2);
        CHECK_THROWS_AS(*e.begin(), y3c::internal::ub_out_of_range);
        CHECK_THROWS_AS(*e.cbegin(), y3c::internal::ub_out_of_range);
        CHECK_THROWS_AS(*e.end(), y3c::internal::ub_out_of_range);
        CHECK_THROWS_AS(*e.cend(), y3c::internal::ub_out_of_range);
    }
    SUBCASE("fill") {
        y3c::array<A, 2> a;
        a.fill(100);
        CHECK_EQ(unwrap(a)[0].val, 100);
        CHECK_EQ(unwrap(a)[1].val, 100);
    }
    SUBCASE("swap") {
        y3c::array<A, 2> a{100, 200};
        y3c::array<A, 2> b{300, 400};
        a.swap(b);
        CHECK_EQ(unwrap(a)[0].val, 300);
        CHECK_EQ(unwrap(a)[1].val, 400);
        CHECK_EQ(unwrap(b)[0].val, 100);
        CHECK_EQ(unwrap(b)[1].val, 200);
    }
}
