# vstouch

[VocalShifterライブラリ vslib](http://ackiesound.ifdef.jp/soko.html) を利用して速度変更やピッチ変更をコマンドラインからできるようにするツールです。

## 注意事項

vstouch は無保証で提供されます。  
vstouch を使用したこと及び使用しなかったことによるいかなる損害について、開発者は何も責任を負いません。

これに同意できない場合、あなたは vstouch を使用することができません。

# インストール／アンインストール

[VocalShifterライブラリ vslib](http://ackiesound.ifdef.jp/soko.html) の DLL である `vslib.dll` を同じ場所に置いてください。

アンインストールは導入したファイルを削除するだけで完了です。

# 使い方

以下のように引数を渡すと、`input.wav` の再生速度を2倍、ピッチも2倍にした `output.wav` を生成します。

```bat
vstouch -speed 2 -pitch 2 input.wav output.wav
```

# 更新履歴

## v0.2 2021-09-03

- パラメーターの渡し方を変更

## v0.1 2021-08-29

- 初版
