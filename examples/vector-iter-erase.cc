#include <y3c/vector>

int main() {
    y3c::vector<int> a = {1, 2, 3, 4, 5};
    for (y3c::vector<int>::iterator it = a.begin(); it < a.end(); ++it) {
        if (*it == 3) {
            a.erase(it);
        }
    }
}
