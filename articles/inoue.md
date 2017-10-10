# 動画再生機能を作ってみよう

### はじめに
* 背景
* 動画再生最近流行ってる
* やってみたいと思った
* 今回の記事でやることざっと紹介
* 動画再生に使用するオブジェクト紹介
* Avplayerの基本
* シンプルな動画再生（アセットからの再生）
* youtubeの再生（ライブラリ使用）
* リポジトリ検索機能つき

### 動画再生するには？
#### 動画再生方法
　動画再生を再生する手段としては2つあります。ざっと紹介していきます。

* UIWebView
* AVPlayer

##### UIWebView
　最も簡単な方法でいうと、こちらの方法になります。`WebView`は、Webページをアプリ内で表示することができる`View`です。
　使い方はシンプルで、URLがあればすぐに実装できてしまいます。
　例えば、`WebView`で`YouTube`を指定すれば、そのアプリはWeb版のYouTubeとまるっきり同じになります。なので、動画再生もアプリ上でそのままされるということになります。アプリ内にブラウザを置けるようなものですね。
　今回`WebView`はメインで取り上げません。以下サンプルコードで動作すると思うので、試してみてください。

```swift:WebViewサンプルコード
import UIKit

class SampleWebViewController: UIViewController {

  @IBOutlet weak var webView: UIWebView!

    override func viewDidLoad() {
      super.viewDidLoad()

        let requestURL = URL(string: "https://www.youtube.com/")
        let req = URLRequest(url: requestURL!)
        webView.loadRequest(req)
    }
}

```

　注意点としては、実際に表示されるのはWebページなので、ボタン等のオブジェクトに対して操作をすることができません。単純にWebページを表示させたい時に利用してみてください。
　
##### AVPlayer
　`WebView`ではカスタマイズができませんでしたが、こちらのライブラリでは色々カスタマイズすることができます。本格的に動画再生機能を実装したい場合、現在だとこちらを使うことになると思います。例えば、動画再生・停止を制御したり、音声を調整したりです。実装方法は`WebView`と比べるとコード量は増えますが、圧倒的にできることが増えます。
　今回は`AVPlayer`を使った実装を試したので、後ほどの節でサンプルコードとともに紹介します。
　ちなみに、`AVPlayerViewController`オブジェクトが`StoryBoard`に用意されているので、そちらを使えば簡単に`View`を作ることができますよ。
　
#### おまけ
　iOS2~iOS9の間では、`MPMoviePlayer`というものが標準ライブラリで使えました。しかしiOS9から`deprecated`なり、それ以降は`AVPlayer`がメインになったようです。
　なので、簡単に実装したいときは`WebView`、カスタマイズしたい場合は`AVPlayer`と覚えておくと良いかもしれません。

### シンプルな動画再生機能実装
では`AVPlayer`を使った動画再生機能の実装手順を、以下のような流れで紹介します。今回は、アセットに動画ファイルを登録し、アプリロード時直後に動画を再生する機能を実装してみます。

* 動画ファイルの登録
* `ViewController`の設定
* ストーリーボードの実装
* アプリ起動

#### アセットに動画ファイルの登録
　今回は、mp4形式の動画ファイルを用意しています。movファイルでも問題ありません。まずは、動画ファイルをアセットに登録しましょう。ドラックするだけでOKです。登録したアセット名はこの後利用するので、覚えておきましょう。

![asset](images/sample-asset.png)

#### `ViewController`の設定
  　`ViewController`は`AVPlayerViewController`を継承したクラスを作成します。そして今回は、ロード時に動画再生をするので`viewDidLoad`に動画読み込み処理を記述していきます。
  　まずは動画ファイルを参照します。動画ファイルは、アセットから`NSDataAsset `として取り出し、一時ファイルとして`NSTemporaryDirectory`に一度保存します。その保存先URLを参照する形になります。
  　まず、一時ファイルとして保存するサンプルコードは以下のようになります。

  ```
  let asset = NSDataAsset(name:"アセット名")
  let videoUrl = URL(fileURLWithPath: NSTemporaryDirectory()).appendingPathComponent("アセット名.ファイル形式")
try! asset!.data.write(to: videoUrl)
  ```

  　次はURLを参照して動画を再生させるコードになります。
  　サンプルコードは以下のようになります。動画ファイルURLから`AVPlayerItem`を作成し、`AVPlayer`にセットする感じですね。そして、その`AVPlayer`インスタンスを`AVPlayerViewController `がもつプロパティ`player`に代入します。あとは、`AVPlayer`オブジェクトがもつ制御メソッドを実行するだけで、動画ファイルの制御ができます。今回は再生するだけにしておきます。

  ```
  let item = AVPlayerItem(url: videoUrl)
let videoPlayer = AVPlayer(playerItem: item)
  player = videoPlayer
player?.play()
  ```

  　クラスの全体像は以下の通りです。

  ```swift:AVPlayerViewControllerのサンプルコード
  import UIKit
  import AVKit

  class SampleAVPlayerViewController: AVPlayerViewController {

    override func viewDidLoad() {
      super.viewDidLoad()

        let asset = NSDataAsset(name:"アセット名")
        let videoUrl = URL(fileURLWithPath: NSTemporaryDirectory()).appendingPathComponent("アセット名.ファイル形式")
        try! asset!.data.write(to: videoUrl)
        let item = AVPlayerItem(url: videoUrl)
        let videoPlayer = AVPlayer(playerItem: item)
        player = videoPlayer
        player?.play()
    }
  }
```

#### ストーリーボードの実装
　`AVPlayer`を使って再生するために用意されている、`AVKitPlayerViewController`を使います。まず、オブジェクトライブラリーから以下のアイコンを選択して、引っ張り出しましょう。

![object](images/sample-object-library.png)

  　ロード直後に動画を再生させるため、`initial ViewController`に設定しておきましょう。そして、先ほど作成した`ViewController`を`Custom Class`に設定します。

#### アプリ起動
  　これでアプリを起動してみましょう。アプリが起動後、動画の読み込みが始まり、再生されます。シンプルな再生機能はこれで実装できますが、`AVPlayer`オブジェクトが持つメソッドを駆使すれば、ボタンパーツ押下時に動画を一時停止させるなど、任意のタイミングで動画を制御することができるので、試してみてください。
  　//余白あれば画像

### 発展編 -youtubeの動画再生-
#### youtubeの動画を再生してみよう
  　シンプルな動画再生機能は実装できたので、次はyoutubeの動画を再生する機能を実装してみましょう。この章では、youtubeの動画再生機能だけでなく、以下のような機能を有するアプリを作ってみます。

  * リポジトリ検索
  * 再生機能
  * ループ再生
  * バックグラウンド再生

  この章は以下の流れで紹介していきます。

  * ライブラリ紹介
  * 必要な設定
  * シンプルな動画再生
  * リポジトリ検索機能実装
  * 両方あわせた実装全体
  * アプリ起動

#### ライブラリ紹介
  　youtubeの動画はストリーミング再生する必要がありますが、実は`YouTube-Player-iOS-Helper`という公式ライブラリが存在します。最終更新が2016年でリリースビルドバージョンが0.1.6となっており、これ大丈夫か？感が漂っていますが、大丈夫です（笑）　また、今回のサンプルコードでは、通信用ライブラリとして`Alamofire`を、パースライブラリとして`SwiftyJSON`を利用しています。これらは、cocoapodsでインストールしておきましょう。

  ```swift:ライブラリリスト
  pod 'Alamofire'
  pod 'youtube-ios-player-helper'
  pod 'SwiftyJSON'
  ```

#### 必要な設定
  　実装へ入る前に`Goole APIs`へアプリの登録をしなければなりません。というのも、ライブラリ検索時に利用する`YouTube Data API`を使うにはトークンが必要になるためです。もしも、ライブラリ検索をせず、特定の動画再生のみできれば良いという方は`Goole APIs`への登録は必要ありません。
  　トークンが手にはいれば、以下のようなフォーマットでライブラリを検索することができます。後ほど利用するので、トークンを覚えておいてください。

  ```
  https://www.googleapis.com/youtube/v3/search?key=【トークン】&q=youtuber&part=snippet&maxResults=3&order=date
  ```
  * シンプルな動画再生
  　では実際に動画再生機能を実装してみます。
  　まずは、肝となるライブラリを`import`しましょう。次に、ストーリーボードのメインView上に新しい`View`を作成し、`Custom class`に`YTPlayerView`を設定します。この`View`がYoutube再生プレイヤー描画領域になります。
  　そして`YTPlayerView`を`ViewController`にアウトレット接続します。あとはこのオブジェクトに動画を`load`させるだけなんですが、そこで必要になってくるのが、`VideoId`です。
  　`VideoId`とはYoutubeの動画再生ページに必ずついているIDです。以下のようなURLフォーマットになっていると思います。ここの`v`パラメーターが`VIdeoId`です。

  ```
  https://www.youtube.com/watch?v=【VideoID】
  ```

  `YouTube Data API`を使うことで取得できる情報には、`VideoId`が含まれています。静的に特定の動画のみを再生したい場合は、ハードコーディングすればOKです。
  　`VideoId`を使った再生は`load`メソッドで実行できます。
  　さらに今回は、自動再生させるためにデリゲートメソッドを実装しておきましょう。`YTPlayerViewDelegate `を採用し、`playerViewDidBecomeReady`メソッドを実装することで、APIの受付が可能になったタイミングで動画が再生されるようになります。（このデリゲートメソッドがない場合、自身でプレイヤーの再生ボタンを押さなくてはいけません。）
  　シンプルな動画再生機能は以上で実装できます。サンプルコードは以下の通りです。

  ```swift:youtube動画再生機能
  import UIKit
  import AVFoundation
  import youtube_ios_player_helper

  class SampleViewController: UIViewController, YTPlayerViewDelegate {

    @IBOutlet weak var playerView: YTPlayerView!
      var videoId = 【任意のVideoId】

      override func viewDidLoad() {
        super.viewDidLoad()

          playerView.delegate = self
          playerView.load(withVideoId: videoId)
      }

    func playerViewDidBecomeReady(_ playerView: YTPlayerView) {
      playerView.playVideo()
    }
  }

```

### おわりに
* 感想
