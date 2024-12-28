#include <y3c/vector>

int main() {
    y3c::vector<int> a = {1, 2, 3, 4, 5};
    y3c::vector<int> b = {1, 2, 3, 4, 5};
    a.insert(b.begin(), 10);
}
