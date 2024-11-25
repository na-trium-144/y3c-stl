#include <y3c/shared_ptr.h>
#include <y3c/exception.h>
#include <gtest/gtest.h>

struct A {
    A(int val) : val(val) {}
    int val;
};

TEST(y3c, shared_ptr) {
    y3c::internal::enable_throw_terminate();

    {
        y3c::shared_ptr<A> a = std::make_shared<A>(42);
        EXPECT_EQ((*a).val, 42);
        EXPECT_EQ(a->val, 42);
        EXPECT_EQ(a.get()->val, 42);
        EXPECT_EQ(a.use_count(), 1L);
        EXPECT_TRUE(static_cast<bool>(a));
        EXPECT_TRUE(static_cast<bool>(y3c::unwrap(a)));

        a.reset();
        EXPECT_THROW((*a).val, y3c::internal::exception_terminate);
        EXPECT_THROW(a->val, y3c::internal::exception_terminate);
        EXPECT_EQ(a.get(), nullptr);
        EXPECT_EQ(a.use_count(), 0L);
        EXPECT_FALSE(static_cast<bool>(a));
        EXPECT_FALSE(static_cast<bool>(y3c::unwrap(a)));
    }
    {
        y3c::strict::shared_ptr<A> a2 = std::make_shared<A>(42);
        EXPECT_EQ((*a2).val, 42);
        EXPECT_EQ(a2->val, 42);
        EXPECT_EQ(a2.get()->val, 42);
        EXPECT_EQ(a2.use_count(), 1L);
        EXPECT_TRUE(static_cast<bool>(a2));
        EXPECT_TRUE(static_cast<bool>(y3c::unwrap(a2)));

        a2.reset();
        EXPECT_THROW((*a2).val, y3c::internal::exception_terminate);
        EXPECT_THROW(a2->val, y3c::internal::exception_terminate);
        EXPECT_THROW(a2.get(), y3c::internal::exception_terminate);
        EXPECT_EQ(a2.use_count(), 0L);
        EXPECT_FALSE(static_cast<bool>(a2));
        EXPECT_THROW(y3c::unwrap(a2), y3c::internal::exception_terminate);
    }
}
