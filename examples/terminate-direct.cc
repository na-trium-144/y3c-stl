#include "y3c/internal.h"
#include <exception>

void func3() { std::terminate(); }
void func2() { func3(); }
void func1() { func2(); }
int main() {
    y3c::link();
    func1();
}
