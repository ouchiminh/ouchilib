# utl

このコンポーネントはouchilibやほかのアプリケーションが利用できる実用的なソフトウェアが含まれています。

- [convertible](#convertible)
- [fps-counter](#fps-counter)
- [property](#property)
- [time_keeper](#time_keeper)

## convertible

[Code](../../include/ouchilib/utl/convertible.hpp)

このクラスは型Tと、Tから任意の型への変換関数、任意の型からTへの変換関数の型をテンプレート引数にとり、変換関数が用意されている型に対してTとの間で変換するためのクラスです。

**注意 : 変換関数が用意されていない型との変換では何もしません。**

### メンバ関数

- ctor
    - `auto_convert(T&&, Converters&& ...)`
    - `auto_convert(T const &, Converters&& ...)`
    - `auto_convert(Converters&& ...)`

    これらのコンストラクタで変換関数を受け取ります。

    ```C++
    ouchi::auto_convert ac(0,
	    [](std::string const & s) { return std::stoi(s); },
	    [](int i) { return std::to_string(i); });
    ```

- operators
    - `template<class U> explicit operator U()`

        型`U`への変換です。変換関数がない場合は何もせずにデフォルト構築したUを返します。

    - `template<class U> auto_convert & operator=(U&& val)`

        型`U`からの変換です。変換関数がない場合は何も代入されません。

## fps-counter

このクラスはテンプレート引数で指定されたクロックで時間を計りフレームレートを計算します。
