#include <y3c/wrap.h>
#include <iostream>

int main() {
    y3c::ptr<int> p;  // int *p;
    // std::cout << *p << std::endl;
    {
        y3c::wrap<int> a = 42;  // int a = 42;
        std::cout << a << std::endl;
        p = &a;
        std::cout << *p << std::endl;

        a = 100;
        std::cout << a << std::endl;
        std::cout << *p << std::endl;
    }
    std::cout << *p << std::endl;
}
