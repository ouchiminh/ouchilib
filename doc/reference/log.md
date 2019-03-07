# log

このコンポーネントはアプリケーションがランタイムにログを出力するための基本的な機能を提供します。

- [format](#format)
- [out](#out)
- [rule](#rule)

## format

[Code](../../include/ouchilib/log/format.hpp)

このコンポーネントはログにフォーマットの機能を提供します

- [ログレベル](#ログレベル)
- [メッセージ要素型](#メッセージ要素型)
- [`message_format`](#message_format)

### ログレベル

ログレベルは以下のように分かれます。また、ログレベルはログカテゴリーと呼びます

- cat_v
    - trace
    - debug
    - info
    - warn
    - error
    - fatal

### メッセージ要素型

`ouchi::log`ではCライクなフォーマット指定を完全に捨て去り、エレガントでC++的な扱いやすいフォーマットの指定法を実装しています。

メッセージを構成する要素を`Msg`, `Category`, `Time`に分類し、さらにユーザーが指定するオプショナルな文字列も要素に加えることができます。

以下に示す型のインスタンスを`message_format`に追加しログをフォーマットします。

#### `Msg`

ログメッセージです。ユーザーが`basic_out<>`などで指定する文字列を表しています。

#### `Category`

ログカテゴリーを表します。

#### `Time`

時間を表します。

#### その他

`ouchi::log`では以上の型がユニークな要素ですが、ほかにも文字列と文字をログのフォーマットに追加できます。

### message_format

メッセージ要素型を保持します。

#### 例

`message_format`クラスを単独で利用することはありませんが、フォーマット指定の方法を知っておくためにはこの例を見る必要があります。

```C++
ouchi::log::message_format<char> f;

f << time << cat << msg << ' ' << "logging successed!";

// フォーマットをリセット
f.clear();
```

## out
[Code](../../include/ouchilib/log/out.hpp)

このコンポーネントはログレベルとメッセージをフォーマットを整えて受け取り出力関数に渡します。いくつかの既定の出力関数オブジェクトも共に定義されています。

- [デフォルトの出力関数(オブジェクト)](#デフォルトの出力関数(オブジェクト))
- [`basic_out`](#basic_out)

### デフォルトの出力関数(オブジェクト)

- `default_out`
    
    このラムダ式は受け取った文字列を`std::clog`にそのまま出力します。

- `out_func<>`

    この関数オブジェクトはコンストラクタあるいは`set()`で受け取った名前のファイルを開いて、そこに受け取った文字列を出力します。複数のインスタンスで同じファイルを指定しても出力ストリームは一つしか作成されませんが、どちらのインスタンスでも正しく出力されます。

### basic_out

このクラスはログレベルとメッセージをフォーマットを整えて受け取り、出力関数に渡します。

#### メンバ

- ctor

    コンストラクタには出力関数を指定してください。既定では`default_out`が指定されます。

- `fatal`
- `error`
- `warn`
- `info`
- `debug`
- `trace`

    これらのメソッドではログメッセージを受け取ります。

- `out`

    このメソッドでは上記の6つのメソッドで受け取ったログメッセージと、ログカテゴリーに基づいてフォーマットを整え、出力関数に渡します。

#### 例

適当なログメッセージをデフォルトの設定で出力します。

```C++
ouchi::log::basic_out<char> logger;
logger.trace("logging was successfully completed.");
```

実行結果
```log
2018-11-11T00:03:04 [TRACE] logging was successfully completed.

```

次にログメッセージのフォーマットを変更して出力します

```C++
ouchi::log::basic_out<char> logger;
logger.reset_format() << ouchi::log::cat << ouchi::log::time << ' ' << ouchi::log::msg;
logger.trace("format has been changed.");
```

実行結果

```log
[TRACE]2018-11-11T00:06:21 format has been changed.

```

## rule

[Code](../../include/ouchilib/log/rule.hpp)

**注意 : このコンポーネントはboostを利用します。boostを利用できない環境ではこのコンポーネントの一部の機能は利用できません。**

このコンポーネントはログのカテゴリーに基づいて出力を切り替えたり、無効にしたりする機能を提供します。

- [`rule`](#rule(class))
- [`ruled_logger`](#ruled_logger)

### rule(class)

ログを出す範囲を表します。`rule`とペアになった`basic_out<>`は`rule::is_valid`が`true`を返す場合だけログを出力します。
コンストラクタまたは`set`メソッドでログカテゴリーの上限と下限を指定します。

### ruled_logger

ログをカテゴリーに基づいて出力先を切り替えたり無視したりします。
boostが使用できる環境であればコンストラクタではログ設定を記したjsonファイルへのパスを指定して動的に`rule`と`basic_out<>`を生成できます。
それ以外では`rule`と`basic_out<>`のペアのリストを指定します。

他のメソッドは[`basic_out<>`](#out)と同様です。

##### 例

デフォルトの設定でログを出力する例です。デフォルトでは`info`から`fatal`までのカテゴリーを出力します。
```C++
	ouchi::log::ruled_logger<char> logger{ {ouchi::log::rule{}, ouchi::log::basic_out<char>()} };
	logger.fatal("poi!");
	logger.trace("this message will be ignored.");

```

実行結果

```log
2018-11-11T11:44:50 [FATAL] poi!

```

次にjsonからログの設定を読んで出力する例です。

 setting.json
```json
{
	"setting":
	[
    {
      "file": "app.log",
      "upper": 5,
      "lower": 3
    },
    {
      "file": "app.txt",
      "upper": 5,
      "lower": 0
    }
	]
}

```

```C++
	ouchi::log::ruled_logger<char> logger("setting.json", "setting");
	logger.fatal("poi!");
	logger.trace("this message will be ignored.");
```
 app.txt
```txt
2018-11-11T12:27:21 [FATAL] poi!
2018-11-11T12:27:21 [TRACE] this message will be ignored.

```
app.log

``` txt
2018-11-11T12:27:21 [FATAL] poi!

```