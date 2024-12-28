#include <y3c/vector>
#include <iostream>

int main() {
    y3c::vector<int> a = {1, 2, 3, 4, 5};
    y3c::ptr<int> p = &a[3];
    a.resize(100);
    std::cout << *p << std::endl;
}
