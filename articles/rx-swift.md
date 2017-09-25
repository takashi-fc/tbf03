# ユースケースで学ぶRxSwift再入門

## はじめに

みなさん、RxSwift[^1]使ってますか？
プロミス・データバインディング・イベントバス・リストをあれこれする処理など色々できて良いですよね。
ただ、「全然分からない。俺は雰囲気でObservableを使っている。」状態で使っていませんか？
本章は、（私含めて）そこからの脱却を目指す第一弾です。
今回は、`RxSwift`を使う上での基本を学んでいきながら、最後に各ユースケースで使い方を学んでいきます。
本章の内容は、`RxSwift`のgithubリポジトリにある`Rx.playground`[^2]を主に参考にしています。

執筆時点の環境

- Xcode: 9.0
- Swift: 4.0
- RxSwift: rxswift4.0-swift4.0ブランチ(cb0fb3bda41f2a5622909b6e6deafd435efaf859)

## 基本的なRxSwiftの基礎知識・考え方

RxSwiftを学ぶにあたって、まず最低限理解しておかないといけないものがあります。
これに関して殆どは、`ReactiveX`全体で同様なので、他の言語で使ったことがある人は見慣れたものになると思います。

### Observable と Observer の関係

公式サイト[^3]の説明を日本語訳すると

> `Observer`は`Observable`を購読します。

と書かれています。
つまり、どういうことなの？と思うかもしれません。
イメージの付きやすいように考える場合、よく`Observable`は川と例えられますが、個人的には蛇口の方が合ってるかと思っています。
その違いは、**蛇口を開かないと水が流れない** というところです。
`Observable`は`subscribe`を呼び出されること **蛇口が開き**、**初めて** 水が流れ出します。
（**初めて** という言葉を強調したのは、例外も存在するからです。これについてはまた後で説明します。）
そして、`Observable`に流れている値は`subscribe`に引数として渡した`Observer`の`onNext,onError,onComplete`に流れ着きます。
`Observer`はObserverパターンと同じ意味合いです。なので、流れ着く＝通知されるということになります。

### Operatorについて

`Observable`を生成・変換するために`Operator`が存在します。

```
Observable.of(1, 2, 3).map { $0 * $0 }
```

例えば、上記のコードの`of`や`map`が`Operator`になります。
他にも、`Observable`を生成する`Operator`は`just,create`、変換する`Operator`は`map, flatMap, scan`などがあります。
まだまだ様々な`Operator`が存在しますが、量が多いので必要に応じてユースケースで学ぶ章で説明をしようと思います。

### Observerについて

章の冒頭で

>　`Observable`は`subscribe(Observer)`を呼び出されること **蛇口が開き**、**初めて** 水が流れ出します。

このように言いました。

つまり先程`Operator`について紹介する際に書いたコードだけでは何も起こりません。
下記のように`subscribe`することで初めて生成した`Observable`が実行されます。

```
Observable.of(1, 2, 3).map { $0 * $0 }.subscribe(onNext: { print($0) })
```

このコードでは、`1, 2, 3`という値がそれぞれ`1 * 1`, `2 * 2`, `3 * 3`と`map`で変換され、`1, 4, 9`と出力されます。

#### Observerと追加された派生系の種類

RxSwift3.x系からは`Observer`の他に、`Single`, `Compaletable`, `Maybe`が追加されています。
これらは通知される`onXXX`が違います。`Observer`を含めて表にするとこのようになります。

| Observerの種類 | 動作 |
|:--------:|-----|
| `Obsever` | `onNext(value)`が1回以上、 `onCompleted`,`onError(error)`がどちらか1回 |
| `Single` | `onSuccess(value)`,`onError(error)`のどちらが1回 |
| `Compaletable` | `onCompleted`,`onError(error)`がどちらか1回 |
| `Maybe` | `onSuccess(value)`,`onCompleted`,`onError(error)`のどれかが1回 |

また、`onNext`,`onSuccess`内で例外が発生すると`onError`が呼び出されます。 // TODO: onCompletedも同じ？

### Scheduler

今までは値を流す方法`Observable`や、流した後どこに着くのか`Observer`について学びました。
実際にこれらを使う時の事を考えてみましょう。
例えば、通信処理で下記のような処理をするとします。

1. リクエストを投げる
2. レスポンスを受け取る
3. 受け取った値をモデルなどにパースする
4. できがったモデルをUIへ反映する

この時、1~3はバックグラウンドで、4はメインスレッドで処理をするのが適切です。
そのスレッドの制御をする役割を持つのが`Scheduler`です。
`Scheduler`はiOSの仕組みとして存在するGCD（Grand Central Dispatch）を利用しています。
それぞれの処理に`Scheduler`を指定するために、`observeOn`,`subscribeOn`メソッドを利用します。
`observeOn`は`Operator`での処理、`subscribeOn`は`Observer`での処理の`Scheduler`を指定できます。

```
Observable.of(1, 2, 3)
          .observeOn(A-scheduler)
          .map { $0 + 1 }
          .observeOn(B-scheduler)
          .map { $0 * $0 }
          .subscribeOn(C-scheduler)
          .subscribe(onNext: { print($0) })
```

このように指定することができます。
`.map { $0 + 1 }`は`.observeOn(A-scheduler)`で指定された`Scheduler`で動き
`.map { $0 * $0 }`は`.observeOn(B-scheduler)`で指定された`Scheduler`で動き
`.subscribe(onNext: { print($0) })`は`.subscribeOn(C-scheduler)`で指定された`Scheduler`で動きます。
もし`.subscribeOn`が2回呼び出されていた場合は、後に呼び出した方の`Scheduler`で動きます。
つまり、`observeOn`,`subscribeOn`を呼び出した以降の処理はそれぞれ指定した`Scheduler`で動きます。

#### Schedulerの種類

| Scheduler | 動作 |
|:--------:|-----|
| MainScheduler | メインスレッドで動きます ※observeOnに最適化されています |
| ConcurrentMainScheduler | メインスレッドで動きます ※subscribeOnに最適化されています |
| CurrentThreadScheduler | 現在のスレッドで動きます |
| SerialDispatchQueueScheduler | 指定されたQOSで生成された直列なQueueで動きます |
| ConcurrentDispatchQueueScheduler | 指定されたQOSで生成された並列なQueueで動きます |

それぞれ`DispatchQueue`を持つ仕組みになっていて、イニシャライザには引数として`DispatchQoS`を渡すものと`DispatchQueue`を渡すものがあります。
`DispatchQoS`を引数として渡すイニシャライザは`iOS8`から追加されていて、指定したいラベルがあるなどのことがなければ`DispatchQueue`を渡すのではなく、`DispatchQoS`を渡す方が良いです。

#### Schedulerで注意すべきこと

Conccurent（並列）な`Scheduler`で処理されていても、1つの`Observable`によって流れる値は順序が保証されています。
そのため、このコードのようにスリープを挟んでも実行すると下記のように出力されます。

```
let observeOnScheduler = ConcurrentDispatchQueueScheduler(qos: .default)

var count = 0
Observable.from(1...3)
    .observeOn(observeOnScheduler)
    .do(onNext: { i in
        let time = arc4random_uniform(3) + 1 // 1~3秒のスリープ
        print("observable sleep \(time)")
        sleep(time)
        count += 1
        print("observable \(i): \(count)")
    }, onError: nil, onCompleted: nil, onSubscribe: nil, onSubscribed: nil, onDispose: nil)
    .subscribe()

// 出力
A observable sleep 2
A observable 1: 1
A observable sleep 1
A observable 2: 2
A observable sleep 1
A observable 3: 3
```

では、先程のスケジューラで2つのObservableをsubscribeしたらどうなるでしょうか？

```
let observeOnScheduler = ConcurrentDispatchQueueScheduler(qos: .default)

var count = 0
Observable.from(1...3)
    .observeOn(observeOnScheduler)
    .do(onNext: { i in
        let time = arc4random_uniform(3) + 1
        print("A observable sleep \(time)")
        sleep(time)
        count += 1
        print("A observable \(i): \(count)")
    }, onError: nil, onCompleted: nil, onSubscribe: nil, onSubscribed: nil, onDispose: nil)
    .subscribe()

Observable.from(4...6)
    .observeOn(observeOnScheduler)
    .do(onNext: { i in
        let time = arc4random_uniform(3) + 1
        print("B observable sleep \(time)")
        sleep(time)
        count += 1
        print("B observable \(i): \(count)")
    }, onError: nil, onCompleted: nil, onSubscribe: nil, onSubscribed: nil, onDispose: nil)
    .subscribe()

// 出力
B observable sleep 2
A observable sleep 1
A observable 1: 1
A observable sleep 1
B observable 4: 2
B observable sleep 3
A observable 2: 3
A observable sleep 3
B observable 5: 4
B observable sleep 1
A observable 3: 5
B observable 6: 6
```

`ConccurentScheduler(並列)`で処理しているため、共有された変数`count`と`Observable`に流れる数値が同じにならず、意図通り並列で動いていることがわかります。
では同じ`Observable`で`SerialScheduler(直列)`を利用してみましょう。


```
let observeOnScheduler = SerialDispatchQueueScheduler(qos: .default)

var count = 0
Observable.from(1...3)
    .observeOn(observeOnScheduler)
    .do(onNext: { i in
        let time = arc4random_uniform(3) + 1
        print("A observable sleep \(time)")
        sleep(time)
        count += 1
        print("A observable \(i): \(count)")
    }, onError: nil, onCompleted: nil, onSubscribe: nil, onSubscribed: nil, onDispose: nil)
    .subscribe()

Observable.from(4...6)
    .observeOn(observeOnScheduler)
    .do(onNext: { i in
        let time = arc4random_uniform(3) + 1
        print("B observable sleep \(time)")
        sleep(time)
        count += 1
        print("B observable \(i): \(count)")
    }, onError: nil, onCompleted: nil, onSubscribe: nil, onSubscribed: nil, onDispose: nil)
    .subscribe()

// 出力
A observable sleep 2
A observable 1: 1
A observable sleep 1
A observable 2: 2
A observable sleep 2
A observable 3: 3
B observable sleep 3
B observable 4: 4
B observable sleep 1
B observable 5: 5
B observable sleep 1
B observable 6: 6
```

それぞれスリープがかかっていることにも関わらず、共有された変数`count`と`Observable`に流れる数値が同じになっていて、意図通り直列に動いていることがわかります。

さて、ここで大事なのが`Observable`を`subscribe`した時、`observeOn`,`subscribeOn`で`Scheduler`を指定していない場合は`CurrentThreadScheduler(今いるスレッド)`で実行するということです。
つまり、何も考えずにメインスレッドで動いている処理中に`subscribe`してしまうと、重い処理をメインスレッドで処理してしまいます。

普段コードを書いている時に、ここまで出てきたように`Observable`をそれぞれ生成して`subscribe`することなんてないからあまり関係なさそうだと思う人がいるかもしれませんが、実は普段からよく発生している処理です。
それがどんなときかというと、`zip`, `merge`など`Observeble`を結合して処理を行う場合です。

```
let observeOnScheduler = // Schedulerを生成

var count = 0
let AObservable = Observable.from(1...3)
    .observeOn(observeOnScheduler)
    .do(onNext: { i in
        let time = UInt32(3)
        print("A observable sleep \(time)")
        sleep(time)
        count += 1
        print("A observable \(i): \(count)")
    }, onError: nil, onCompleted: nil, onSubscribe: nil, onSubscribed: nil, onDispose: nil)

let BObservable = Observable.from(4...6)
    .observeOn(observeOnScheduler)
    .do(onNext: { i in
        let time = UInt32(3)
        print("B observable sleep \(time)")
        sleep(time)
        count += 1
        print("B observable \(i): \(count)")
    }, onError: nil, onCompleted: nil, onSubscribe: nil, onSubscribed: nil, onDispose: nil)

Observable.zip(AObservable, BObservable, resultSelector: { e1, e2 in
    print("e1: \(e1), e2: \(e2)")
}).subscribe()
```

このコードは、今までに説明で利用してきた`Observable`を変数化して`Observable.zip`で結合したものを`subscribe`しています。
`let observeOnScheduler`に`ConcurrentDispatchQueueScheduler`を指定した場合は、並列で走るため`3秒スリープ×3回`分の時間で完了します。
しかし、`SerialDispatchQueueScheduler`を指定した場合は、直列で走るため`3秒スリープ×3回×2つObservable`分の時間がかかってしまいます。
通信処理の待ち合わせなどで`zip`,`merge`などを使っていて、なぜか遅いなと思ったらSchedulerを疑ってみると良いかもしれません。

### Subject(余力あれば)

ここで冒頭で説明した、この説明が関わってきます。

> `Observable`は`subscribe(Observer)`を呼び出されることで、**初めて** 川が流れ出します。

`Hot` or `Cold`
急に暑いとか寒いってなに？って感じですね。
実は`subscribe(Observer)`されなくとも、川が流れ出す場合があります。
行き着く`Observer`がないので、永遠に流れている川のようなイメージです。


http://qiita.com/KentaKudo/items/4d7154c3dada93f11492
http://qiita.com/k5n/items/98aaf84fc164f7a5502c

用途は主に同期的なUI操作のと気に使われる事が多いです。

## Operatorを自作してみる

さて、基本的な知識を学んできました。
川の流れと例えて来ましたが、実は理解する上で逆流する必要があります。
http://hadashia.hatenablog.com/entry/2016/02/23/175736
https://qiita.com/fu_nya/items/8e52f835737389f8492c

// TODO: observable, subscribeの数珠つなぎの仕組みはここで説明する？　https://qiita.com/k5n/items/643cc07e3973dd1fded4


```
import RxSwift

extension PrimitiveSequenceType where TraitType == SingleTrait {

    public func myDebug(identifier: String) -> Single<Self.ElementType> {
        return Single<ElementType>.create(subscribe: { observer in
            Logger.debug("subscribed \(identifier)")
            let subscription = self.subscribe { e in
                Logger.debug("event \(identifier)  \(e)")
                switch e {
                case .success(let value):
                    observer(.success(value))
                case .error(let error):
                    observer(.error(error))
                }
            }
            return Disposables.create {
                Logger.debug("disposing \(identifier)")
                subscription.dispose()
            }
        })
    }

}

extension ObservableType {
    public func myDebug(identifier: String) -> Observable<Self.E> {
        return Observable.create { observer in
            Logger.debug("subscribed \(identifier)")
            let subscription = self.subscribe { e in
                Logger.debug("event \(identifier)  \(e)")
                switch e {
                case .next(let value):
                    observer.on(.next(value))

                case .error(let error):
                    observer.on(.error(error))

                case .completed:
                    observer.on(.completed)
                }
            }
            return Disposables.create {
                Logger.debug("disposing \(identifier)")
                subscription.dispose()
            }
        }
    }
}
```

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
