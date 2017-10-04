
= RxSwift再入門 ~雰囲気から抜け出そう~

== はじめに


みなさん、RxSwift@<fn>{1}使ってますか？
プロミス・データバインディング・イベントバス・リストをあれこれする処理など色々できて良いですよね。
ただ、「全然分からない。俺は雰囲気でRxSwiftを使っている。」状態で使っていませんか？
恥ずかしながら私は完全に雰囲気で使っていました。
本章は、（私含めて）そこからの脱却を目指す第一弾です。
今回は、@<tt>{RxSwift}を使う上での基本を抑えつつ、どうやって動いているのかを学んでいきます。
本章の内容は、@<tt>{RxSwift}のgithubリポジトリにある@<tt>{Rx.playground}@<fn>{2}を主に参考にしています。



対象とする読者のイメージ

 * RxSwiftを使ったことがある
 * いまいち理解せず雰囲気で使っている



執筆時点の環境

 * Xcode: 9.0
 * Swift: 3.1
 * ReactiveX/RxSwift: master（bd5a9657b9a7cf52f583eecf00dc8b7c0cb9ebaa）


== 登場人物


本章で説明する登場人物は以下です。

 * Observable
 * Observer
 * Operator
 * Scheduler
 * Subject



登場人物達の繋がり



[イメージ図]



Observableで流れてくる値を、Operatorという道を通って、Observerへ到達します。
RxSwiftを使うと何が嬉しいのかというと予め道とゴールを決めておき、そこに値が流れてくることができるため、データフローが明確になります。


== Observable と Observer の関係


公式サイト@<fn>{3}の説明を日本語訳すると


//quote{
@<tt>{Observer}は@<tt>{Observable}を購読します。

//}


と書かれています。
​​イメージの付きやすいように例えると、蛇口と水路のような感じです。重要なのは、 @<strong>{蛇口は開かないと水が流れない} という点です。
@<tt>{Observable}は@<tt>{subscribe}が呼び出される（蛇口が開く）ことで、 @<strong>{初めて} 値が流れ出します。
（初めてという言葉を強調したのは、例外も存在するからです。これについてはまた後で説明します。）
そして、@<tt>{Observable}に​​流れている値は@<tt>{Operator}（水路）を通り、@<tt>{subscribe}に引数として渡した@<tt>{Observer}の@<tt>{onNext, onError, onComplete}に流れ着きます。
@<tt>{Observer}はObserverパターンと同じ意味合いです。なので、流れ着く＝通知されるということになります。


== Operatorについて


@<tt>{Observable}を生成・変換するために@<tt>{Operator}が存在します。


//emlist{
Observable.of(1, 2, 3, 4).map { $0 * $0 }.filter { $0 % 2 == 0 }
//}


例えば、上記のコードの@<tt>{of}や@<tt>{map}が@<tt>{Operator}になります。
他にも、@<tt>{Observable}を生成する@<tt>{Operator}は@<tt>{just, create}、変換する@<tt>{Operator}は@<tt>{map, flatMap, scan}などがあります。
@<tt>{Operator}は@<tt>{Observable.subscribe}を呼び出し、@<tt>{Observable}を生成します。
そのため上記のコードように、@<tt>{Operator}はメソッドチェーンをすることができます。


== Observerについて


章の冒頭で


//quote{
　@<tt>{Observable}は@<tt>{subscribe(Observer)}を呼び出されること @<strong>{蛇口が開き}、@<strong>{初めて} 水が流れ出します。

//}


このように言いました。



つまり先程@<tt>{Operator}について紹介する際に書いたコードだけでは何も起こりません。
下記のように@<tt>{subscribe}することで初めて生成した@<tt>{Observable}が流れ出します。


//emlist{
Observable.of(1, 2, 3).map { $0 * $0 }.subscribe(onNext: { print($0) })
//}


このコードでは、@<tt>{1, 2, 3}という値がそれぞれ@<tt>{1 * 1}, @<tt>{2 * 2}, @<tt>{3 * 3}と@<tt>{map}で変換され、@<tt>{1, 4, 9}と出力されます。


=== ObserverとRxSwift3.xで追加された派生系の種類


RxSwift3.x系からは@<tt>{Observer}の他に、@<tt>{Single}, @<tt>{Completable}, @<tt>{Maybe}が追加されています。
これらは通知される@<tt>{onXXX}が違います。@<tt>{Observer}を含めて表にするとこのようになります。

//table[tbl1][]{
Observerの種類	動作
-----------------
@<tt>{Obsever}	@<tt>{onNext(value)}が1回以上、 @<tt>{onCompleted},@<tt>{onError(error)}がどちらか1回
@<tt>{Single}	@<tt>{onSuccess(value)},@<tt>{onError(error)}のどちらが1回
@<tt>{Compaletable}	@<tt>{onCompleted},@<tt>{onError(error)}がどちらか1回
@<tt>{Maybe}	@<tt>{onSuccess(value)},@<tt>{onCompleted},@<tt>{onError(error)}のどれかが1回
//}

== Scheduler


今までは値を流す方法@<tt>{Observable}や、流した後どこに着くのか@<tt>{Observer}について学びました。
実際にこれらを使う時の事を考えてみましょう。
例えば、通信処理で下記のような処理をするとします。

 1. ツイートのIDを指定して、それを取得するリクエストを投げる
 1. レスポンスを受け取る
 1. 受け取ったJSONStringをTweetモデルに変換する
 1. できがったモデルをUIへ反映する



この時、1~3はバックグラウンドで、4はメインスレッドで処理をするのが適切です。
そのスレッドの制御をする役割を持つのが@<tt>{Scheduler}です。
@<tt>{Scheduler}はiOSの仕組みとして存在するGCD（Grand Central Dispatch）を利用しています。
また、@<tt>{Scheduler}を指定するためには@<tt>{observeOn},@<tt>{subscribeOn}メソッドを利用します。


//emlist{
TweetService.requestTweet(by: ツイートのID)　　　　　　　　              　 // 1
            .map { JSONStringをTweetモデルへ変換($0) }                   // 2,3
            .subscribeOn(background-scheduler)
            .observeOn(main-scheduler)
            .subscribe(onNext: { label.text = $0.ツイートのテキスト変数 }) // 4
//}


このように指定することができます。
これで@<tt>{1~3}はバックグラウンドスレッド、@<tt>{4}はメインスレッドで動作します。
きちんと@<tt>{observeOn},@<tt>{subscribeOn}の動作を理解をするためには、@<tt>{Observable.subscribe()}の動作を理解する必要があり、次の章で詳しくみていきます。
この章を見る上では@<tt>{observeOn}は下方向に適応し、@<tt>{subscribeOn}は上方向に適応されるぐらいの認識で大丈夫です。


=== Schedulerの種類
//table[tbl2][]{
Scheduler	動作
-----------------
MainScheduler	メインスレッドで動きます ※observeOnに最適化されています
ConcurrentMainScheduler	メインスレッドで動きます ※subscribeOnに最適化されています
CurrentThreadScheduler	現在のスレッドで動きます
SerialDispatchQueueScheduler	指定されたQOSで生成された直列なQueueで動きます
ConcurrentDispatchQueueScheduler	指定されたQOSで生成された並列なQueueで動きます
//}


それぞれ@<tt>{DispatchQueue}を持つ仕組みになっていて、イニシャライザには引数として@<tt>{DispatchQoS}を渡すものと@<tt>{DispatchQueue}を渡すものがあります。
@<tt>{DispatchQoS}を引数として渡すイニシャライザは@<tt>{iOS8}から追加されていて、指定したいラベルがあるなどのことがなければ@<tt>{DispatchQueue}を渡すのではなく、@<tt>{DispatchQoS}を渡す方が良いです。


=== Schedulerで注意すべきこと


Concurrent（並列）な@<tt>{Scheduler}で処理されていても、1つの@<tt>{Observable}で流れる値は順序が保証されています。
そのため、このコードのようにスリープを挟んでも実行すると下記のように出力されます。


//emlist{
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
//}


では、先程のスケジューラで2つのObservableをsubscribeしたらどうなるでしょうか？


//emlist{
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
//}


@<tt>{ConcurrentScheduler(並列)}で処理しているため、共有された変数@<tt>{count}と@<tt>{Observable}に流れる数値が同じにならず、並列で動いていることがわかります。
では同じ@<tt>{Observable}で@<tt>{SerialScheduler(直列)}を利用してみましょう。


//emlist{
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
//}


それぞれスリープがかかっていることにも関わらず、共有された変数@<tt>{count}と@<tt>{Observable}に流れる数値が同じになっていて、直列に動いていることがわかります。



さて、このSchedulerの指定で大事なのが@<tt>{Observable}を@<tt>{subscribe}した時、@<tt>{observeOn},@<tt>{subscribeOn}で@<tt>{Scheduler}を指定していない場合は@<tt>{CurrentThreadScheduler(今いるスレッド)}で実行するということです。
つまり、何も考えずにメインスレッドで動いている処理中に@<tt>{subscribe}してしまうと、重い処理をメインスレッドで処理してしまいます。



普段コードを書いている時に、ここまで出てきたように@<tt>{Observable}をそれぞれ生成して@<tt>{subscribe}することなんてないからあまり関係なさそうだと思う人がいるかもしれませんが、実は普段からよく発生している処理です。
それがどんなときかというと、@<tt>{zip}, @<tt>{merge}など@<tt>{Observable}を結合して処理を行う場合です。


//emlist{
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
//}


このコードは、今までに説明で利用してきた@<tt>{Observable}のスリープ処理だけを残し変数化して@<tt>{Observable.zip}で結合したものを@<tt>{subscribe}しています。
@<tt>{let observeOnScheduler}に@<tt>{ConcurrentDispatchQueueScheduler}を指定した場合は、並列で走るため@<tt>{3秒スリープ×3回}分の時間で完了します。
しかし、@<tt>{SerialDispatchQueueScheduler}を指定した場合は、直列で走るため@<tt>{3秒スリープ×3回×2つObservable}分の時間がかかってしまいます。
通信処理の待ち合わせなどで@<tt>{zip},@<tt>{merge}などを使っていて、なぜか遅いなと思ったらSchedulerを疑ってみると良いかもしれません。


== Subject


章の冒頭で


//quote{
　@<tt>{Observable}は@<tt>{subscribe(Observer)}を呼び出されること @<strong>{蛇口が開き}、@<strong>{初めて} 水が流れ出します。
（@<strong>{初めて} という言葉を強調したのは、例外も存在するからです。これについてはまた後で説明します。）

//}


このように説明した例外が、この章で説明する@<tt>{Subject}です。
@<tt>{Subject}は初めから蛇口が開いている状態、つまり初めから値が流れています。
というと語弊があるかもしれませんが、@<tt>{subscribe}されていなくても値を流すことができるということです。


//emlist{
let subject = PublishSubject<String>()
subject.onNext("1")
subject.subscribe(onNext: { print("subscribe: \($0)") })
subject.onNext("2")
subject.onCompleted()
subject.onNext("3")

出力
A subscribe: 2
//}


@<tt>{Subject}も@<tt>{Observable}であり、@<tt>{onNext, onCompleted, onError}を呼ぶことができます。
しかし、@<tt>{onCompleted, onError}のどちらかが呼ばれてしまうと、その後はイベントを流すことができません。
そのため、上記のコードでは@<tt>{subscribe}した後から@<tt>{onCompleted}が呼ばれる前までの@<tt>{onNext}のみ受け取っています。


=== Subjectの種類
//table[tbl3][]{
Subject	動作
-----------------
PublishSubject	キャッシュせず、来たイベントをそのまま通知する
ReplaySubject	指定した値だけキャッシュし、subscribe時に直近のキャッシュしたものを通知する
BehaviorSubject	初期値を持つことができ、1つだけキャッシュし、subscribe時に直近のキャッシュしたものを通知する
Variable	変数のように扱うことができ、valueプロパティを変更するとonNextへ通知する
//}

== HotとCold


基本的に@<tt>{Observable}は、@<tt>{subscribe}するまで値が流れません。
その例外として@<tt>{Subject}を利用した@<tt>{Observable}は、常に値が流れています。
これを@<tt>{Rx}の概念では@<tt>{Hot},@<tt>{Cold}と言います。


//emlist{
let hotObservable = Observable
    .from(1...3)
    .publish()

hotObservable.subscribe(onNext: { i in
    print("subscribe onNext \(i)")
})

hotObservable.connect()
//}


上記のコードは@<tt>{publish}という@<tt>{Cold}->@<tt>{Hot}に変換する@<tt>{Operator}を使った例です。
@<tt>{Cold}->@<tt>{Hot}の変換は、@<tt>{multicast}というメソッドで@<tt>{Cold}な@<tt>{Observable}の@<tt>{subscribe}時に@<tt>{Subject}で包む仕組みです。
また、@<tt>{Hot}な@<tt>{Observable}は自ら@<tt>{subscribe}を呼び出す性質を持ちます



しかし、上記のコードで@<tt>{o.subscribe()}した時点では、実は値は流れてきません。
@<tt>{publish}で返ってくるのは@<tt>{ConnectableObservable}という型で、@<tt>{connect}されることで初めて値が流れます。
その時に@<tt>{subscribe}されているものに対して値が流れます。


//emlist{
let hotObservable = Observable
        .from(1...3)
        .do(onNext: { print("onNext: \($0)") })
        .publish()

hotObservable.connect()

出力
onNext: 1
onNext: 2
onNext: 3
//}


また、@<tt>{subscribe}されなくとも値は流れることは上記のコードの出力見ればわかるでしょう。



@<tt>{Hot},@<tt>{Cold}の違いは、他にもあります。
例えば、@<tt>{Hot}な@<tt>{Observable}は複数の@<tt>{subscribe}に対してストリームを共有しています。


//emlist{
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
//}

//emlist{
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
//}


そのため、上記の出力のように複数の@<tt>{subscribe}があった場合に、@<tt>{1, 2, 3}がそれぞれイベントが共有され、2つの@<tt>{subscribe}に対して @<strong>{同時} に流れています。
@<tt>{Cold}の場合はイベントが複製され、それぞれの@<tt>{subscribe}に対して @<strong>{別々} に流れています。


=== Cold -> Hot変換で注意すべきこと


@<tt>{Cold}->@<tt>{Hot}変換した@<tt>{ConnectableObservable}では注意しないといけないことがあります。
それは、@<tt>{connect}した@<tt>{Observable}を@<tt>{dispose}しないと開放されないということです。
しかし、正しいタイミングを気にしながら@<tt>{dispose}するのはとても難しいことです。
そこで@<tt>{ConnectableObservable}は@<tt>{refCount()}を持っています。


//emlist{
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
//}


@<tt>{refCount}を呼び出すと、内部で@<tt>{connect}が呼び出しています。
そして@<tt>{refCount}は全ての@<tt>{subscribe}が@<tt>{dispose}されると、自動で@<tt>{Observable}を@<tt>{dispose}してくれます。
これはiOSで使われているメモリ管理の@<tt>{ARC}と同じ仕組みで@<tt>{subscribe}毎にカウントアップし、@<tt>{dispose}毎にカウントダウンされます。
そしてカウントが@<tt>{0}になった時に@<tt>{dispose}されるため、@<tt>{dispose}のタイミングを意識しなくて良くなります。



@<tt>{ConnectableObservable}は@<tt>{connect}を呼び出した時に@<tt>{subscribe}されているものに対して、値を流すと説明しました。
上記のコードは@<tt>{refCount}内で@<tt>{connect}されていて、その時点では@<tt>{subscribe}されていません。
しかし、その後に呼び出した@<tt>{subscribe}に値が流れています。
これは@<tt>{refCount}による作用で、@<tt>{refCount}を呼び出すと@<tt>{connect}されているのにも関わらず、それ以降@<tt>{subscribe}されるまで値が流れないようになります。



では、複数の@<tt>{subscribe}をしてみましょう。


//emlist{
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
//}


2つ@<tt>{subscribe}しているはずが、最初の@<tt>{subscribe}にしか流れていません。
これはストリームを共有しているため、初めに@<tt>{subscribe}した時点でそれに対して値が流れ
2つ目の@<tt>{subscribe}の時点では、既に流れたイベントは流れ終えているため流れないということです。


//emlist{
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
//}


@<tt>{publish}は@<tt>{PublishSubject}を利用するのに対して、@<tt>{replay}は@<tt>{ReplaySubject}を利用します。
これにより@<tt>{publish}を@<tt>{replay(3)}に変えることで、上記の出力のように2つ目の@<tt>{subscribe}にも値が流れます。
3つ流れる値があるため、@<tt>{replay(3)}としていますが、例えば@<tt>{replay(1)}にすると値は@<tt>{1}しか流れません。



また、@<tt>{.publish().refCount()}は@<tt>{share()}、@<tt>{.replay().refCount()}は@<tt>{shareReplay()}というエイリアス的なメソッドが用意されています。
ここまでは動作を理解するためにそれぞれを呼んでいましたが、動作を理解した上では@<tt>{share(), shareReplay()}を使った方が良いでしょう。


== Observable.subscribe()の動作の流れ


@<tt>{Observable.subscribe()}するとどんな流れで実行されるのか見ていきます。



Observableについての章では


//quote{
殆どの@<tt>{Operator}は@<tt>{Observable.subscribe}を呼び出し、@<tt>{Observable}を生成します。

//}


このように説明し、Operatorの章では


//quote{
@<tt>{subscribe}することで初めて生成した@<tt>{Observable}が流れ出します。

//}


このように説明し、Schedulerの章では


//quote{
きちんと@<tt>{observeOn},@<tt>{subscribeOn}の動作を理解をするためには、@<tt>{Observable.subscribe()}の動作を理解する必要があり...
@<tt>{observeOn}は下方向に適応し、@<tt>{subscribeOn}は上方向に適応されるぐらいの認識...

//}


このように説明しました。
実はこれらが動作の流れの全てです。もう少しコードを元に動作を詳しく見ていきましょう。
まずは@<tt>{Observable.subscribe()}するとどうなるのか。


//emlist{
Observable.from(1...3)
          .subscribeOn(concurrentDispatchQueueScheduler)
          .map { $0 + 1 }
          .filter { $0 % 2 == 0 }
          .observeOn(mainScheduler)
          .subscribe(onNext: { i in
              print("onNext \(i)")
          }, onError: nil, onCompleted: nil, onDisposed: nil)
//}
 1. @<tt>{subscribe}により、@<tt>{map { $0 + 1 \}}が@<tt>{subscribe}される 
 1. @<tt>{map}により、@<tt>{Observable.from(1...3)}が@<tt>{subscribe}される



このように@<tt>{.subscribe()}を呼び出した箇所から、上方向に@<tt>{subscribe}していきます。
この時の@<tt>{Scheduler}の指定が@<tt>{subscribeOn}になります。
上記のコードでは、@<strong>{map, filterはsubscribe()が呼び出されたスレッド、from()はsubscribeOn()で指定したスレッドでsubscribeが呼び出されます。}
そして、ここから下方向へ値が流れていきます。
@<tt>{Observable}は値を一つ一つ流していくため

 1. @<tt>{from(1...3)}により@<tt>{onNext}に@<tt>{1}が流れる
 1. @<tt>{map { $0 + 1 \}}により@<tt>{2}になる
 1. @<tt>{filter { $0 % 2 == 0 \}}により結果が@<tt>{true}のため、通過する
 1. @<tt>{subscribe}に指定された@<tt>{Observer}の@<tt>{onNext}で@<tt>{onNext 1}と出力される
 1. 値が最後の@<tt>{Observer}まで到達したため、@<tt>{from(1...3)}により@<tt>{onNext}に@<tt>{2}が流れる
 1. 値が全て流れるまで繰り返される
 1. 全てが終わると@<tt>{dispose}が呼ばれ、開放される



このように、初めの@<tt>{Observable}から、下方向に@<tt>{onXXX}を呼び出していきます。
この時の@<tt>{Scheduler}の指定が@<tt>{observeOn}になります。
上記のコードでは、@<strong>{from, map, filterはfromのonXXXが呼び出されたスレッド、subscribeで指定されたObserverはobserveOn()で指定したスレッドでonXXXが呼び出されます。}



つまり、@<tt>{subscribe}された時にそこから上へ進み、一番上の@<tt>{Observable}まで到達したら下へ流れていきます。
そのため、@<tt>{subscribeOn}は上方向に@<tt>{subscribe}される時に適応され、@<tt>{observeOn}は下方向に@<tt>{onXXX}される時に適応されるということです。


== Operatorを自作してみる


さて、基本的な知識を学んできました。
ここでもう少し理解を深めるために、今までに使ってきた@<tt>{Operator}を作ってみましょう。
@<tt>{Operator}は@<tt>{ObservableType}の@<tt>{extension}として実装されています。
ちなみに@<tt>{Observable}を最初に生成する@<tt>{of, from, create}などのメソッドは@<tt>{Observable}の@<tt>{extension}として実装されています。


//emlist{
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
//}


上記のコードは既存で存在する@<tt>{debug}という@<tt>{Operator}を模して、独自ロガーで出力するようにしたものです。
既存の@<tt>{Operator}を見ると@<tt>{class}として実装されていますが、@<tt>{create}を利用することで簡単に独自の@<tt>{Operator}を作ることができます。
@<tt>{create}に渡すクロージャの中で、@<tt>{ObservableType}に宣言されている@<tt>{subscribe}を呼び出すことで、自身が@<tt>{subscribe}された時の動作を指定できます。
そこで各イベントの時に独自ロガーを使ってメッセージを出力しています。


== おわりに


どうでしたか？「全然分からない。俺は雰囲気でObservableを使っている。」状態からは抜け出せたでしょうか？
本当は@<tt>{ユースケースで学ぶRxSwift}というタイトルで書こうと思ったのですが、まず基本を理解しないといけないなと思って書いているとあっという間にページが埋まってしまいました。
また訪れるであろう技術書典で、今度こそかければと思っています。



また、本章の内容に間違い・紛らわしい内容があると思ったら Twitter: @roana0229 までメンションしていただけると嬉しいです。


//footnote[1][https://github.com/ReactiveX/RxSwift]

//footnote[2][https://github.com/ReactiveX/RxSwift/tree/master/Rx.playground]

//footnote[3][http://reactivex.io/documentation/observable.html]
