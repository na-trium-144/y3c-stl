#include <y3c/wrap.h>
#include <iostream>

int main() {
    y3c::ptr<int> p;
    y3c::wrap<int> a = 42;
    p = &a;
    std::cout << p << ": " << *p << std::endl;
    p++;
    std::cout << p << ": " << *p << std::endl;
}
