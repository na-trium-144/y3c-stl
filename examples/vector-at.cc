#include <y3c/vector>

int main() {
    y3c::vector<int> a = {1, 2, 3, 4, 5};
    a.at(100) = 42;
}
