#include <y3c/wrap>
#include <iostream>

int main() {
    y3c::ptr<int> p;
    {
        y3c::wrap<int> a = 42;
        std::cout << *p << std::endl;
    }
    std::cout << *p << std::endl;
}
