#include <y3c/vector>
#include <iostream>

int main() {
    y3c::ptr<int> p;
    {
        y3c::vector<int> a = {1, 2, 3, 4, 5};
        p = &a[3];
        std::cout << *p << std::endl;
    }
    std::cout << *p << std::endl;
}
