# y3c-stl (やさしいSTL)

C++ STL wrapper with friendly error messages and automatic stack trace on exceptions, terminate(), or UB.

```cpp
// use this instead of #include <array>
#include <y3c/array>

int main() {
    // use this instead of std::array<int, 5>
    y3c::array<int, 5> a;
    // this would usually throw std::out_of_range
    a.at(100) = 42;
}
```

Example output (on MacOS):
```
libc++abi: terminating due to uncaught exception of type y3c::exception_std::out_of_range: throwed at y3c::array::at(): got number 100, that is larger than size 5.
Stack trace (most recent call first):
#0 0x0000000102687c13 in main at /Users/kou/projects/y3c-stl/build/../examples/array-at.cc:5:7
       3: int main() {
       4:     y3c::array<int, 5> a;
       5:     a.at(100) = 42;
       6: }
#1 0x000000019290b153 at /usr/lib/dyld

Abort trap: 6
```

Also handles some cases that would usually get undefined behaviors.
```cpp
int main() {
    y3c::array<int, 5> a;
    a[100] = 42;
}
```

```
y3c-stl: undefined behavior occurred!!
  at y3c::array::operator[](), got number 100, that is larger than size 5.
Stack trace (most recent call first):
#0 0x000000010227feaf in main at /Users/kou/projects/y3c-stl/build/../examples/array-operator.cc:5:5
       3: int main() {
       4:     y3c::array<int, 5> a;
       5:     a[100] = 42;
       6: }
#1 0x000000019290b153 at /usr/lib/dyld
libc++abi: terminating
Abort trap: 6
```
