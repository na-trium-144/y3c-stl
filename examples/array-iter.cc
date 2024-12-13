#include <y3c/array.h>
#include <iostream>

int main() {
    y3c::array<int, 5> a = {1, 2, 3, 4, 5};
    for(auto it = a.begin(); it <= a.end(); it++){
        std::cout << *it << std::endl;
    }
}
