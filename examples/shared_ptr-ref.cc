#include <y3c/memory>
#include <iostream>

int main() {
    y3c::shared_ptr<int> a = std::make_shared<int>(100);
    y3c::wrap_ref<int> ref = *a;
    std::cout << ref << std::endl;
    a.reset();
    std::cout << ref << std::endl;
}
