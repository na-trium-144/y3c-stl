#include <y3c/wrap.h>
#include <y3c/terminate.h>
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
struct B : A {
    B(int val) : A(val) {}
};

TEST_CASE("wrap") {
    y3c::internal::throw_on_terminate = true;

    SUBCASE("wrap") {
        SUBCASE("ctor") {
            A base(100);
            y3c::wrap<A> *a;
            SUBCASE("default ctor") {
                a = new y3c::wrap<A>();
                CHECK_EQ(unwrap(*a).val, -1);
                CHECK_EQ(static_cast<A>(*a).val, -1);
            }
            SUBCASE("args") {
                SUBCASE("ctor") { a = new y3c::wrap<A>(100); }
                SUBCASE("assign") {
                    a = new y3c::wrap<A>();
                    *a = 100;
                }
                CHECK_EQ(unwrap(*a).val, 100);
                CHECK_EQ(static_cast<A>(*a).val, 100);
            }
            SUBCASE("copy base") {
                SUBCASE("ctor") { a = new y3c::wrap<A>(base); }
                SUBCASE("assign") {
                    a = new y3c::wrap<A>();
                    *a = base;
                }
                CHECK_EQ(unwrap(*a).val, 100);
            }
            SUBCASE("move base") {
                SUBCASE("ctor") { a = new y3c::wrap<A>(std::move(base)); }
                SUBCASE("assign") {
                    a = new y3c::wrap<A>();
                    *a = std::move(base);
                }
                CHECK_EQ(unwrap(*a).val, 100);
                CHECK_EQ(base.val, -1);
            }
            delete a;
        }
        SUBCASE("") {
            y3c::wrap<A> a(100);
            y3c::wrap<A> *b;
            SUBCASE("copy") {
                SUBCASE("ctor") { b = new y3c::wrap<A>(a); }
                SUBCASE("assign") {
                    b = new y3c::wrap<A>();
                    *b = a;
                }
                CHECK_EQ(unwrap(a).val, 100);
                CHECK_EQ(unwrap(*b).val, 100);
            }
            SUBCASE("move") {
                SUBCASE("ctor") { b = new y3c::wrap<A>(std::move(a)); }
                SUBCASE("assign") {
                    b = new y3c::wrap<A>();
                    *b = std::move(a);
                }
                CHECK_EQ(unwrap(a).val, -1);
                CHECK_EQ(unwrap(*b).val, 100);
            }
            delete b;
        }
    }
    SUBCASE("ref") {
        SUBCASE("ctor") {
            SUBCASE("same type") {
                y3c::wrap<A> a(100);
                y3c::wrap_ref<A> r(a);
                CHECK_EQ(unwrap(r).val, 100);
            }
            SUBCASE("inherited type") {
                y3c::wrap<B> b(100);
                y3c::wrap_ref<A> r(b);
                CHECK_EQ(unwrap(r).val, 100);
            }
        }
        SUBCASE("") {
            y3c::wrap<A> *a = new y3c::wrap<A>(100);
            y3c::wrap<A> b(50);
            y3c::wrap_ref<A> r(*a);
            CHECK_EQ(&unwrap(r), &unwrap(*a));
            CHECK_EQ(unwrap(r).val, 100);
            CHECK_EQ(static_cast<A &>(r).val, 100);
            SUBCASE("delete") {
                delete a;
                a = nullptr;
                CHECK_THROWS_AS(unwrap(r), y3c::internal::ub_access_deleted);
                CHECK_THROWS_AS(static_cast<A &>(r),
                                y3c::internal::ub_access_deleted);
            }
            SUBCASE("assign") {
                r = A(200);
                CHECK_EQ(unwrap(r).val, 200);
                CHECK_EQ(unwrap(*a).val, 200);
            }
            SUBCASE("copy ctor") {
                y3c::wrap_ref<A> r2(r);
                CHECK_EQ(&unwrap(r), &unwrap(r2));
                CHECK_EQ(unwrap(r2).val, 100);
            }
            SUBCASE("move ctor") {
                y3c::wrap_ref<A> r2(std::move(r));
                CHECK_EQ(&unwrap(r), &unwrap(r2));
                CHECK_EQ(unwrap(r2).val, 100);
                CHECK_EQ(unwrap(r).val, 100);
            }
            SUBCASE("copy assign") {
                y3c::wrap_ref<A> r2(b);
                r2 = r;
                CHECK_EQ(&unwrap(r2), &unwrap(b));
                CHECK_NE(&unwrap(r2), &unwrap(r));
                CHECK_EQ(unwrap(r).val, 100);
                CHECK_EQ(unwrap(r2).val, 100);
            }
            SUBCASE("move assign") {
                y3c::wrap_ref<A> r2(b);
                r2 = std::move(r);
                CHECK_EQ(&unwrap(r2), &unwrap(b));
                CHECK_NE(&unwrap(r2), &unwrap(r));
                CHECK_EQ(unwrap(r).val, -1);
                CHECK_EQ(unwrap(r2).val, 100);
            }
            if (a) {
                delete a;
            }
        }
    }
    SUBCASE("auto") {
        y3c::array<A, 2> aa{100, 200};
        y3c::wrap_auto<A> ar = aa[0];
        CHECK_EQ(unwrap(ar).val, 100);
        CHECK_EQ(static_cast<A &>(ar).val, 100);
        CHECK_EQ(&unwrap(ar), &unwrap(aa)[0]);
        SUBCASE("assign") {
            SUBCASE("value") {
                ar = A(200);
                CHECK_EQ(unwrap(ar).val, 200);
            }
            SUBCASE("copy auto") {
                y3c::wrap_auto<A> ar2 = aa[1];
                ar = ar2;
                CHECK_EQ(unwrap(ar).val, 200);
                CHECK_EQ(unwrap(ar2).val, 200);
                CHECK_NE(&unwrap(ar), &unwrap(ar2));
            }
            SUBCASE("move auto") {
                y3c::wrap_auto<A> ar2 = aa[1];
                ar = std::move(ar2);
                CHECK_EQ(unwrap(ar).val, 200);
                CHECK_EQ(unwrap(ar2).val, 200);
                CHECK_NE(&unwrap(ar), &unwrap(ar2));
            }
            CHECK_EQ(unwrap(aa[0]).val, 100);
            CHECK_NE(&unwrap(ar), &unwrap(aa)[0]);
        }
    }
    SUBCASE("ptr") {
        SUBCASE("ctor") {
            SUBCASE("same type") {
                y3c::wrap<A> a(100);
                y3c::ptr<A> p(&a);
                CHECK_EQ(unwrap(p)->val, 100);
            }
            SUBCASE("inherited type") {
                y3c::wrap<B> b(100);
                y3c::ptr<A> p(&b);
                CHECK_EQ(unwrap(p)->val, 100);
            }
            SUBCASE("from_ref") {
                y3c::wrap<A> a(100);
                y3c::wrap_ref<A> r(a);
                y3c::ptr<A> p(&r);
                CHECK_EQ(unwrap(p)->val, 100);
                CHECK_EQ(unwrap(p), &unwrap(r));
            }
            SUBCASE("from_auto") {
                y3c::array<A, 2> aa{100, 200};
                y3c::wrap_auto<A> ar = aa[0];
                y3c::ptr<A> p(&ar);
                CHECK_EQ(unwrap(p)->val, 100);
                CHECK_EQ(unwrap(p), &unwrap(ar));
            }
        }
        SUBCASE("void"){
            y3c::wrap<A> a;
            y3c::ptr<A> ap(&a);
            y3c::ptr<void> vp(&a);
            CHECK_EQ(unwrap(ap), unwrap(vp));
            CHECK_EQ(ap, vp);
        }
        SUBCASE("null") {
            y3c::wrap<A> a(100);
            y3c::ptr<A> *p;
            SUBCASE("default") { p = new y3c::ptr<A>(); }
            SUBCASE("nullptr") { p = new y3c::ptr<A>(nullptr); }
            SUBCASE("assign nullptr") {
                p = new y3c::ptr<A>(&a);
                *p = nullptr;
            }
            CHECK(!*p);
            CHECK_EQ(unwrap(*p), nullptr);
            CHECK_THROWS_AS((*p)->val, y3c::internal::ub_access_nullptr);
            delete p;
        }
        SUBCASE("") {
            y3c::wrap<A> *a = new y3c::wrap<A>(100);
            y3c::ptr<A> p(&*a);

            CHECK_EQ(unwrap(p), &unwrap(*a));
            CHECK_EQ(unwrap(p)->val, 100);

            CHECK_EQ(p->val, 100);
            SUBCASE("out of range") {
                CHECK_EQ(unwrap(p + 1), &unwrap(*a) + 1);
                CHECK_THROWS_AS((p + 1)->val, y3c::internal::ub_out_of_range);
                CHECK_EQ(unwrap(p - 1), &unwrap(*a) - 1);
                CHECK_THROWS_AS((p - 1)->val, y3c::internal::ub_out_of_range);
                CHECK_EQ(unwrap(p + 5) - unwrap(p), 5);
                // CHECK_EQ(unwrap(&p[5]), &unwrap(*a) + 5);
                CHECK_THROWS_AS(unwrap(p[5]).val,
                                y3c::internal::ub_out_of_range);

                SUBCASE("inc") {
                    SUBCASE("left") { CHECK_EQ(unwrap(++p), &unwrap(*a) + 1); }
                    SUBCASE("right") { CHECK_EQ(unwrap(p++), &unwrap(*a)); }
                    CHECK_EQ(unwrap(p), &unwrap(*a) + 1);
                }
                SUBCASE("dec") {
                    SUBCASE("left") { CHECK_EQ(unwrap(--p), &unwrap(*a) - 1); }
                    SUBCASE("right") { CHECK_EQ(unwrap(p--), &unwrap(*a)); }
                    CHECK_EQ(unwrap(p), &unwrap(*a) - 1);
                }
                SUBCASE("add") {
                    CHECK_EQ(unwrap(p += 5), &unwrap(*a) + 5);
                    CHECK_EQ(unwrap(p), &unwrap(*a) + 5);
                }
                SUBCASE("sub") {
                    CHECK_EQ(unwrap(p -= 5), &unwrap(*a) - 5);
                    CHECK_EQ(unwrap(p), &unwrap(*a) - 5);
                }
                CHECK_THROWS_AS(p->val, y3c::internal::ub_out_of_range);
            }

            SUBCASE("delete") {
                delete a;
                a = nullptr;
                CHECK_THROWS_AS(p->val, y3c::internal::ub_access_deleted);
            }
            SUBCASE("remains on") {
                SUBCASE("assign") {
                    *a = 200;
                    CHECK_EQ(p->val, 200);
                }
                SUBCASE("copied") {
                    *a = y3c::wrap<A>(200);
                    CHECK_EQ(p->val, 200);
                }
                SUBCASE("moved") {
                    *a = std::move(y3c::wrap<A>(200));
                    CHECK_EQ(p->val, 200);
                }
                SUBCASE("copy") {
                    y3c::wrap<A> b(*a);
                    CHECK_EQ(p->val, 100);
                }
                SUBCASE("move") {
                    y3c::wrap<A> b(std::move(*a));
                    CHECK_EQ(p->val, -1);
                }
                CHECK_EQ(unwrap(p), &unwrap(*a));
                CHECK_THROWS_AS((p - 1)->val, y3c::internal::ub_out_of_range);
                CHECK_THROWS_AS((p + 1)->val, y3c::internal::ub_out_of_range);
            }
            if (a) {
                delete a;
            }
        }
    }
}
