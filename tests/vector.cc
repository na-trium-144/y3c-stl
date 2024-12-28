#include <y3c/vector.h>
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

TEST_CASE_TEMPLATE("vector ctor", P, y3c::vector<A>, std::vector<A>) {
    P base{100, 200};
    y3c::vector<A> *a;
    SUBCASE("copy") {
        SUBCASE("ctor") { a = new y3c::vector<A>(base); }
        SUBCASE("assign") {
            a = new y3c::vector<A>();
            *a = base;
        }
        CHECK_EQ(static_cast<A &>(base[0]).val, 100);
        CHECK_EQ(static_cast<A &>(base[1]).val, 200);
        CHECK_EQ(unwrap(*a)[0].val, 100);
        CHECK_EQ(unwrap(*a)[1].val, 200);
    }
    SUBCASE("move") {
        SUBCASE("ctor") { a = new y3c::vector<A>(std::move(base)); }
        SUBCASE("assign") {
            a = new y3c::vector<A>();
            *a = std::move(base);
        }
        // CHECK(base.empty());
        CHECK_EQ(unwrap(*a)[0].val, 100);
        CHECK_EQ(unwrap(*a)[1].val, 200);
    }
    delete a;
}

TEST_CASE("vector") {
    y3c::internal::throw_on_terminate = true;

    SUBCASE("") {
        y3c::vector<A> a;
        y3c::vector<A> e;
        a.reserve(4);
        a.push_back(100);
        a.push_back(200);
        CHECK_EQ(a.capacity(), 4);

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

        CHECK_EQ(&unwrap(*a.begin()), &*unwrap(a).begin());
        CHECK_EQ(&unwrap(*a.cbegin()), &*unwrap(a).cbegin());
        CHECK_THROWS_AS(*a.end(), y3c::internal::ub_out_of_range);
        CHECK_THROWS_AS(*a.cend(), y3c::internal::ub_out_of_range);
        CHECK_EQ(unwrap(a.end()), &*unwrap(a).begin() + 2);
        CHECK_EQ(unwrap(a.cend()), &*unwrap(a).cbegin() + 2);
        CHECK_THROWS_AS(*e.begin(), y3c::internal::ub_out_of_range);
        CHECK_THROWS_AS(*e.cbegin(), y3c::internal::ub_out_of_range);
        CHECK_THROWS_AS(*e.end(), y3c::internal::ub_out_of_range);
        CHECK_THROWS_AS(*e.cend(), y3c::internal::ub_out_of_range);

        CHECK_EQ(a.size(), 2);
        CHECK_EQ(e.size(), 0);
        CHECK(!a.empty());
        CHECK(e.empty());

        auto a0 = a.begin();
        auto a1 = a0 + 1;
        auto a2 = a.end();
        CHECK_EQ(unwrap(*a0).val, 100);
        CHECK_EQ(unwrap(*a1).val, 200);
        CHECK_EQ(unwrap(*(a2 - 1)).val, 200);
        CHECK_THROWS_AS(unwrap(*a2).val, y3c::internal::ub_out_of_range);

        auto e0 = e.begin();
        auto e1 = e.end();
        CHECK_EQ(unwrap(e0), unwrap(e1));
        CHECK_EQ(e0, e1);
        CHECK_THROWS_AS(*e0, y3c::internal::ub_out_of_range);
        CHECK_THROWS_AS(*e1, y3c::internal::ub_out_of_range);

        SUBCASE("assign") {
            y3c::vector<A> b{300, 400};
            std::vector<A> c{300, 400};
            auto b0 = b.begin();
            auto b2 = b.end();
            SUBCASE("copy") {
                a = b;
                CHECK_EQ(unwrap(b0), unwrap(b.data()));
                CHECK_EQ(unwrap(b2), unwrap(b.data()) + 2);
                CHECK_EQ(unwrap(*b0).val, 300);
                CHECK_EQ(unwrap(*(b2 - 1)).val, 400);
            }
            SUBCASE("move") {
                a = std::move(b);
                CHECK_EQ(unwrap(b0), unwrap(a.data()));
                CHECK_EQ(unwrap(b2), unwrap(a.data()) + 2);
                CHECK_EQ(unwrap(*b0).val, 300);
                CHECK_EQ(unwrap(*(b2 - 1)).val, 400);
            }
            SUBCASE("base copy") { a = c; }
            SUBCASE("base move") { a = std::move(c); }
            SUBCASE("initializer list") { a = {300, 400}; }
            SUBCASE("assign iter") { a.assign(b.begin(), b.end()); }
            SUBCASE("assign initializer list") { a.assign({300, 400}); }
            CHECK_THROWS_AS(*a0, y3c::internal::ub_access_deleted);
            CHECK_THROWS_AS(*a1, y3c::internal::ub_access_deleted);
            CHECK_THROWS_AS(*a2, y3c::internal::ub_access_deleted);
            CHECK_THROWS_AS(a2 - 1, y3c::internal::ub_invalid_iter);
            CHECK_EQ(a.size(), 2);
            CHECK_EQ(unwrap(a[0]).val, 300);
            CHECK_EQ(unwrap(a[1]).val, 400);
        }
        SUBCASE("clear") {
            a.clear();
            CHECK_THROWS_AS(*a0, y3c::internal::ub_access_deleted);
            CHECK_THROWS_AS(*a1, y3c::internal::ub_access_deleted);
            CHECK_THROWS_AS(*a2, y3c::internal::ub_access_deleted);
            CHECK_THROWS_AS(a2 - 1, y3c::internal::ub_invalid_iter);
            CHECK(a.empty());
            CHECK_EQ(a.size(), 0);
        }
        SUBCASE("reserve") {
            SUBCASE("small") {
                a.reserve(1);
                CHECK_EQ(a.size(), 2);
                CHECK_EQ(a.capacity(), 4);
                CHECK_EQ(unwrap(*a0).val, 100);
                CHECK_EQ(unwrap(*(a2 - 1)).val, 200);
            }
            SUBCASE("no-allocate") {
                a.reserve(3);
                CHECK_EQ(a.size(), 2);
                CHECK_EQ(a.capacity(), 4);
                CHECK_EQ(unwrap(*a0).val, 100);
                CHECK_EQ(unwrap(*(a2 - 1)).val, 200);
            }
            SUBCASE("allocate") {
                a.reserve(100);
                CHECK_EQ(a.size(), 2);
                CHECK_EQ(a.capacity(), 100);
                CHECK_THROWS_AS(*a0, y3c::internal::ub_access_deleted);
                CHECK_THROWS_AS(*a1, y3c::internal::ub_access_deleted);
                CHECK_THROWS_AS(*a2, y3c::internal::ub_access_deleted);
                CHECK_THROWS_AS(a2 - 1, y3c::internal::ub_invalid_iter);
            }
        }
        // SUBCASE("shrink_to_fit"){}
        SUBCASE("erase") {
            CHECK_THROWS_AS(a.erase(a2), y3c::internal::ub_out_of_range);
            CHECK_THROWS_AS(a.erase(e0), y3c::internal::ub_wrong_iter);
            CHECK_THROWS_AS(a.erase(a1, e0), y3c::internal::ub_wrong_iter);
            CHECK_THROWS_AS(a.erase(e0, a1), y3c::internal::ub_wrong_iter);
            CHECK_THROWS_AS(
                a.erase(a2, a1),
                y3c::internal::ub_out_of_range); // out_of_rangeではない気がする

            y3c::vector<A>::iterator *a1_next;
            SUBCASE("iter") {
                a1_next = new y3c::vector<A>::iterator(a.erase(a1));
            }
            SUBCASE("range") {
                a1_next = new y3c::vector<A>::iterator(a.erase(a1, a2));
            }
            CHECK_EQ(unwrap(*a0).val, 100);
            CHECK_THROWS_AS(*a1, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(*a2, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(a1 - 1, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(a2 - 1, y3c::internal::ub_invalid_iter);
            CHECK_EQ(unwrap(*a1_next), unwrap(a1));
            CHECK_EQ(unwrap(*a1_next), unwrap(a0) + 1);
            CHECK_THROWS_AS(**a1_next, y3c::internal::ub_out_of_range);
            CHECK_EQ(unwrap(*(*a1_next - 1)).val, 100);
            CHECK_THROWS_AS(*a1_next + 1, y3c::internal::ub_iter_after_end);
            delete a1_next;
        }
        SUBCASE("push_back") {
            A v{300};
            SUBCASE("copy") {
                a.push_back(v);
                CHECK_EQ(v.val, 300);
            }
            SUBCASE("move") {
                a.push_back(std::move(v));
                CHECK_EQ(v.val, -1);
            }
            SUBCASE("emplace") { a.emplace_back(300); }
            CHECK_EQ(a.size(), 3);
            CHECK_EQ(unwrap(a[2]).val, 300);
            CHECK_EQ(unwrap(*a0).val, 100);
            CHECK_EQ(unwrap(*a1).val, 200);
            CHECK_THROWS_AS(*a2, y3c::internal::ub_invalid_iter);
            CHECK_EQ(unwrap(a2), &unwrap(a[2]));
        }
        SUBCASE("insert") {
            SUBCASE("end") {
                A v{300};
                a.insert(a2, v);
                CHECK_EQ(a.size(), 3);
                CHECK_EQ(unwrap(a[0]).val, 100);
                CHECK_EQ(unwrap(a[1]).val, 200);
                CHECK_EQ(unwrap(a[2]).val, 300);
                CHECK_EQ(unwrap(*a0).val, 100);
                CHECK_EQ(unwrap(*a1).val, 200);
                CHECK_THROWS_AS(*a2, y3c::internal::ub_invalid_iter);
                CHECK_EQ(unwrap(a2), &unwrap(a[2]));
            }
            SUBCASE("single") {
                A v{300};
                SUBCASE("copy") {
                    a.insert(a1, v);
                    CHECK_EQ(v.val, 300);
                }
                SUBCASE("move") {
                    a.insert(a1, std::move(v));
                    CHECK_EQ(v.val, -1);
                }
                SUBCASE("emplace") { a.insert(a1, 300); }
                CHECK_EQ(a.size(), 3);
                CHECK_EQ(unwrap(a[0]).val, 100);
                CHECK_EQ(unwrap(a[1]).val, 300);
                CHECK_EQ(unwrap(a[2]).val, 200);
                CHECK_EQ(unwrap(*a0).val, 100);
                CHECK_THROWS_AS(*a1, y3c::internal::ub_invalid_iter);
                CHECK_THROWS_AS(*a2, y3c::internal::ub_invalid_iter);
                CHECK_EQ(unwrap(a1), &unwrap(a[1]));
                CHECK_EQ(unwrap(a2), &unwrap(a[2]));
            }
            SUBCASE("multiple") {
                SUBCASE("copy") {
                    A v{300};
                    a.insert(a1, 2, v);
                    CHECK_EQ(v.val, 300);
                }
                SUBCASE("range") {
                    y3c::array<A, 2> b{300, 300};
                    a.insert(a1, b.begin(), b.end());
                }
                SUBCASE("initializer list") { a.insert(a1, {A(300), A(300)}); }
                CHECK_EQ(a.size(), 4);
                CHECK_EQ(unwrap(a[0]).val, 100);
                CHECK_EQ(unwrap(a[1]).val, 300);
                CHECK_EQ(unwrap(a[2]).val, 300);
                CHECK_EQ(unwrap(a[3]).val, 200);
                CHECK_EQ(unwrap(*a0).val, 100);
                CHECK_THROWS_AS(*a1, y3c::internal::ub_invalid_iter);
                CHECK_THROWS_AS(*a2, y3c::internal::ub_invalid_iter);
                CHECK_EQ(unwrap(a1), &unwrap(a[1]));
                CHECK_EQ(unwrap(a2), &unwrap(a[2]));
            }
        }
        SUBCASE("resize") {
            SUBCASE("small") {
                a.resize(1);
                CHECK_EQ(a.size(), 1);
                CHECK_EQ(unwrap(a0), &unwrap(a[0]));
                CHECK_EQ(unwrap(*a0).val, 100);
                CHECK_THROWS_AS(*a1, y3c::internal::ub_invalid_iter);
                CHECK_THROWS_AS(*a2, y3c::internal::ub_invalid_iter);
            }
            SUBCASE("no-allocate") {
                SUBCASE("default") {
                    a.resize(3);
                    CHECK_EQ(unwrap(a[2]).val, -1);
                }
                SUBCASE("value") {
                    a.resize(3, A(300));
                    CHECK_EQ(unwrap(a[2]).val, 300);
                }
                CHECK_EQ(a.size(), 3);
                CHECK_EQ(unwrap(a0), &unwrap(a[0]));
                CHECK_EQ(unwrap(*a0).val, 100);
                CHECK_EQ(unwrap(*a1).val, 200);
                CHECK_THROWS_AS(*a2, y3c::internal::ub_invalid_iter);
                CHECK_THROWS_AS(a2 - 1, y3c::internal::ub_invalid_iter);
            }
            SUBCASE("allocate") {
                SUBCASE("default") {
                    a.resize(100);
                    for (std::size_t i = 2; i < 100; i++) {
                        CHECK_EQ(unwrap(a[i]).val, -1);
                    }
                }
                SUBCASE("value") {
                    a.resize(100, A(300));
                    for (std::size_t i = 2; i < 100; i++) {
                        CHECK_EQ(unwrap(a[i]).val, 300);
                    }
                }
                CHECK_EQ(a.size(), 100);
                CHECK_NE(unwrap(a0), &unwrap(a[0]));
                CHECK_EQ(unwrap(a[0]).val, 100);
                CHECK_EQ(unwrap(a[1]).val, 200);
                CHECK_THROWS_AS(*a0, y3c::internal::ub_access_deleted);
                CHECK_THROWS_AS(*a1, y3c::internal::ub_access_deleted);
                CHECK_THROWS_AS(*a2, y3c::internal::ub_access_deleted);
                CHECK_THROWS_AS(a2 - 1, y3c::internal::ub_invalid_iter);
            }
        }
        SUBCASE("pop_back") {
            a.pop_back();
            CHECK_EQ(a.size(), 1);
            CHECK_EQ(unwrap(a0), &unwrap(a[0]));
            CHECK_EQ(unwrap(a[0]).val, 100);
            CHECK_EQ(unwrap(*a0).val, 100);
            CHECK_THROWS_AS(*a1, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(*a2, y3c::internal::ub_invalid_iter);
        }
    }
    SUBCASE("swap") {
        y3c::vector<A> a{100, 200};
        y3c::vector<A> b{300, 400};
        a.swap(b);
        CHECK_EQ(unwrap(a)[0].val, 300);
        CHECK_EQ(unwrap(a)[1].val, 400);
        CHECK_EQ(unwrap(b)[0].val, 100);
        CHECK_EQ(unwrap(b)[1].val, 200);
    }
}
