#include <y3c/array>
#include <iostream>

int main() {
    y3c::ptr<int> p;
    {
        y3c::array<int, 5> a;
        p = &a[3];
        std::cout << *p << std::endl;
    }
    std::cout << *p << std::endl;
}
