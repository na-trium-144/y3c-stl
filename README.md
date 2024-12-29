# y3c-stl (やさしいSTL)

std:: の標準ライブラリの代わりに y3c:: 以下のクラスを使うことで、例外がcatchされなかった場合や未定義動作の代わりにわかりやすいエラーメッセージとスタックトレースを表示します。
C++初心者におすすめです。

Linux, MacOS, Windows (MSVC, MinGW) で動作確認しています。
スタックトレースの取得と表示に関しては [cpptrace](https://github.com/jeremy-rifkin/cpptrace) ライブラリを使用しています。

## Example

```cpp
#include <y3c/vector>  // <= instead of <vector>
#include <iostream>
 
int main() {
    y3c::vector<int> a = {1, 2, 3, 4, 5};  // <= instead of std::vector<int>
    y3c::vector<int>::iterator it = a.begin();
    a.resize(100);                  // reallocation occurs here and
    std::cout << *it << std::endl;  // <= this would be undefined behavior, but...
}
```
↓
```
y3c-stl terminated: undefined behavior detected (ub_access_deleted)
  at y3c::vector<int>::iterator::operator*(): attempted to access the deleted value.
Stack trace (most recent call first):
#0 0x000056364363f72e in main at /home/runner/work/y3c-stl/y3c-stl/build/../examples/vector-iter-reallocate.cc:8:18
       6:     y3c::vector<int>::iterator it = a.begin();
       7:     a.resize(100);
       8:     std::cout << *it << std::endl;
       9: }
```

[こちら](https://na-trium-144.github.io/y3c-stl/examples.html)から他のサンプルコードと実行結果の例を見ることができます。

## インストール

### Homebrew (MacOS, Linux)

```sh
brew tap na-trium-144/y3c
brew install y3c-stl
```
でインストールできます。

### ソースからビルド

Meson (>=1.1) をインストールしてください。

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
Linux,Macの場合は特にこだわりがなければ何も指定せずデフォルトのまま (`/usr/local` など) がおすすめです。

また、Windows MSVC の場合は release ビルド(デフォルト)と debug ビルド(meson setup 時に `-Dbuildtype=debug` を指定)の両方をインストールする必要があります。

## 使い方

* y3c-stlは y3c という名前の共有ライブラリとなっているのでそれをリンクするだけでokです。
    * ただしWindowsのMSVCでdebugビルドの場合は y3cd という名前になります。
* また、Windowsの場合はy3cのdllファイル(インストール場所のbinディレクトリにあります)を実行ファイルと同じディレクトリにコピーするか、PATHを通す必要があります。

### 直接コンパイラの引数で指定

* Linux,Macでインストール先がデフォルトの場合は `-ly3c` と `-g` を渡せばよいです。
    * インストール先を変更している場合は `-I/path/to/y3c/include -L/path/to/y3c/lib -ly3c` などとパスも指定する必要があります。
    * `-g` はなくてもビルドできますが、スタックトレースが表示されなくなります。
* コンパイラによってはさらに `-std=c++11` またはそれ以上がないとコンパイルエラーになる場合があります。
* Windows MSVCの場合は `/IC:\path\to\y3c\include` と `/libpath:C:\path\to\y3c\lib` でパスを指定し、
`y3c.lib`(releaseビルドの場合) または `y3cd.lib`(debugビルドの場合) を渡してください。

### pkg-config

* コンパイラの引数に `$(pkg-config --cflags --libs y3c)` と `-g` を渡せば良いです。
    * インストール先を変更している場合は `PKG_CONFIG_PATH` 環境変数に `/path/to/y3c/lib/pkgconfig` などを追加する必要があります。
* コンパイラによってはさらに `-std=c++11` またはそれ以上がないとコンパイルエラーになる場合があります。

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
    * さらに`int`などのクラスでない変数については `y3c::wrap<int>` 、配列 `int[5]` → `y3c::wrap<int[5]>`、参照 `int&` → `y3c::wrap<int&>`、 生ポインタ `int*` → `y3c::wrap<int*>` で置き換えることで、nullptrや範囲外へのアクセス、また寿命が切れた変数へのアクセスかどうかをチェックすることができます。
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

リンクはDoxygenによるドキュメントへのリンクです。

* `#include <y3c/wrap>` (y3c独自のユーティリティ関数、STLのクラスに属さないものなど)
    * y3c::link()
    * y3c::unwrap(y3c::wrap&lt;T&gt;)
    * [y3c::wrap&lt;T&gt;](https://na-trium-144.github.io/y3c-stl/classy3c_1_1wrap.html) ← `T`
    * [y3c::wrap&lt;T&>](https://na-trium-144.github.io/y3c-stl/classy3c_1_1wrap_3_01element__type_01_6_01_4.html) ← `T&`
        * y3c::wrap_ref&lt;T&gt; = `y3c::wrap<T&>`
        * y3c::const_wrap_ref&lt;T&gt; = `y3c::wrap<const T&>`
    * [y3c::wrap&lt;T*&gt;](https://na-trium-144.github.io/y3c-stl/classy3c_1_1wrap_3_01element__type_01_5_01_4.html) ← `T*`
        * y3c::ptr&lt;T&gt; = `y3c::wrap<T*>`
        * y3c::const_ptr&lt;T&gt; = `y3c::wrap<const T*>`
        * y3c::ptr_const&lt;T&gt; = `const y3c::wrap<T*>`
        * y3c::const_ptr_const&lt;T&gt; = `const y3c::wrap<const T*>`
* `#include <y3c/array>`
    * [y3c::array&lt;T, N&gt;](https://na-trium-144.github.io/y3c-stl/classy3c_1_1array.html) ← `std::array<T, N>`
* `#include <y3c/vector>`
    * [y3c::vector&lt;T&gt;](https://na-trium-144.github.io/y3c-stl/classy3c_1_1vector.html) ← `std::vector<T>`
* `#include <y3c/memory>`
    * [y3c::shared_ptr&lt;T&gt;](https://na-trium-144.github.io/y3c-stl/classy3c_1_1shared__ptr.html) ← `std::shared_ptr<T>`
        * y3c::make_shared&lt;T&gt;() ← `std::make_shared<T>()`

## ライセンス

y3c-stlはMITライセンスの下で公開されています。

依存ライブラリとして
[cpptrace](https://github.com/jeremy-rifkin/cpptrace) (MITライセンス),
[libdwarf](https://github.com/davea42/libdwarf-code) (LGPL),
[rang](https://github.com/agauniyal/rang) (Unlicense)
を使用しています。
