#include <y3c/wrap>

int main() {
    y3c::wrap<int[5]> a;
    a[100] = 42;
}
