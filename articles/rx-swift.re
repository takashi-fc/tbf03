
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


== 登場人物とそれぞれの関係


本章で説明する登場人物は以下です。

 * Observable
 * Observer
 * Operator
 * Scheduler
 * Subject



公式サイト@<fn>{3}には


//quote{
In ReactiveX an observer subscribes to an Observable

//}


「 @<strong>{Observer} は @<strong>{Observable} を購読します」と書かれています。
ObserverはObserverパターンと同じ意味合いで使われ、通知される立場にあります。
Observableに流れているイベントが、Operatorという道を通って、Observerへ到達します。
RxSwiftを使うと何が嬉しいのかというと予め道とゴールを決めておき、そこに値が流れてくることができるため、データフローが明確になります。



​​イメージの付きやすいように例えると　@<strong>{Observableは蛇口}　、@<strong>{Operatorは水路}　のような感じです。重要なのは、 @<strong>{蛇口は開かないと水が流れない} という点です。
Observableはsubscribeが呼び出される（蛇口が開く）ことで、 @<strong>{初めて} 流れ出します。
（初めてという言葉を強調したのは、例外も存在するからです。これについてはまた後で説明します。）
そして、Observableに流れているイベントはOperator（水路）を通り、subscribeに引数として渡したObserverのonNext,onError,onCompleteに流れ着きます。



[イメージ図]


== Observableについて


Observableはストリームで、subscribeされることで初めてストリームが流れます。
そのストリームに値をonNext,onError,onCompletedというイベントに包んで、購読しているObserverへ通知します。
そしてonNextは複数回、onEror,onCompleteは一度だけ通知することができます。
onErrorもしくはonCompletedを呼んだ後は一切イベントが流れません。


== Operatorについて


OperatorはObservableをsubscribeし、Observableを生成します。


//emlist{
Observable.of(1, 2, 3).map { $0 * $0 }.filter { $0 % 2 == 0 }
//}


例えば、上記のコードの @<strong>{of} や @<strong>{map} が @<strong>{Operator} です。
他にも、Observableを生成するOperatorはjust, create、変換するOperatorはmap, flatMap, scanなどがあります。
Operatorは生成・変換するとObservableが返ってくるため、Observable,Operator同士は繋ぐことができます。


== Observerについて


登場人物を紹介したとき


//quote{
　Observableはsubscribeが呼び出される（蛇口が開く）ことで、 @<strong>{初めて} イベントが流れ出します。

//}


このように言いました。
つまり先程Operatorについて紹介する際に書いたコードだけでは何も起こりません。
下記のようにsubscribeすることで初めてObservableが流れ出します。


//emlist{
Observable.of(1, 2, 3).map { $0 * $0 }.subscribe(onNext: { print($0) })
//}


このコードでは、@<tt>{1, 2, 3}という値がonNextというイベントに包まれ、それぞれ@<tt>{1 * 1}, @<tt>{2 * 2}, @<tt>{3 * 3}と@<tt>{map}で変換され、ObserverのonNextへ通知され@<tt>{1, 4, 9}と出力されます。


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


ここまでイベントを通知するObservable、それを生成・変換するOperator、通知されるObserverについて学びました。
実際にこれらを使う時の事を考えてみましょう。
例えば、通信処理で下記のような処理をするとします。

 1. ツイートのIDを指定して、それを取得するリクエストを投げる
 1. レスポンスを受け取る
 1. 受け取ったJSONStringをTweetモデルに変換する
 1. できがったモデルをUIへ反映する



この時、1~3はバックグラウンドで、4はメインスレッドで処理をするのが適切です。
そのスレッドの制御をする役割を持つのが @<strong>{Scheduler} です。
@<strong>{Scheduler} はiOSの仕組みとして存在するGCD（Grand Central Dispatch）を利用することでスレッド制御を実現しています。
そのSchedulerを指定するためには @<strong>{observeOn} , @<strong>{subscribeOn} メソッドを利用します。


//emlist{
TweetService.requestTweet(by: ツイートのID)　　　　　　　　              　 // 1
            .map { JSONStringをTweetモデルへ変換($0) }                   // 2,3
            .subscribeOn(background-scheduler)
            .observeOn(main-scheduler)
            .subscribe(onNext: { label.text = $0.ツイートのテキスト変数 }) // 4
//}


これは実際にSchedulerを指定したコードの例です。
これで@<tt>{1~3}はバックグラウンドスレッド、@<tt>{4}はメインスレッドで動作します。
きちんとobserveOn,subscribeOnの動作を理解をするためには、 @<strong>{Observable.subscribe()} の動作を理解する必要があり、後で詳しくみていきます。
この章を見る上では @<strong>{observeOnは下方向} に適応し、 @<strong>{subscribeOnは上方向} に適応されるぐらいの認識で大丈夫です。


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


それぞれ DispatchQueueを持つ仕組みになっていて、イニシャライザには引数としてDispatchQoSを渡すものとDispatchQueueを渡すものがあります。
DispatchQoSを引数として渡すイニシャライザはiOS8から追加されていて、指定したいラベルがあるなどのことがなければDispatchQueueを渡すのではなく、DispatchQoSを渡す方が良いです。


=== Schedulerで注意すべきこと


まず、Concurrent（並列）なSchedulerで処理されていても、1つのObservableで流れる値は順序が保証されています。
そのため、このコードのようにランダムなスリープを挟んでも実行すると@<tt>{1, 2, 3}と値は順に出力されています。


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


では、同じ並列なScheduerで2つのObservableをsubscribeしたらどうなるでしょうか？


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


Concurrent（並列）なSchedulerで処理しているため、共有された変数countとObservableに流れる値が同じにならず、並列で動いていることがわかります。
では、Serial（直列）なSchedulerに変えてみましょう。


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


それぞれランダムにスリープがかかっていることにも関わらず、共有された変数countとObservableに流れる値が同じになっていて、直列に動いていることがわかります。



さて、このSchedulerの指定で大事なのがObservableをsubscribeした時、observeOn,subscribeOnでSchedulerを指定していない場合は　@<strong>{CurrentThreadScheduler(今いるスレッドで実行するScheduler)} で実行されるということです。
つまり、何も考えずにメインスレッドで動いている処理中にsubscribeしてしまうと、重い処理をメインスレッドで処理してしまいます。



普段コードを書いている時に、これまでに示してきたようにObservableをそれぞれ生成してsubscribeすることなんてないからあまり関係なさそうだと思う人がいるかもしれませんが、実は普段からよく発生している処理です。
それがどんなときかというと、zip,mergeなどObservableを結合して処理を行う場合です。


//emlist{
let observeOnScheduler = // ConcurrentDispatchQueueScheduler or SerialDispatchQueueScheduler

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


このコードは、今までに説明で利用してきたObservableのスリープ処理だけを残し変数化してzipで結合したものをsubscribeしています。
ConcurrentDispatchQueueSchedulerを指定した場合は、並列で走るため @<strong>{3秒スリープ×3回分} の時間で完了します。
しかし、SerialDispatchQueueSchedulerを指定した場合は、直列で走るため @<strong>{3秒スリープ×3回×2つObservable分} の時間がかかってしまいます。
通信処理の待ち合わせなどでzip,mergeなどを使っていて、なぜか遅いなと思ったらSchedulerを疑ってみると良いかもしれません。


== Subject


登場人物を紹介したとき


//quote{
Observableはsubscribeが呼び出される（蛇口が開く）ことで、 @<strong>{初めて} イベントが流れ出します。
（初めてという言葉を強調したのは、例外も存在するからです。これについてはまた後で説明します。）

//}


このように説明した例外が、この章で説明する @<strong>{Subject} です。
SubjectもObservableです。しかし、初めから蛇口が開いている状態、つまり初めから流れています。
というと語弊があるかもしれませんが、subscribeされていなくてもイベントを流すことができるということです。


//emlist{
let subject = PublishSubject<String>()
subject.onNext("1") // subscribeされていなくてもイベントを流せる
subject.subscribe(onNext: { print("subscribe: \($0)") })
subject.onNext("2")
subject.onCompleted()
subject.onNext("3") // Observableと同じく既にonCompletedが呼ばれているのでイベントが流れない

出力
A subscribe: 2
//}


基本はObservableと同じ性質であるため、onNext,onCompleted,onErrorを呼ぶことができます。
また、onCompleted,onErrorのどちらかが呼ばれてしまうと、その後はイベントを流すことができません。
そのため、上記のコードではsubscribeした後からonCompletedが呼ばれる前までのonNextのみの通知をObserverが受け取っています。


=== Subjectの種類
//table[tbl3][]{
Subject	動作
-----------------
PublishSubject	キャッシュせず、来たイベントをそのまま通知する
ReplaySubject	指定した値だけキャッシュし、subscribe時に直近のキャッシュしたものを通知する
BehaviorSubject	初期値を持つことができ、1つだけキャッシュし、subscribe時に直近のキャッシュしたものを通知する
Variable	変数のように扱うことができ、valueプロパティを変更するとonNextへ通知する ※RxSwift独自
//}

== HotとCold


基本的にObservableは、subscribeするまでイベントが流れません。
その例外としてSubjectを利用したObservableは、常にイベントが流れています。
これを @<strong>{ReactiveX} の概念では @<strong>{Hot,Cold} と言います。


//emlist{
let hotObservable = Observable
    .from(1...3)
    .publish()

hotObservable.subscribe(onNext: { i in
    print("subscribe onNext \(i)")
})

hotObservable.connect()
//}


上記の @<strong>{publish} というOperatorは @<strong>{Cold->Hot} に変換します。
Cold->Hotの変換は、multicastというメソッドで @<strong>{ColdなObservableのsubscribe時にSubjectで包む} 仕組みです。
また、HotなObservableは @<strong>{自らsubscribeを呼び出す} 性質を持ちます



しかし、上記のコードでhotObservable.subscribe()した時点では流れていません。
publishで返ってくるのはConnectableObservableという型で、connectされることで初めて流れ出し、その時にsubscribeされているものに対してイベントが流れます。


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


また、subscribeされなくとも値は流れることは上記のコードの出力見ればわかるでしょう。



Hot,Coldの違いは、他にもあります。
例えば、HotなObservableは複数のsubscribeに対して流れを共有しています。


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


流れを共有しているため、上記の出力のように複数のsubscribeがあった場合に@<tt>{1, 2, 3}がそれぞれイベントが共有され、2つのsubscribeに対して @<strong>{同時} に流れています。
Coldの場合はイベントが複製され、それぞれのsubscribeに対して @<strong>{別々} に流れています。


=== Cold -> Hot変換で注意すべきこと


Cold->Hot変換したConnectableObservableでは注意しないといけないことがあります。
それは、 @<strong>{connectしたObservableをdispose(unSubscribe)しないと開放されない} ということです。
しかし、正しいタイミングを気にしながらdisposeするのはとても難しいことです。
そこで、ConnectableObservableはrefCount()というメソッドを持っています。


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


@<strong>{refCount()} を呼び出すと、内部でconnectが呼び出しています。
そして、@<strong>{全てのsubscribeがdisposeされると、自動でObservableをdispose} してくれます。
これはiOSで使われているメモリ管理のARCと同じ仕組みで @<strong>{subscribe毎にカウントアップし、dispose毎にカウントダウン} されます。
そしてカウントが0になった時にdisposeされるため、disposeのタイミングを意識しなくて良くなります。


//quote{
publishで返ってくるのはConnectableObservableという型で、connectされることで初めて流れ出し、その時にsubscribeされているものに対してイベントが流れます。

//}


と説明しました。上記のコードはrefCount内でconnectされていて、その時点ではsubscribeされていません。
しかし、その後に呼び出したsubscribeに値が流れています。
これはrefCountによる作用で、refCount()を呼び出すと @<strong>{connectされているのにも関わらず、それ以降subscribeされるまで値が流れない} ようになります。



では、次に複数のsubscribeをしてみましょう。


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


2つsubscribeしているはずが、最初のsubscribeにしかイベントが流れていません。
これは流れを共有しているため、初めにsubscribeした時点でそれに対してイベントが流れ、2つ目のsubscribeの時点では、既に流れたイベントは流れ終えているため流れないということです。


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


これはreplyというCold->Hot変換のOperatorを使ったコードです。
publishはPublishSubjectを利用するのに対して、replayはReplaySubjectを利用します。
ReplaySubjectは指定した数のキャッシュを持つため、上記の出力のように2つ目のsubscribeにもキャッシュしてあるイベントが流れます。
3つ流れるイベントがあるため、replay(3)としていますが、例えばreplay(1)にするとイベントはonNext(1)しか流れません。



また、 @<strong>{.publish().refCount()はshare()} 、 @<strong>{.replay().refCount()はshareReplay()} というエイリアス的なメソッドが用意されています。
ここまでは動作を理解するためにそれぞれを呼んでいましたが、動作を理解した上ではshare(),shareReplay()を使った方が良いでしょう。


== Observable.subscribe()の動作


これまで、Obserbavle,Operator,Schedulerのそれぞれの章で


//quote{
Observableはストリームで、subscribeされることで初めてストリームが流れます。
OperatorはObservableをsubscribeし、Observableを生成します。
@<strong>{observeOnは下方向} に適応し、 @<strong>{subscribeOnは上方向} に適応されるぐらいの認識で大丈夫です。

//}


このように説明しました。そして、実はこれらが動作の流れの全体です。
Observable.subscribe()するとどんな流れで実行されるのか、もう少しコードを元に動作を詳しく見ていきましょう。


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
