#include <y3c/wrap>

int main() {
    y3c::wrap<int[5]> a;
    y3c::ptr<int> p = a;
    p += 10;
    *p = 5;
}
