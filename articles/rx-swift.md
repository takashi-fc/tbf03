# ユースケースで学ぶRxSwift再入門

## はじめに

みなさん、RxSwift[^1]使ってますか？
プロミス・データバインディング・イベントバス・リストをあれこれする処理など色々できて良いですよね。
ただ、「全然分からない。俺は雰囲気でObservableを使っている。」状態で使っていませんか？
本章は、（私含めて）そこからの脱却を目指す第一弾です。
今回は、`RxSwift`を使う上での基本知識と各ユースケースでの使い方を学んでいきます。
本章の内容は、`RxSwift`のgithubリポジトリにある`Rx.playground`[^2]を主に参考にしています。

執筆時点の環境

- Xcode: 8.3.3
- Swift: 3.1
- RxSwift: 3.6.1

## 基本的なRxSwiftの基礎知識・考え方

RxSwiftを学ぶにあたって、まず最低限理解しておかないといけないものがあります。
これに関して殆どは、`ReactiveX`全体で同様なので、他の言語で使ったことがある人は見慣れたものになると思います。

### Observable と Observer の関係

公式サイト[^3]の説明を日本語訳すると

> `Observer`は`Observable`を購読します。

と書かれています。
つまり、どういうことなの？と思うかもしれません。
イメージの付きやすいように考える場合、よく`Observable`は川と例えられます。（個人的には蛇口の方が合ってるかと思っています。）
`Observable`は`subscribe(Observer)`を呼び出されることで、**初めて** 川が流れ出します。
（**初めて** という言葉を強調したのは、例外も存在するからです。これについてはまた後で説明します。）
そして、`Observable`に流れている値は`subscribe`に引数として渡した`Observer`の`onNext()`, `onError()`, `onComplete()`に流れ着きます。
`Observer`は名前の通りでObserverパターンと同じです。なので、流れ着く＝通知されるということになります。

### Observableについて

`Observable`を生成・変換するために`Operator`が存在します。
ここでは本章で使うorよく実装で使うものを中心に紹介していきます。

#### 生成する

| Operator | 動作 |
|:--------:|-----|
| never | 何も流れず、終了しない |
| empty | onComplete()のみ流れる |
| just | 任意の1つの値が、onNext()に流れ、その後onComplete()が流れる |
| of | 任意の複数の値が、onNext()に流れ、全てが流れたらonComplete()が流れる |
| from | 任意の1つの値(Array, Dictionary, Set型)が、要素毎にonNext()に流れ、全てが流れたらonComplete()が流れる |
| error | 任意の1つのErrorが、onError()に流れる |
| create | 任意のObserverの呼び出しからObservableを生成される |
| deferred | 任意のObservableをsubscribeする毎に生成される |
| doOn | Observerの`onNext()`, `onError()`, `onComplete()`などの前に処理を行うObservableが生成される |

#### 変換・抽出する

| Operator | 動作 |
|:--------:|-----|
| map |  |
| flatMap |  |
| filter |  |
| distinct |  |
| take |  |
| first |  |
| last |  |
| skip |  |

#### 結合する

| Operator | 動作 |
|:--------:|-----|
| merge |  |
| zip |  |
| combineLatest |  |

#### まとめる

| Operator | 動作 |
|:--------:|-----|
| concat |  |
| reduce |  |
| toArray |  |


### Observerについて

RxSwift3.x系からは`Observer`の他に、`Single`, `Compaletable`, `Maybe`が追加されています。
これらの違いは、それぞれ`onNext`, `onComplete`, `onError`の呼び出しです。

#### Observer

<table>
<tr>
  <th align=center>onNext</th>
  <th align=center>onComplete</th>
  <th align=center>onError</th>
</tr>
<tr>
  <td align=center>複数回</td>
  <td colspan=2 align=center>1回</td>
</tr>
</table>

#### Single, Compaletable, Maybe

<table>
<tr>
  <th align=center></th>
  <th align=center>onSuccess</th>
  <th align=center>onComplete</th>
  <th align=center>onError</th>
</tr>
<tr>
  <td align=center>Single</td>
  <td align=center>1回</td>
  <td align=center>-</td>
  <td align=center>1回</td>
</tr>
<tr>
  <td align=center>Compaletable</td>
  <td align=center>-</td>
  <td align=center>1回</td>
  <td align=center>1回</td>
</tr>
<tr>
  <td align=center>Maybe</td>
  <td align=center>0 or 1回</td>
  <td align=center>1回</td>
  <td align=center>1回</td>
</tr>
</table>


http://qiita.com/monoqlo/items/7bcec98432389b3b8909

### `Scheduler`

今までは値を川に流す方法`Observable`や、流した後どこに着くのか`Observer`について学びました。
実際にこれらを使う時の事を考えてみましょう。
例えば、通信処理を例にすると以下のような処理をすることがあると思います。

1. リクエストを投げる
2. レスポンスを受け取る
3. 受け取った値をモデルなどにパースする
4. できがったモデルをUIへ反映する

| Scheduler | 動作 |
|:--------:|-----|
| CurrentThreadScheduler.instance | 現在のスレッドで実行する |
| MainScheduler.instance | メインスレッドで実行する |

`MainScheduler.instance`が結局何をしているかというと、このようになっています。
ただメインスレッドを取得して、`SerialDispatchQueueScheduler`の実行スレッドとして指定しているだけです。

```
public final class MainScheduler : SerialDispatchQueueScheduler {

    private let _mainQueue: DispatchQueue

    var numberEnqueued: AtomicInt = 0

    /// Initializes new instance of `MainScheduler`.
    public init() {
        _mainQueue = DispatchQueue.main
        super.init(serialQueue: _mainQueue)
    }

    /// Singleton instance of `MainScheduler`
    public static let instance = MainScheduler()

    ...
}
```

// TODO: .zip系は中を同じスレッドで走らせると遅くなる？のでその動きを調べる
http://hadashia.hatenablog.com/entry/2016/03/17/212413
https://stackoverflow.com/questions/37973445/does-the-order-of-subscribeon-and-observeon-matter

### `Subject`(余力あれば)

ここで冒頭で説明した、この説明が関わってきます。

> `Observable`は`subscribe(Observer)`を呼び出されることで、**初めて** 川が流れ出します。

`Hot` or `Cold`
急に暑いとか寒いってなに？って感じですね。
実は`subscribe(Observer)`されなくとも、川が流れ出す場合があります。
行き着く`Observer`がないので、永遠に流れている川のようなイメージです。


http://qiita.com/KentaKudo/items/4d7154c3dada93f11492
http://qiita.com/k5n/items/98aaf84fc164f7a5502c

用途は主に同期的なUI操作のと気に使われる事が多いです。



## ユースケースで学ぶ（通信編）

さて、ここまでRxSwiftを使う上での基本を学びました。
ここからはイメージの付きやすいように、TwitterにおけるユースケースでRxSwiftを学んでいきます。（皆さんTwitterは使っていますか？使っていますよね。）
※APIは実際のものではなく、模したAPIを利用しています。

### ユーザのアカウントを認証する

説明

```
コード
```

### ユーザのアカウントを認証してから、ツイートの一覧を取得する

説明

```
コード
```

### ツイートの一覧からミュートしているユーザを除いて取得する（.zip + .filter）

説明

```
コード
```

### いいねとリツイートを混ぜて一覧として取得する（.merge）

説明

```
コード
```

### エラーハンドリング

通信処理はいつでも失敗する可能性がありますよね。この記事がとてもわかりやすいです。
https://academy.realm.io/posts/best-practices-pain-points-mobile-networking-rest-api-failures/

#### タイムアウトしたら、通知を出す。そしてもう一度実行するか聞く。


#### 返ってきたレスポンスのJSONスキーマが違って、モデルにパースできなかった。

#### ツイートしたけど、オフライン状態だった

通常の処理では、ツイートして返ってきたレスポンスを保存しますよね。
ただ、ツイートも失敗する可能性がありますよね。
公式アプリではその場合に下書きというところに保存するようになっています。

```
コード
```

#### いいねしたけど、オフライン状態だった

いいねボタン、アニメーションもあって気持ちいいですよね。
ただ、失敗した時はいいねボタンをオフ状態に戻したかったりします。

```
コード
```

#### ツイートを削除したけど、オフライン状態だった

リモートを削除したと同時に、ローカルを削除も削除したいですよね。
ただ、リモートが削除できた場合にのみ、ローカルを削除する必要がありますね。
しかし、PUTを叩いても204しか返ってこず、削除したツイートのIDがない場合も。

```
コード
```

## ユースケースで学ぶ（UI編）

### ツイートが空の場合にEmptyStateを表示したい

説明

```
コード
```

### ログインフォームのバリデーション

説明

```
コード
```

### チャットの他の人が入力中…みたいなやつ

- チャット形式チュートリアルver（タイマーで自動投稿）
  - 配列を回して次の文章を取る方法でチャット形式のチュートリアルできそう
    - ユーザの入力待ち合わせも試す

説明

```
コード
```

### タブ間でいいねを同期したい

説明

```
コード
```


## おわりに

[^1]: https://github.com/ReactiveX/RxSwift
[^2]: https://github.com/ReactiveX/RxSwift/tree/master/Rx.playground/Pages
[^3]: http://reactivex.io/documentation/observable.html
