#include <y3c/array.h>
#include <iostream>

int main() {
    y3c::array<int, 5> a;
    y3c::ptr<int> p = &a[3];
    p += 10;
    std::cout << *p << std::endl;
}
