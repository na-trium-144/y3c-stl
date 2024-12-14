#include <y3c/array.h>
#include <iostream>

int main() {
    y3c::array<int, 1> a;
    try {
        a.at(100) = 0;
    } catch (const std::out_of_range &e) {
        std::cout << "catch as std::out_of_range, what(): " << e.what()
                  << std::endl;
    }
    try {
        a.at(100) = 0;
    } catch (const std::logic_error &e) {
        std::cout << "catch as std::logic_error, what(): " << e.what()
                  << std::endl;
    }
    try {
        a.at(100) = 0;
    } catch (const std::exception &e) {
        std::cout << "catch as std::exception, what(): " << e.what()
                  << std::endl;
    }
    try {
        a.at(100) = 0;
    } catch (...) {
        std::cout << "catch anything" << std::endl;
    }
}
