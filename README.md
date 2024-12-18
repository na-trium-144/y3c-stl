# y3c-stl (やさしいSTL)

std:: の標準ライブラリの代わりに y3c:: 以下のクラスを使うことで、例外がcatchされなかった場合や未定義動作の代わりにわかりやすいエラーメッセージとスタックトレースを表示します。
C++初心者におすすめです。

Linux, MacOS, Windows (MSVC, MinGW) で動作確認しています。
スタックトレースの取得と表示に関しては [cpptrace](https://github.com/jeremy-rifkin/cpptrace) ライブラリを使用しています。

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

イテレータや参照、ポインタなどを雑に使っても大丈夫です。
(生参照や生ポインタはさすがに対策できないのでそれらと同等の `y3c::wrap<T&>`, `y3c::wrap<T*>` を用意しています)
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

また、範囲外だけでなく寿命切れやnullptrアクセスなど他の未定義動作も対策済みです。
```cpp
#include <y3c/array>
#include <iostream>

int main() {
    y3c::ptr<int> p;  // <- instead of int &p;
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

## インストール

Meson (>=0.56) をインストールしてください。

* Ubuntu: `pip install meson` または `sudo apt install meson`
* MacOS: `brew install meson`
* Windows MSVC: [公式サイト](https://mesonbuild.com/Getting-meson.html) からダウンロードしてください。
* MSYS2: `pacboy -S meson`

以下のコマンドでy3c-stlと依存ライブラリをビルド、インストールできます。

```sh
git clone https://github.com/na-trium-144/y3c-stl
cd y3c-stl
meson setup build
meson install -Cbuild --skip-subprojects
```

Windowsの場合は meson setup 時の引数で `-Dprefix=C:/Users/hoge/y3c` などとインストール先を指定するとよいです。
Linux,Macの場合は何も指定せずデフォルトのまま (`/usr/local` など) がおすすめです。

また、Windows MSVC の場合はdebugビルド(デフォルト)とreleaseビルド(meson setup 時に `-Dbuildtype=release` を指定)の両方をインストールする必要があります。

## 使い方

* y3c-stlは y3c という名前の共有ライブラリとなっているのでそれをリンクするだけでokです。
    * ただしWindowsのMSVCでdebugビルドの場合は y3cd という名前になります。
* また、Windowsの場合はy3cのdllファイル(インストール場所のbinディレクトリにあります)を実行ファイルと同じディレクトリにコピーするか、PATHを通す必要があります。

### 直接コンパイラの引数で指定

* Linux,Macでインストール先がデフォルトの場合は `-ly3c` を渡せばよいです。
    * インストール先を変更している場合は `-I/path/to/y3c/include -L/path/to/y3c/lib -ly3c` などとパスも指定する必要があります。
* Windows MSVCの場合は `/IC:\path\to\y3c\include` と `/libpath:C:\path\to\y3c\lib` でパスを指定し、
`y3c.lib`(releaseビルドの場合) または `y3cd.lib`(debugビルドの場合) を渡せば良いです。
両方渡せば確実です(内部でnamespaceを分けているので混ぜても問題ありません)。

### pkg-config

* コンパイラの引数に `$(pkg-config --cflags --libs y3c)` を渡せば良いです。
    * インストール先を変更している場合は `PKG_CONFIG_PATH` 環境変数に `/path/to/y3c/lib/pkgconfig` などを追加する必要があります。

### CMake

```cmake
find_package(y3c REQUIRED)
target_link_libraries(your_target y3c::y3c)
```

### Meson

```meson
y3c_dep = dependency('y3c')
executable('your_target', ..., dependencies: y3c_dep)
```

## 仕様

* std:: 以下の各種クラスの代わりに y3c:: 以下のクラスで置き換えることで、よくある例外や未定義動作についていろいろな追加のチェックが実行時に行われます。
    * さらに`int`などのクラスでない変数については `y3c::wrap<int>` 、参照 `int&` → `y3c::wrap<int&>`、 生ポインタ `int*` → `y3c::wrap<int*>` で置き換えることで、nullptrや範囲外へのアクセス、また寿命が切れた変数へのアクセスかどうかをチェックすることができます。
    * いずれも `y3c::unwrap()` (または `unwrap()`) 関数を使うことで対応する std:: 以下のクラス、wrap元の型(の参照)に復元できます。
    (unwrap後はチェックが行われませんが)
* 例外を投げるパターンの場合は (例えば `std::array::at()` → `std::out_of_range`)、
それがcatchされなかった場合にわかりやすいエラーメッセージとスタックトレースを表示します。
    * 通常の例外と同じようにcatchすることも可能です。(その場合は何も表示されません)
    * catchする場合はstdクラスの場合と同様に std:: の例外クラスでcatchできます。
    継承関係もそのままです。
    (例えば `std::out_of_range` は `std::logic_error`, `std::exception` としてもcatchできます。)
* 未定義動作を起こすパターンの場合、エラーメッセージとスタックトレースを表示した後 abort() します。
* スタックトレースの表示処理は `std::set_terminate()` に登録したハンドラーの中で行っているため、
y3c:: のラッパークラス以外が投げた標準のexceptionや、直接 `std::terminate()` を呼び出した場合もスタックトレースが表示できる場合があります。

### チェック対象の処理

* nullptrアクセスのチェック
* 範囲外アクセスのチェック
* 寿命が切れた値へのアクセスのチェック

ただしいずれもチェックされる条件として生ポインタや参照の代わりに `y3c::ptr`, `y3c::wrap_ref` などを使用することに加えて、
アクセスする対象も y3c:: のクラスまたは `y3c::wrap` でラップされている必要があります

### 実装済みの関数、クラス一覧

todo: doxygenのドキュメントを作りそれへのリンクにするとか

* `#include <y3c/wrap>` (y3c独自のユーティリティ関数、STLのクラスに属さないものなど)
    * `y3c::link()`
    * `y3c::unwrap(y3c::wrap<T>)`
    * `y3c::wrap<T>` ← `T`
        * `y3c::wrap_ref<T>` = `y3c::wrap<T&>`
        * `y3c::const_wrap_ref<T>` = `y3c::wrap<const T&>`
        * `y3c::ptr<T>` = `y3c::wrap<T*>`
        * `y3c::const_ptr<T>` = `y3c::wrap<const T*>`
        * `y3c::ptr_const<T>` = `const y3c::wrap<T*>`
        * `y3c::const_ptr_const<T>` = `const y3c::wrap<const T*>`
* `#include <y3c/array>`
    * `y3c::array<T, N>` ← `std::array<T, N>`
* `#include <y3c/memory>`
    * `y3c::shared_ptr<T>` ← `std::shared_ptr<T>`
        * `y3c::make_shared<T>()` ← `std::make_shared<T>()`

## ライセンス

y3c-stlはMITライセンスの下で公開されています。

依存ライブラリとして
[cpptrace](https://github.com/jeremy-rifkin/cpptrace) (MITライセンス),
[libdwarf](https://github.com/davea42/libdwarf-code) (LGPL)
を使用しています。
