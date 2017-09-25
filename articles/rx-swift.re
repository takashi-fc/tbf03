
= ユースケースで学ぶRxSwift再入門

== はじめに


みなさん、RxSwift@<fn>{1}使ってますか？
プロミス・データバインディング・イベントバス・リストをあれこれする処理など色々できて良いですよね。
ただ、「全然分からない。俺は雰囲気でObservableを使っている。」状態で使っていませんか？
本章は、（私含めて）そこからの脱却を目指す第一弾です。
今回は、@<tt>{RxSwift}を使う上での基本を学んでいきながら、最後に各ユースケースで使い方を学んでいきます。
本章の内容は、@<tt>{RxSwift}のgithubリポジトリにある@<tt>{Rx.playground}@<fn>{2}を主に参考にしています。



執筆時点の環境

 * Xcode: 9.0
 * Swift: 4.0
 * RxSwift: rxswift4.0-swift4.0ブランチ(cb0fb3bda41f2a5622909b6e6deafd435efaf859)


== 基本的なRxSwiftの基礎知識・考え方


RxSwiftを学ぶにあたって、まず最低限理解しておかないといけないものがあります。
これに関して殆どは、@<tt>{ReactiveX}全体で同様なので、他の言語で使ったことがある人は見慣れたものになると思います。


=== Observable と Observer の関係


公式サイト@<fn>{3}の説明を日本語訳すると


//quote{
@<tt>{Observer}は@<tt>{Observable}を購読します。

//}


と書かれています。
つまり、どういうことなの？と思うかもしれません。
イメージの付きやすいように考える場合、よく@<tt>{Observable}は川と例えられますが、個人的には蛇口の方が合ってるかと思っています。
その違いは、@<strong>{蛇口を開かないと水が流れない} というところです。
@<tt>{Observable}は@<tt>{subscribe}を呼び出されること @<strong>{蛇口が開き}、@<strong>{初めて} 水が流れ出します。
（@<strong>{初めて} という言葉を強調したのは、例外も存在するからです。これについてはまた後で説明します。）
そして、@<tt>{Observable}に流れている値は@<tt>{subscribe}に引数として渡した@<tt>{Observer}の@<tt>{onNext,onError,onComplete}に流れ着きます。
@<tt>{Observer}はObserverパターンと同じ意味合いです。なので、流れ着く＝通知されるということになります。


=== Operatorについて


@<tt>{Observable}を生成・変換するために@<tt>{Operator}が存在します。


//emlist{
Observable.of(1, 2, 3).map { $0 * $0 }
//}


例えば、上記のコードの@<tt>{of}や@<tt>{map}が@<tt>{Operator}になります。
他にも、@<tt>{Observable}を生成する@<tt>{Operator}は@<tt>{just,create}、変換する@<tt>{Operator}は@<tt>{map, flatMap, scan}などがあります。
まだまだ様々な@<tt>{Operator}が存在しますが、量が多いので必要に応じてユースケースで学ぶ章で説明をしようと思います。


=== Observerについて


章の冒頭で


//quote{
　@<tt>{Observable}は@<tt>{subscribe(Observer)}を呼び出されること @<strong>{蛇口が開き}、@<strong>{初めて} 水が流れ出します。

//}


このように言いました。



つまり先程@<tt>{Operator}について紹介する際に書いたコードだけでは何も起こりません。
下記のように@<tt>{subscribe}することで初めて生成した@<tt>{Observable}が実行されます。


//emlist{
Observable.of(1, 2, 3).map { $0 * $0 }.subscribe(onNext: { print($0) })
//}


このコードでは、@<tt>{1, 2, 3}という値がそれぞれ@<tt>{1 * 1}, @<tt>{2 * 2}, @<tt>{3 * 3}と@<tt>{map}で変換され、@<tt>{1, 4, 9}と出力されます。


==== Observerと追加された派生系の種類


RxSwift3.x系からは@<tt>{Observer}の他に、@<tt>{Single}, @<tt>{Compaletable}, @<tt>{Maybe}が追加されています。
これらは通知される@<tt>{onXXX}が違います。@<tt>{Observer}を含めて表にするとこのようになります。

//table[tbl1][]{
Observerの種類	動作
-----------------
@<tt>{Obsever}	@<tt>{onNext(value)}が1回以上、 @<tt>{onCompleted},@<tt>{onError(error)}がどちらか1回
@<tt>{Single}	@<tt>{onSuccess(value)},@<tt>{onError(error)}のどちらが1回
@<tt>{Compaletable}	@<tt>{onCompleted},@<tt>{onError(error)}がどちらか1回
@<tt>{Maybe}	@<tt>{onSuccess(value)},@<tt>{onCompleted},@<tt>{onError(error)}のどれかが1回
//}


また、@<tt>{onNext},@<tt>{onSuccess}内で例外が発生すると@<tt>{onError}が呼び出されます。 // TODO: onCompletedも同じ？


=== Scheduler


今までは値を流す方法@<tt>{Observable}や、流した後どこに着くのか@<tt>{Observer}について学びました。
実際にこれらを使う時の事を考えてみましょう。
例えば、通信処理で下記のような処理をするとします。

 1. リクエストを投げる
 1. レスポンスを受け取る
 1. 受け取った値をモデルなどにパースする
 1. できがったモデルをUIへ反映する



この時、1~3はバックグラウンドで、4はメインスレッドで処理をするのが適切です。
そのスレッドの制御をする役割を持つのが@<tt>{Scheduler}です。
@<tt>{Scheduler}はiOSの仕組みとして存在するGCD（Grand Central Dispatch）を利用しています。
それぞれの処理に@<tt>{Scheduler}を指定するために、@<tt>{observeOn},@<tt>{subscribeOn}メソッドを利用します。
@<tt>{observeOn}は@<tt>{Operator}での処理、@<tt>{subscribeOn}は@<tt>{Observer}での処理の@<tt>{Scheduler}を指定できます。


//emlist{
Observable.of(1, 2, 3)
          .observeOn(A-scheduler)
          .map { $0 + 1 }
          .observeOn(B-scheduler)
          .map { $0 * $0 }
          .subscribeOn(C-scheduler)
          .subscribe(onNext: { print($0) })
//}


このように指定することができます。
@<tt>{.map { $0 + 1 \}}は@<tt>{.observeOn(A-scheduler)}で指定された@<tt>{Scheduler}で動き
@<tt>{.map { $0 * $0 \}}は@<tt>{.observeOn(B-scheduler)}で指定された@<tt>{Scheduler}で動き
@<tt>{.subscribe(onNext: { print($0) \})}は@<tt>{.subscribeOn(C-scheduler)}で指定された@<tt>{Scheduler}で動きます。
もし@<tt>{.subscribeOn}が2回呼び出されていた場合は、後に呼び出した方の@<tt>{Scheduler}で動きます。
つまり、@<tt>{observeOn},@<tt>{subscribeOn}を呼び出した以降の処理はそれぞれ指定した@<tt>{Scheduler}で動きます。


==== Schedulerの種類


// TODO: 動作内容書く
| Scheduler | 動作 |
|:--------:|-----|
| MainScheduler | |
| ConcurrentMainScheduler | |
| SerialDispatchQueueScheduler | |
| ConcurrentDispatchQueueScheduler | |



それぞれ@<tt>{DispatchQueue}を持つ仕組みになっていて、引数として@<tt>{DispatchQoS}を渡すものと@<tt>{DispatchQueue}を渡すものがありますが、@<tt>{DispatchQoS}を渡した場合は、それをパラメータとして@<tt>{DispatchQueue}を生成しているので、どちらもスレッドに関しては同様の動作をします。
実際のコードはこのような感じになっていて、意図しない動作を減らすために@<tt>{DispatchQoS}を渡すイニシャライザを利用する方が良いと思います。


//list[ConcurrentDispatchQueueScheduler.swift][ConcurrentDispatchQueueScheduler.swift]{
public init(queue: DispatchQueue, leeway: DispatchTimeInterval = DispatchTimeInterval.nanoseconds(0)) {
    configuration = DispatchQueueConfiguration(queue: queue, leeway: leeway)
}

@available(iOS 8, OSX 10.10, *)
public convenience init(qos: DispatchQoS, leeway: DispatchTimeInterval = DispatchTimeInterval.nanoseconds(0)) {
    self.init(queue: DispatchQueue(
        label: "rxswift.queue.\(qos)",
        qos: qos,
        attributes: [DispatchQueue.Attributes.concurrent],
        target: nil),
        leeway: leeway
    )
}
//}

==== Schedulerで注意すべきこと


http://hadashia.hatenablog.com/entry/2016/03/17/212413



Conccurent（並列）な@<tt>{Scheduler}で処理されていても、1つの@<tt>{Observable}によって流れる値は順序が保証されています。
複数の@<tt>{Observable}は干渉しないため、それぞれをConccurent（並列）な@<tt>{Scheduler}を指定すれば、意図通り並列で動きます。
https://github.com/ReactiveX/RxSwift/blob/master/Documentation/GettingStarted.md#implicit-observable-guarantees // TODO: ソースこれであってる？



逆に、複数の@<tt>{Observable}を直列な@<tt>{Scheduler}で実行するとどうなるでしょうか？
// TODO: .zip系は中を同じスレッドで走らせると遅くなる？のでその動きを調べる



また、大事なのが@<tt>{Observable}を@<tt>{subscribe}した時、@<tt>{observeOn},@<tt>{subscribeOn}で@<tt>{Scheduler}を指定していない場合は今いるスレッドで実行するということです。
つまり、何も考えずにメインスレッドで動いている処理中に@<tt>{subscribe}してしまうと、重い@<tt>{Operator}の処理をメインスレッドで処理してい、描画やユーザの操作をブロックしてしまいます。


=== Subject(余力あれば)


ここで冒頭で説明した、この説明が関わってきます。


//quote{
@<tt>{Observable}は@<tt>{subscribe(Observer)}を呼び出されることで、@<strong>{初めて} 川が流れ出します。

//}


@<tt>{Hot} or @<tt>{Cold}
急に暑いとか寒いってなに？って感じですね。
実は@<tt>{subscribe(Observer)}されなくとも、川が流れ出す場合があります。
行き着く@<tt>{Observer}がないので、永遠に流れている川のようなイメージです。



http://qiita.com/KentaKudo/items/4d7154c3dada93f11492
http://qiita.com/k5n/items/98aaf84fc164f7a5502c



用途は主に同期的なUI操作のと気に使われる事が多いです。


== Operatorを自作してみる


さて、基本的な知識を学んできました。
川の流れと例えて来ましたが、実は理解する上で逆流する必要があります。
http://hadashia.hatenablog.com/entry/2016/02/23/175736
https://qiita.com/fu_nya/items/8e52f835737389f8492c



// TODO: observable, subscribeの数珠つなぎの仕組みはここで説明する？　https://qiita.com/k5n/items/643cc07e3973dd1fded4


//emlist{
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
//}

== ユースケースで学ぶ（通信編）


さて、ここまでRxSwiftを使う上での基本を学びました。
ここからはイメージの付きやすいように、TwitterにおけるユースケースでRxSwiftを学んでいきます。（皆さんTwitterは使っていますか？使っていますよね。）
※APIは実際のものではなく、模したAPIを利用しています。


=== ユーザのアカウントを認証する


説明


//emlist{
コード
//}

=== ユーザのアカウントを認証してから、ツイートの一覧を取得する


説明


//emlist{
コード
//}

=== ツイートの一覧からミュートしているユーザを除いて取得する（.zip + .filter）


説明


//emlist{
コード
//}

=== いいねとリツイートを混ぜて一覧として取得する（.merge）


説明


//emlist{
コード
//}

=== エラーハンドリング


通信処理はいつでも失敗する可能性がありますよね。この記事がとてもわかりやすいです。
https://academy.realm.io/posts/best-practices-pain-points-mobile-networking-rest-api-failures/


==== タイムアウトしたら、通知を出す。そしてもう一度実行するか聞く。

==== 返ってきたレスポンスのJSONスキーマが違って、モデルにパースできなかった。

==== ツイートしたけど、オフライン状態だった


通常の処理では、ツイートして返ってきたレスポンスを保存しますよね。
ただ、ツイートも失敗する可能性がありますよね。
公式アプリではその場合に下書きというところに保存するようになっています。


//emlist{
コード
//}

==== いいねしたけど、オフライン状態だった


いいねボタン、アニメーションもあって気持ちいいですよね。
ただ、失敗した時はいいねボタンをオフ状態に戻したかったりします。


//emlist{
コード
//}

==== ツイートを削除したけど、オフライン状態だった


リモートを削除したと同時に、ローカルを削除も削除したいですよね。
ただ、リモートが削除できた場合にのみ、ローカルを削除する必要がありますね。
しかし、PUTを叩いても204しか返ってこず、削除したツイートのIDがない場合も。


//emlist{
コード
//}

== ユースケースで学ぶ（UI編）

=== ツイートが空の場合にEmptyStateを表示したい


説明


//emlist{
コード
//}

=== ログインフォームのバリデーション


説明


//emlist{
コード
//}

=== チャットの他の人が入力中…みたいなやつ
 * チャット形式チュートリアルver（タイマーで自動投稿）
 ** 配列を回して次の文章を取る方法でチャット形式のチュートリアルできそう
 ** ユーザの入力待ち合わせも試す



説明


//emlist{
コード
//}

=== タブ間でいいねを同期したい


説明


//emlist{
コード
//}

== おわりに

//footnote[1][https://github.com/ReactiveX/RxSwift]

//footnote[2][https://github.com/ReactiveX/RxSwift/tree/master/Rx.playground/Pages]

//footnote[3][http://reactivex.io/documentation/observable.html]
