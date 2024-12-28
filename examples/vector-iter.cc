#include <y3c/vector>
#include <iostream>

int main() {
    y3c::vector<int> a = {1, 2, 3, 4, 5};
    y3c::vector<int>::iterator it = a.begin();
    it += 10;
    std::cout << *it << std::endl;
}
