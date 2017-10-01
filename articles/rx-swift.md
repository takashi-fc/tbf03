# RxSwift再入門

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
そして、`Observable`に流れている値は`subscribe`に引数として渡した`Observer`の`onNext, onError, onComplete`に流れ着きます。
`Observer`はObserverパターンと同じ意味合いです。なので、流れ着く＝通知されるということになります。

### Operatorについて

`Observable`を生成・変換するために`Operator`が存在します。

```
Observable.of(1, 2, 3, 4).map { $0 * $0 }.filter { $0 % 2 == 0 }
```

例えば、上記のコードの`of`や`map`が`Operator`になります。
他にも、`Observable`を生成する`Operator`は`just, create`、変換する`Operator`は`map, flatMap, scan`などがあります。
殆どの`Operator`は`Observable.subscribe`を呼び出し、`Observable`を生成します。
そのため上記のコードように、`Operator`はメソッドチェーンをすることができます。

### Observerについて

章の冒頭で

>　`Observable`は`subscribe(Observer)`を呼び出されること **蛇口が開き**、**初めて** 水が流れ出します。

このように言いました。

つまり先程`Operator`について紹介する際に書いたコードだけでは何も起こりません。
下記のように`subscribe`することで初めて生成した`Observable`が流れ出します。

```
Observable.of(1, 2, 3).map { $0 * $0 }.subscribe(onNext: { print($0) })
```

このコードでは、`1, 2, 3`という値がそれぞれ`1 * 1`, `2 * 2`, `3 * 3`と`map`で変換され、`1, 4, 9`と出力されます。

#### ObserverとRxSwift3.xで追加された派生系の種類

RxSwift3.x系からは`Observer`の他に、`Single`, `Completable`, `Maybe`が追加されています。
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

1. ツイートのIDを指定して、それを取得するリクエストを投げる
2. レスポンスを受け取る
3. 受け取ったJSONStringをTweetモデルに変換する
4. できがったモデルをUIへ反映する

この時、1~3はバックグラウンドで、4はメインスレッドで処理をするのが適切です。
そのスレッドの制御をする役割を持つのが`Scheduler`です。
`Scheduler`はiOSの仕組みとして存在するGCD（Grand Central Dispatch）を利用しています。
また、`Scheduler`を指定するためには`observeOn`,`subscribeOn`メソッドを利用します。

```
TweetService.requestTweet(by: ツイートのID)　　　　　　　　              　 // 1
            .map { JSONStringをTweetモデルへ変換($0) }                   // 2,3
            .subscribeOn(background-scheduler)
            .observeOn(main-scheduler)
            .subscribe(onNext: { label.text = $0.ツイートのテキスト変数 }) // 4
```

このように指定することができます。
これで`1~3`はバックグラウンドスレッド、`4`はメインスレッドで動作します。
きちんと`observeOn`,`subscribeOn`の動作を理解をするためには、`Observable.subscribe()`の動作を理解する必要があり、次の章で詳しくみていきます。
この章を見る上では`observeOn`は下方向に適応し、`subscribeOn`は上方向に適応されるぐらいの認識で大丈夫です。

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

Concurrent（並列）な`Scheduler`で処理されていても、1つの`Observable`によって流れる値は順序が保証されています。
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

`ConcurrentScheduler(並列)`で処理しているため、共有された変数`count`と`Observable`に流れる数値が同じにならず、意図通り並列で動いていることがわかります。
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
それがどんなときかというと、`zip`, `merge`など`Observable`を結合して処理を行う場合です。

```
let observeOnScheduler = // Schedulerを生成

let aObservable = Observable.from(1...3)
    .observeOn(observeOnScheduler)
    .do(onNext: { i in
        sleep(UInt32(3))
    }, onError: nil, onCompleted: nil, onSubscribe: nil, onSubscribed: nil, onDispose: nil)

let bObservable = Observable.from(4...6)
    .observeOn(observeOnScheduler)
    .do(onNext: { i in
        sleep(UInt32(3))
    }, onError: nil, onCompleted: nil, onSubscribe: nil, onSubscribed: nil, onDispose: nil)

Observable.zip(aObservable, bObservable, resultSelector: { e1, e2 in
    print("e1: \(e1), e2: \(e2)")
}).subscribe()
```

このコードは、今までに説明で利用してきた`Observable`のスリープ処理だけを残し変数化して`Observable.zip`で結合したものを`subscribe`しています。
`let observeOnScheduler`に`ConcurrentDispatchQueueScheduler`を指定した場合は、並列で走るため`3秒スリープ×3回`分の時間で完了します。
しかし、`SerialDispatchQueueScheduler`を指定した場合は、直列で走るため`3秒スリープ×3回×2つObservable`分の時間がかかってしまいます。
通信処理の待ち合わせなどで`zip`,`merge`などを使っていて、なぜか遅いなと思ったらSchedulerを疑ってみると良いかもしれません。

### Subject

章の冒頭で

>　`Observable`は`subscribe(Observer)`を呼び出されること **蛇口が開き**、**初めて** 水が流れ出します。
> （**初めて** という言葉を強調したのは、例外も存在するからです。これについてはまた後で説明します。）

このように説明した例外が、この章で説明する`Subject`です。
`Subject`は初めから蛇口が開いている状態、つまり初めから値が流れています。
というと語弊があるかもしれませんが、`subscribe`されていなくても値を流すことができるということです。

iOSアプリ開発ではよくタップなどのユーザの起こしたアクションを受け取るために使われます。
その中でもいくつか方法はありますが`Observable`なので繋げた`Operator`で処理をする、`Pub/Sub`の値を流すように使われる事が多いです。

#### Subjectの種類

| Subject | 動作 |
|:--------:|-----|
|  |  |

### HotとCold

基本的に`Observable`は、`subscribe`するまで値が流れません。
その例外として`Subject`を利用した`Observable`は、常に値が流れています。
これを`Rx`の概念では`Hot`,`Cold`と言います。
`Subject`以外にも`Cold`を`Hot`へ変換する`Operator`が存在します。

```
let hotObservable = Observable
    .from(1...3)
    .publish()

hotObservable.subscribe(onNext: { i in
    print("subscribe onNext \(i)")
})

hotObservable.connect()
```

上記のコードは`publish`という`Cold`->`Hot`に変換する`Operator`を使った例です。
`Cold`->`Hot`の変換は、`multicast`というメソッドで`Cold`な`Observable`の`subscribe`時に`Subject`で包む仕組みです。
そして、`Hot`な`Observable`は自ら`subscribe`を呼び出します。

しかし、上記のコードで`o.subscribe()`した時点では、実は値は流れてきません。
`publish`で返ってくるのは`ConnectableObservable`という型で、`connect`されることで初めて値が流れます。
その時に`subscribe`されているものに対して値が流れます。

```
let hotObservable = Observable
        .from(1...3)
        .do(onNext: { print("onNext: \($0)") })
        .publish()

hotObservable.connect()

出力
onNext: 1
onNext: 2
onNext: 3
```

また、`subscribe`されなくとも値は流れることは上記のコードの出力見ればわかるでしょう。

`Hot`,`Cold`の違いは、他にもあります。
例えば、`Hot`な`Observable`は複数の`subscribe`に対してストリームを共有しています。

```
var count = 0
let hotObservable = Observable
    .from(1...3)
    .do(onNext: { i in
        count += 1
        print("doOnNext \(i), count:\(count)")
    })
    .publish()

hotObservable.subscribe(onNext: { i in
    print("1 subscribe onNext \(i)")
})
hotObservable.subscribe(onNext: { i in
    print("2 subscribe onNext \(i)")
})

hotObservable.connect()


出力
doOnNext 1, count:1
1 subscribe onNext 1
2 subscribe onNext 1
doOnNext 2, count:2
1 subscribe onNext 2
2 subscribe onNext 2
doOnNext 3, count:3
1 subscribe onNext 3
2 subscribe onNext 3
```

```
var count = 0
let coldObservable = Observable
    .from(1...3)
    .do(onNext: { i in
        count += 1
        print("doOnNext \(i), count:\(count)")
    })

coldObservable.subscribe(onNext: { i in
    print("1 subscribe onNext \(i)")
})
coldObservable.subscribe(onNext: { i in
    print("2 subscribe onNext \(i)")
})


出力
doOnNext 1, count:1
1 subscribe onNext 1
doOnNext 2, count:2
1 subscribe onNext 2
doOnNext 3, count:3
1 subscribe onNext 3
doOnNext 1, count:4
2 subscribe onNext 1
doOnNext 2, count:5
2 subscribe onNext 2
doOnNext 3, count:6
2 subscribe onNext 3
```

そのため、上記の出力のように複数の`subscribe`があった場合に、`1, 2, 3`がそれぞれイベントが共有され、2つの`subscribe`に対して **同時** に流れています。
`Cold`の場合はイベントが複製され、それぞれの`subscribe`に対して **別々** に流れています。

#### Cold -> Hot変換で注意すべきこと

`Cold`->`Hot`変換した`ConnectableObservable`では注意しないといけないことがあります。
それは、`connect`した`Observable`を`dispose`しないと開放されないということです。
しかし、正しいタイミングを気にしながら`dispose`するのはとても難しいことです。
そこで`ConnectableObservable`は`refCount()`を持っています。

```
let hotObservable = Observable
    .from(1...3)
    .publish()
    .refCount()

hotObservable.subscribe(onNext: { i in
    print("subscribe onNext \(i)")
})

出力
subscribe onNext 1
subscribe onNext 2
subscribe onNext 3
```

`refCount`を呼び出すと、内部で`connect`が呼び出しています。
そして`refCount`は全ての`subscribe`が`dispose`されると、自動で`Observable`を`dispose`してくれます。
これはiOSで使われているメモリ管理の`ARC`と同じ仕組みで`subscribe`毎にカウントアップし、`dispose`毎にカウントダウンされます。
そしてカウントが`0`になった時に`dispose`されるため、`dispose`のタイミングを意識しなくて良くなります。

`ConnectableObservable`は`connect`を呼び出した時に`subscribe`されているものに対して、値を流すと説明しました。
上記のコードは`refCount`内で`connect`されていて、その時点では`subscribe`されていません。
しかし、その後に呼び出した`subscribe`に値が流れています。
これは`refCount`により作用で、`refCount`を呼び出すと`connect`されているのにも関わらず、それ以降`subscribe`されるまで値が流れないようになります。

では、複数の`subscribe`をしてみましょう。

```
let hotObservable = Observable
    .from(1...3)
    .publish()
    .refCount()

hotObservable.subscribe(onNext: { i in
    print("1 subscribe onNext \(i)")
})
hotObservable.subscribe(onNext: { i in
    print("2 subscribe onNext \(i)")
})

出力
1 subscribe onNext 1
1 subscribe onNext 2
1 subscribe onNext 3
```

2つ`subscribe`しているはずが、最初の`subscribe`にしか流れていません。
これはストリームを共有しているため、初めに`subscribe`した時点でそれに対して値が流れ
2つ目の`subscribe`の時点では、既に流れたイベントは流れ終えているため流れないということです。

```
var count = 0
let hotObservable = Observable
    .from(1...3)
    .do(onNext: { i in
        count += 1
        print("doOnNext \(i), count:\(count)")
    })
    .replay(3)
    .refCount()

hotObservable.subscribe(onNext: { i in
    print("1 subscribe onNext \(i)")
})

hotObservable.subscribe(onNext: { i in
    print("2 subscribe onNext \(i)")
})

出力
doOnNext 1, count:1
1 subscribe onNext 1
doOnNext 2, count:2
1 subscribe onNext 2
doOnNext 3, count:3
1 subscribe onNext 3
2 subscribe onNext 1
2 subscribe onNext 2
2 subscribe onNext 3
```

`publish`は`PublishSubject`を利用するのに対して、`replay`は`ReplaySubject`を利用します。
これにより`publish`を`replay(3)`に変えることで、上記の出力のように2つ目の`subscribe`にも値が流れます。
3つ流れる値があるため、`replay(3)`としていますが、例えば`replay(1)`にすると値は`1`しか流れません。

また、`.publish().refCount()`は`share()`、`.replay().refCount()`は`shareReplay()`というエイリアス的なメソッドが用意されています。
ここまでは動作を理解するためにそれぞれを呼んでいましたが、動作を理解した上では`share(), shareReplay()`を使った方が良いでしょう。

## Observable.subscribe()の動作の流れ

`Observable.subscribe()`するとどんな流れで実行されるのか見ていきます。

Observableについての章では

> 殆どの`Operator`は`Observable.subscribe`を呼び出し、`Observable`を生成します。

このように説明し、Operatorの章では

> `subscribe`することで初めて生成した`Observable`が流れ出します。

このように説明し、Schedulerの章では

> きちんと`observeOn`,`subscribeOn`の動作を理解をするためには、`Observable.subscribe()`の動作を理解する必要があり...
> `observeOn`は下方向に適応し、`subscribeOn`は上方向に適応されるぐらいの認識...

このように説明しました。
実はこれらが動作の流れの全てです。もう少しコードを元に動作を詳しく見ていきましょう。
まずは`Observable.subscribe()`するとどうなるのか。

```
Observable.from(1...3)
          .subscribeOn(concurrentDispatchQueueScheduler)
          .map { $0 + 1 }
          .filter { $0 % 2 == 0 }
          .observeOn(mainScheduler)
          .subscribe(onNext: { i in
              print("onNext \(i)")
          }, onError: nil, onCompleted: nil, onDisposed: nil)
```

1. `subscribe`により、`map { $0 + 1 }`が`subscribe`される 
2. `map`により、`Observable.from(1...3)`が`subscribe`される

このように`.subscribe()`を呼び出した箇所から、上方向に`subscribe`していきます。
この時の`Scheduler`の指定が`subscribeOn`になります。
上記のコードでは、**map, filterはsubscribe()が呼び出されたスレッド、from()はsubscribeOn()で指定したスレッドでsubscribeが呼び出されます。**
そして、ここから下方向へ値が流れていきます。
`Observable`は値を一つ一つ流していくため

1. `from(1...3)`により`onNext`に`1`が流れる
2. `map { $0 + 1 }`により`2`になる
3. `filter { $0 % 2 == 0 }`により結果が`true`のため、通過する
4. `subscribe`に指定された`Observer`の`onNext`で`onNext 1`と出力される
5. 値が最後の`Observer`まで到達したため、`from(1...3)`により`onNext`に`2`が流れる
6. 値が全て流れるまで繰り返される
7. 全てが終わると`dispose`が呼ばれ、開放される

このように、初めの`Observable`から、下方向に`onXXX`を呼び出していきます。
この時の`Scheduler`の指定が`observeOn`になります。
上記のコードでは、**from, map, filterはfromのonXXXが呼び出されたスレッド、subscribeで指定されたObserverはobserveOn()で指定したスレッドでonXXXが呼び出されます。**

つまり、`subscribe`された時にそこから上へ進み、一番上の`Observable`まで到達したら下へ流れていきます。
そのため、`subscribeOn`は上方向に`subscribe`される時に適応され、`observeOn`は下方向に`onXXX`される時に適応されるということです。

## Operatorを自作してみる

さて、基本的な知識を学んできました。
ここでもう少し理解を深めるために、今までに使ってきた`Operator`を作ってみましょう。
`Operator`は`ObservableType`の`extension`として実装されています。
ちなみに`Observable`を最初に生成する`of, from, create`などのメソッドは`Observable`の`extension`として実装されています。

```
import RxSwift

public struct Logger {
    static func debug(_ string: String) {
        print(string)
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

Observable.from(1...3).myDebug(identifier: "from").subscribe()


出力
subscribed from
event from  next(1)
event from  next(2)
event from  next(3)
event from  completed
disposing from
```

上記のコードは既存で存在する`debug`という`Operator`を模して、独自ロガーで出力するようにしたものです。
既存の`Operator`を見ると`class`として実装されていますが、`create`を利用することで簡単に独自の`Operator`を作ることができます。
`create`に渡すクロージャの中で、`ObservableType`に宣言されている`subscribe`を呼び出すことで、自身が`subscribe`された時の動作を指定できます。
そこで各イベントの時に独自ロガーを使ってメッセージを出力しています。

## DisposeBag

iOSアプリを開発している中で`RxSwift`を使っていると複数の`Observable`を使うことになると思います。
しかし、それぞれきちんと`dispose`するのは大変ですし忘れてしまうこともあると思います。

そこで、`DisposeBag`というものがあります。
`DisposeBag`は複数の`Disposable`を管理し、`DisposeBag`のインスタンスが開放されるタイミングで管理している`Disposable`を開放してくれます。
`DisposeBag`への`Disposable`の追加は、`Observable`に対して`.disposed(by: DisposeBag)`するだけです。
これにより、例えば`UIViewController`の変数として`DisposeBag`のインスタンスを用意するだけで、`UIViewController`単位で`Observable`の`dispose`を管理することができます。


## おわりに

どうでしたか？「全然分からない。俺は雰囲気でObservableを使っている。」状態からは抜け出せたでしょうか？
本当は`ユースケースで学ぶRxSwift`というタイトルで書こうと思ったのですが、まず基本を理解しないといけないなと思って書いているとあっという間にページが埋まってしまいました。
また訪れるであろう技術書典で、今度こそかければと思っています。

また、本章の内容に間違い・紛らわしい内容があると思ったら Twitter: @roana0229 までメンションしていただけると嬉しいです。

[^1]: https://github.com/ReactiveX/RxSwift
[^2]: https://github.com/ReactiveX/RxSwift/tree/master/Rx.playground/Pages
[^3]: http://reactivex.io/documentation/observable.html
