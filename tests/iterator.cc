#include <y3c/terminate.h>
#include <y3c/iterator.h>
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

TEST_CASE("iter") {
    y3c::internal::throw_on_terminate = true;

    A array[5] = {100, 200, 300, 400, 500};
    A array2[4] = {500, 600, 700, 800};
    y3c::internal::life *life_ = new y3c::internal::life(&array[0], &array[4]);
    std::string name = "hoge";
    y3c::internal::contiguous_iterator<A> p(&array[1], life_->observer(),
                                            &name);
    y3c::internal::contiguous_iterator<A> p0(&array[0], life_->observer(),
                                             &name);
    y3c::internal::contiguous_iterator<A> p4(&array[4], life_->observer(),
                                             &name);

    CHECK_EQ(unwrap(p), &array[1]);
    CHECK_EQ(unwrap(*p).val, 200);
    CHECK_EQ(p->val, 200);

    CHECK_EQ(unwrap(p0 + 1), unwrap(p));
    CHECK_EQ(unwrap(p0), unwrap(p - 1));

    SUBCASE("out of range") {
        CHECK_THROWS_AS(p - 2, y3c::internal::ub_iter_before_begin);
        CHECK_EQ(unwrap(p - 1), &array[1] - 1);
        CHECK_EQ(unwrap(p + 3), &array[1] + 3);
        CHECK_THROWS_AS((p + 3)->val, y3c::internal::ub_out_of_range);
        CHECK_THROWS_AS(p[3], y3c::internal::ub_out_of_range);
        CHECK_THROWS_AS(p + 4, y3c::internal::ub_iter_after_end);
        CHECK_THROWS_AS(p[4], y3c::internal::ub_iter_after_end);

        CHECK_EQ(unwrap(p + 3) - unwrap(p), 3);

        SUBCASE("inc") {
            SUBCASE("left") {
                CHECK_EQ(unwrap(++p), &array[1] + 1);
                CHECK_THROWS_AS(++p4, y3c::internal::ub_iter_after_end);
            }
            SUBCASE("right") {
                CHECK_EQ(unwrap(p++), &array[1]);
                CHECK_THROWS_AS(p4++, y3c::internal::ub_iter_after_end);
            }
            CHECK_EQ(unwrap(p), &array[1] + 1);
        }
        SUBCASE("dec") {
            SUBCASE("left") {
                CHECK_EQ(unwrap(--p), &array[1] - 1);
                CHECK_THROWS_AS(--p0, y3c::internal::ub_iter_before_begin);
            }
            SUBCASE("right") {
                CHECK_EQ(unwrap(p--), &array[1]);
                CHECK_THROWS_AS(p0--, y3c::internal::ub_iter_before_begin);
            }
            CHECK_EQ(unwrap(p), &array[1] - 1);
        }
        SUBCASE("add") {
            CHECK_EQ(unwrap(p += 3), &array[1] + 3);
            CHECK_EQ(unwrap(p), &array[1] + 3);
            CHECK_THROWS_AS(p4 += 1, y3c::internal::ub_iter_after_end);
        }
        SUBCASE("sub") {
            CHECK_EQ(unwrap(p -= 1), &array[1] - 1);
            CHECK_EQ(unwrap(p), &array[1] - 1);
            CHECK_THROWS_AS(p0 -= 1, y3c::internal::ub_iter_before_begin);
        }
    }
    SUBCASE("delete") {
        SUBCASE("delete") {
            delete life_;
            life_ = nullptr;
        }
        SUBCASE("destroy") { life_->update(&array2[0], &array2[4]); }
        // assert_iter(): nullptr > deleted > invalidated > out_of_range
        CHECK_THROWS_AS(*p, y3c::internal::ub_access_deleted);
        CHECK_THROWS_AS(p->val, y3c::internal::ub_access_deleted);

        // update_iter(): invalidated > out_of_range
        CHECK_THROWS_AS(p + 1, y3c::internal::ub_invalid_iter);
        CHECK_THROWS_AS(p - 1, y3c::internal::ub_invalid_iter);
    }
    SUBCASE("invalidate") {
        SUBCASE("no change") {
            life_->update(&array[0], &array[4]);
            CHECK_EQ(unwrap(*p0).val, 100);
            CHECK_EQ(p0->val, 100);
            CHECK_EQ(unwrap(p0 + 1), unwrap(p));
            CHECK_EQ((p0 + 1)->val, 200);

            CHECK_THROWS_AS(*p4, y3c::internal::ub_out_of_range);
            CHECK_THROWS_AS(p4->val, y3c::internal::ub_out_of_range);
            CHECK_EQ(unwrap(p4 - 3), unwrap(p));
            CHECK_EQ(unwrap(*(p4 - 3)).val, 200);
            CHECK_EQ((p4 - 3)->val, 200);
        }
        SUBCASE("shrink") {
            life_->update(&array[1], &array[3]);
            CHECK_EQ(unwrap(p), &array[1]);
            CHECK_EQ(unwrap(*p).val, 200);
            CHECK_EQ(p->val, 200);

            CHECK_THROWS_AS(*p0, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(p0->val, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(*p4, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(p4->val, y3c::internal::ub_invalid_iter);

            CHECK_THROWS_AS(p0 - 1, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(p0 + 1, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(p0[1], y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(p0++, y3c::internal::ub_invalid_iter);

            CHECK_THROWS_AS(*p4, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(p4->val, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(p4 - 1, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(p4 + 1, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(p4[1], y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(p4--, y3c::internal::ub_invalid_iter);
        }
        SUBCASE("grow") {
            life_->update(&array[0], &array[5]);
            CHECK_EQ(unwrap(p), &array[1]);
            CHECK_EQ(unwrap(*p).val, 200);
            CHECK_EQ(p->val, 200);

            CHECK_EQ(unwrap(*p0).val, 100);
            CHECK_EQ(p0->val, 100);
            CHECK_EQ(unwrap(p0 + 4), unwrap(p4));
            CHECK_EQ(unwrap(*(p0 + 4)).val, 500);
            CHECK_EQ((p0 + 4)->val, 500);
            CHECK_THROWS_AS(*p4, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(p4->val, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(p4 - 1, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(p4 + 1, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(p4[1], y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(p4--, y3c::internal::ub_invalid_iter);
        }
        SUBCASE("invalidate_from") {
            life_->update(&array[0], &array[4], &array[1]);
            CHECK_EQ(unwrap(p), &array[1]);
            CHECK_THROWS_AS(unwrap(*p).val, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(p->val, y3c::internal::ub_invalid_iter);

            CHECK_EQ(unwrap(*p0).val, 100);
            CHECK_EQ(p0->val, 100);
            CHECK_EQ(unwrap(p0 + 1), unwrap(p));
            CHECK_EQ(unwrap(*(p0 + 1)).val, 200);
            CHECK_EQ((p0 + 1)->val, 200);
            CHECK_THROWS_AS(*p4, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(p4->val, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(p4 - 1, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(p4 + 1, y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(p4[1], y3c::internal::ub_invalid_iter);
            CHECK_THROWS_AS(p4--, y3c::internal::ub_invalid_iter);
        }
    }

    if (life_) {
        delete life_;
    }
}
