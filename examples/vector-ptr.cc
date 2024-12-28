#include <y3c/vector>
#include <iostream>

int main() {
    y3c::vector<int> a;
    y3c::ptr<int> p = &a[3];
    p += 10;
    std::cout << *p << std::endl;
}
