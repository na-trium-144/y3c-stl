# y3c-stl (やさしいSTL)

STLの代わりに y3c:: 以下のクラスを使うことで、例外や未定義動作の代わりにわかりやすいエラーメッセージとスタックトレースを表示します。
C++初心者におすすめです。

```cpp
// use this instead of #include <array>
#include <y3c/array>

int main() {
    // use this instead of std::array<int, 5>
    y3c::array<int, 5> a;
    // this would usually throw std::out_of_range, but...
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

Abort trap: 6
```

## 仕様

* std:: 以下の各種クラスの代わりに y3c:: 以下のクラスで置き換えることで、よくある例外や未定義動作についていろいろな追加のチェックが実行時に行われます。
    * さらに`int`などのクラスでない変数については `y3c::wrap<int>` 、参照 `int&` → `y3c::wrap_ref<int>`、 生ポインタ `int*` → `y3c::ptr<int>` で置き換えることで、nullptrや範囲外へのアクセス、また寿命が切れた変数へのアクセスかどうかもチェックされます。
    * いずれも `y3c::unwrap()` (または `unwrap()`) 関数を使うことで対応する std:: 以下のクラス、wrap元の型(の参照)に復元できます。
    (unwrap後はチェックが行われませんが)
* 例外を投げるパターンの場合は (例えば `std::array::at()` → `std::out_of_range`)、
それがcatchされなかった場合にわかりやすいエラーメッセージとスタックトレースを表示します。
    * 通常の例外と同じようにcatchすることも可能です。(その場合は何も表示されません)
    * catchする場合はstdクラスの場合と同様に std:: の例外クラスでcatchできます。
    継承関係もそのままです。
    (例えば `std::out_of_range` は `std::runtime_error`, `std::exception` としてもcatchできます。)
* 未定義動作を起こすパターンの場合、エラーメッセージとスタックトレースを表示した後 terminate() します。

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
