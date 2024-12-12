#include <y3c/wrap.h>

void some_func() {
    y3c::ptr<int> a = nullptr;
    int v = *a;
}
int main() { some_func(); }
