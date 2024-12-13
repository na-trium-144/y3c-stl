#include <y3c/shared_ptr.h>

int main() {
    y3c::shared_ptr<int> a = nullptr;
    int v = *a;
}
