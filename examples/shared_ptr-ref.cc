#include <y3c/shared_ptr.h>

void some_func() {
    y3c::shared_ptr<int> a = nullptr;
    [[maybe_unused]] int v = *a;
}
int main() { some_func(); }
