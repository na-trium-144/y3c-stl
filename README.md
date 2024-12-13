# y3c-stl (やさしいSTL)

std:: の標準ライブラリの代わりに y3c:: 以下のクラスを使うことで、標準ライブラリの例外や未定義動作の代わりにわかりやすいエラーメッセージとスタックトレースを表示します。
C++初心者におすすめです。

例えば範囲外アクセス:
```cpp
#include <y3c/array> // <- instead of #include <array>

int main() {
    y3c::array<int, 5> a; // <- instead of std::array<int, 5>
    a[100] = 42;  // this would be undefined behavior, but...
}
```
↓
```
y3c-stl terminated: undefined behavior detected
  at y3c::array::operator[](): attempted to access index 100, that is outside the bounds of size 5.
Stack trace (most recent call first):
#0 0x0000000104f6a16f in main at /Users/kou/projects/y3c-stl/build/../examples/array-operator.cc:5:5
       3: int main() {
       4:     y3c::array<int, 5> a;
       5:     a[100] = 42;
       6: }
Abort trap: 6
```

イテレータを使ったりしても...
```cpp
#include <y3c/array>
#include <iostream>

int main() {
    y3c::array<int, 5> a = {1, 2, 3, 4, 5};
    y3c::array<int, 5>::iterator it = a.begin();
    it += 10;
    std::cout << *it << std::endl;
}
```
↓
```
y3c-stl terminated: undefined behavior detected
  at y3c::array::iterator::operator*(): attempted to access index 10, that is outside the bounds of size 5.
Stack trace (most recent call first):
#0 0x0000000102929573 in main at /Users/kou/projects/y3c-stl/build/../examples/array-iter.cc:8:18
       6:     y3c::array<int, 5>::iterator it = a.begin();
       7:     it += 10;
       8:     std::cout << *it << std::endl;
       9: }
Abort trap: 6
```

生ポインタはさすがに対策できませんが、生ポインタと同様に使える `y3c::ptr<T>` で代用すれば...
```cpp
#include <y3c/array>
#include <iostream>

int main() {
    y3c::array<int, 5> a;
    y3c::ptr<int> p = &a[3];  // <- int *p = &a[3];
    p += 10;
    std::cout << *p << std::endl;
}
```
↓
```
y3c-stl terminated: undefined behavior detected
  at y3c::ptr::operator*(): attempted to access index 13, that is outside the bounds of size 5.
Stack trace (most recent call first):
#0 0x0000000102a31467 in main at /Users/kou/projects/y3c-stl/build/../examples/array-ptr.cc:8:18
       6:     y3c::ptr<int> p = &a[3];
       7:     p += 10;
       8:     std::cout << *p << std::endl;
       9: }
Abort trap: 6
```

範囲外だけでなく寿命切れも対策済みです。
```cpp
#include <y3c/array>
#include <iostream>

int main() {
    y3c::ptr<int> p;
    {
        y3c::array<int, 5> a;
        p = &a[3];
        std::cout << *p << std::endl;
    }
    std::cout << *p << std::endl;
}
```
↓
```
0
y3c-stl terminated: undefined behavior detected
  at y3c::ptr::operator*(): attempted to access the deleted value.
Stack trace (most recent call first):
#0 0x0000000100195337 in main at /Users/kou/projects/y3c-stl/build/../examples/array-ptr-local.cc:11:18
       9:         std::cout << *p << std::endl;
      10:     }
      11:     std::cout << *p << std::endl;
      12: }
Abort trap: 6
```

## 仕様

* std:: 以下の各種クラスの代わりに y3c:: 以下のクラスで置き換えることで、よくある例外や未定義動作についていろいろな追加のチェックが実行時に行われます。
    * さらに`int`などのクラスでない変数については `y3c::wrap<int>` 、参照 `int&` → `y3c::wrap_ref<int>`、 生ポインタ `int*` → `y3c::ptr<int>` で置き換えることで、nullptrや範囲外へのアクセス、また寿命が切れた変数へのアクセスかどうかをチェックすることができます。
    * いずれも `y3c::unwrap()` (または `unwrap()`) 関数を使うことで対応する std:: 以下のクラス、wrap元の型(の参照)に復元できます。
    (unwrap後はチェックが行われませんが)
* 例外を投げるパターンの場合は (例えば `std::array::at()` → `std::out_of_range`)、
それがcatchされなかった場合にわかりやすいエラーメッセージとスタックトレースを表示します。
    * 通常の例外と同じようにcatchすることも可能です。(その場合は何も表示されません)
    * catchする場合はstdクラスの場合と同様に std:: の例外クラスでcatchできます。
    継承関係もそのままです。
    (例えば `std::out_of_range` は `std::runtime_error`, `std::exception` としてもcatchできます。)
* 未定義動作を起こすパターンの場合、エラーメッセージとスタックトレースを表示した後 terminate() します。
* スタックトレースの表示処理は `std::set_terminate()` に登録したハンドラーの中で行っているため、
y3c:: のラッパークラス以外が投げた標準のexceptionや、直接 `std::terminate()` を呼び出した場合もスタックトレースが表示できる場合があります。

### チェック対象の処理

* nullptrアクセスのチェック
    * 未初期化の `y3c::ptr`, shared_ptr など
* 範囲外アクセスのチェック
    * arrayなどのコンテナ型 (`at()` も `[]` でのアクセスも同様)
    * arrayなどのイテレータ
    * arrayなどの要素を指すポインタ
* 寿命が切れた値へのアクセスのチェック
    * イテレータ、ポインタ、参照など

ただしいずれもチェックされる条件として生ポインタや参照の代わりに `y3c::ptr`, `y3c::wrap_ref` などを使用することに加えて、
アクセスする対象も y3c:: のクラスまたは `y3c::wrap` でラップされている必要があります
