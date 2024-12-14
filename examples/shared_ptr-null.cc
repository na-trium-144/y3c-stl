#include <y3c/memory>

int main() {
    y3c::shared_ptr<int> a = nullptr;
    int v = *a;
}
