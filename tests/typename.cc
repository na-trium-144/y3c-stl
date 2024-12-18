#include <y3c/typename.h>
#ifdef Y3C_DOCTEST_NESTED_HEADER
#include <doctest/doctest.h>
#else
#include <doctest.h>
#endif

struct A {};

template <typename T>
struct B {};

TEST_CASE("typename") {
    CHECK_EQ(y3c::internal::get_type_name<int>(), "int");
    CHECK_EQ(y3c::internal::get_type_name<const int>(), "const int");
    CHECK_MESSAGE((y3c::internal::get_type_name<int *>() == "int*" ||
                   y3c::internal::get_type_name<int *>() == "int *"),
                  y3c::internal::get_type_name<int *>());
    CHECK_EQ(y3c::internal::get_type_name<A>(), "A");
    CHECK_EQ(y3c::internal::get_type_name<B<int>>(), "B<int>");
}
