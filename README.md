# 超技術書典

## じゅんび

* OSX or macOSちほー
* rbenvが使えるフレンズ

#### PDFを作るのが得意なフレンズになる方法

[C89-FirstStepReVIEW-v2/blob/master/articles/setup.re](https://github.com/TechBooster/C89-FirstStepReVIEW-v2/blob/master/articles/setup.re) より引用

> Re:VIEW文書をPDFに変換するにはLaTeX（platexまたはlualatexなど）を使います。  
出力時の処理はreview形式→reviewツール実行→latex形式→platex実行→PDF という流れです。

とゆーことなので、論文などを書いた環境じゃなければ下記のインストールが必要だよ

1. MacTeX http://tug.org/cgi-bin/mactex-download/MacTeX.pkg をダウンロードしてインストール  
(※2.7GBぐらいある重たいフレンズなので、時間がかかるよ)
2. ターミナル再起動
3. これであなたも本が作れるフレンズになったよ

## サンプルをpdf化して動作確認

```
# 環境構築
./init.sh

# サンプルをreviewでコンパイルして、できあがったpdfを開くよ
./sample.sh
```

## 自分の書いた文章をpdf化したいフレンズ

1. 自分の書いた記事を追記する。

```
vim articles/catalog.yml
### CHAPS:
###  - ファイル名を追記(例：filename.re)
```

2. 後は記事を書きながらRe:Viewでコンパイルして確認

```
# markdownで書いていた場合、Re:View形式に変換して、コンパイル
./build.sh ファイル名
```
