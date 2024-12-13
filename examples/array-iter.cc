#include <y3c/array.h>
#include <iostream>

int main() {
    y3c::array<int, 5> a = {1, 2, 3, 4, 5};
    y3c::array<int, 5>::iterator it = a.begin();
    it += 10;
    std::cout << *it << std::endl;
}
