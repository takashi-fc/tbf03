
= GoogleAppsScriptライブラリ開発


みなさんGoogleAppsScript@<fn>{1}（以下、GAS）は使ったことがありますか？
GASは簡単に言うと、Googleのサーバ上でJavaScriptを実行できるサービスです。
外部にAPIとして公開しPOST,GETのリクエストを受けたり、Google Spreadsheets@<fn>{2}（以下、Spreadsheets）やGoogle FusionTables@<fn>{3}（以下、FusionTables）でデータを保持することもできます。
個人のアプリでバックエンドにGASを利用していて、DBにSpreadsheetsを利用し、ライブラリを開発した際に得られた知見と成果の内容です。


== きっかけとGoogleAppsScriptから利用するDBの選択


結論から言うと、FusionTablesよりSpreadsheetsを利用することをオススメします。
FusionTablesのメリットはGoogleMapとの連携やチャート化など@<strong>{データをビジュアル化しやすい}というところです。良さそうなのになぜ使わないのか、その理由としては、FusionTablesがSELECT句以外でWHERE句が利用できないことが大きいです。
正確に言うと、ROW@<b>{IDというオートインクリメントされるカラムしか指定できません。
削除したい対象をSELECTで抽出してから、ROW}IDを指定してDELETEすることは可能です。
しかし、書き込みの上限が30件/分かつ5000件/日と厳しく、リクエストを送るクライアントが多いと現実的ではありません。



Spreadsheetsメリットとしては@<strong>{非エンジニアでも触れる}だと思います。例えば、

 * ボタンやプッシュ通知のA/Bパターン文言
 * 動的なテンプレートメッセージ



などを文言やパターンを変更する際に、エンジニアを介さず変更を行うことができるます。
そんな気軽に変更されてはたまらん！というデータはシートを分けておき、編集可能者を制限しておくと良いでしょう。
それぞれに@<strong>{容易に変更できる環境でありながら実行権限を付与できる}こともメリットです。



良いところしかないのかと言うとそうではありません。GASでSpreadsheetsを利用する際に使うSpreadsheet Service@<fn>{4}が提供しているメソッドでは、SQLを利用してデータを抽出することができません。
そこで SpreadSheetsSQL@<fn>{5} というSpreadsheetsからSQLライクにデータを抽出できるGASのライブラリを公開しています。ライブラリを使うメリットとしては@<strong>{Spreadsheetsのセルを意識する必要がなくなる}というところにあります。
もちろん、このライブラリなしでもDBとして利用することは可能です。
常に固定のセルを取得するだけなら、直にSpreadsheet Serviceを利用した方が良いと思います。


== GASライブラリの開発


GASライブラリはGASで作ることができます。SpreadsheetsSQLのソースコードを例に説明します。
まず初めにこのライブラリを使う時は以下のように使います。


//emlist[][javascript:サンプルコード.gs]{
var result = SpreadSheetsSQL.open(SHEET_ID, SHEET_NAME).select(['name', 'cv']).result();

// resultの内容
[{
  name: 'サーバルちゃん',
  cv: '尾崎由香'
},{
  name: 'かばんちゃん',
  cv: '内田彩'
}]
//}


SpreadsheetsSQLを利用する際には、@<tt>{SpreadSheetsSQL.open()}を呼ぶ必要があります。
これは@<tt>{ライブラリのプロジェクト名.メソッド}という形になっていて


//emlist[][javascript:SpreadSheetsSQL.gs]{
/**
 * This method use to create SpreadSheetsSQL instance.
 * @param {String} id SpreadSheet id
 * @param {String} name SpreadSheet sheet name
 * @return {SpreadSheetsSQL} SpreadSheetsSQL instance
 */
function open(id, name) {
  return new SpreadSheetsSQL_(id, name);
}
//}


コードではこのようになっています。コメントはJSDoc@<fn>{6}で記述することができます。
@<tt>{new SpreadSheetsSQL_(id, name)}で@<tt>{SpreadSheetsSQL_}というクラスをインスタンス化していることがわかると思います。
また、クラス名の末尾にアンダースコアがついていることにお気付きかと思います。これはスコープ制御をするためであり@<strong>{ライブラリ内でのみ参照できるプライベートな状態}になります。
@<tt>{SpreadSheetsSQL.open()}した際の戻り値は@<tt>{SpreadSheetsSQL_}型ですが、JSDocにはアンダースコアなしで記述されています。
実際にはプライベートクラスのインスタンスが返っているが、JSDocを見るとプロジェクト名のクラスが返るということです。



次に取得するカラムの指定に@<tt>{SpreadSheetsSQL.open().select()}を呼ぶ必要があります。


//emlist[][javascript:SpreadSheetsSQL.gs]{
/**
 * This method use to get columns.
 * <pre><code>Example: SpreadSheetsSQL.open(id, name).select(['name', 'age', 'married', 'company']).result();
 * </pre></code>
 * @param {String[]} selects column array
 * @return {SpreadSheetsSQL} SpreadSheetsSQL instance
 */
function select(selects) {
  throw new Error("it's a mock method for content assist");
}
//}


コードではこのようになっています。@<tt>{result()}でも同様のコードが記述されています。
呼び出されたら例外を投げるメソッドに見えます。しかし、ここで宣言されているメソッドはあくまで@<strong>{ライブラリ外から参照するためのメソッド}でしかありません。実態は


//emlist[][javascript:SpreadSheetsSQL.gs]{
class SpreadSheetsSQL_ {
  ...
  select(selects) {...}
  ...
}
//}


のように@<tt>{SpreadSheetsSQL_}クラス内に記述されています。通常ではプライベートなクラスのメソッドは外からは見えず呼び出すことができません。メソッドが見えないということは呼び出すことができないということになっていまします。
そのため、ライブラリのライブラリのルート階層にインターフェースとして記述し、プライベートなメソッドでもGASのオンラインエディタでの補完を有効にしています。



まとめるとこのようになります。

 * プロジェクト名.メソッドという形で呼び出すことができる。
 * 末尾にアンダースコアでライブラリ外からは見えないようにする。
 * 補完のためのインターフェースをルート階層に列挙し、JSDocの@returnにはライブラリ名を指定する。


== GASライブラリの公開


開発もそこまで気をつけることはありませんでしたが、公開はもっと簡単です。

 1. GASのライブラリ化したいプロジェクトを開く。
 1. 「ファイル」->「版を管理」に移動して、新しいバージョンを保存する。
 1. 「ファイル」->「プロジェクトのプロパティ」で表示される、スクリプトIDを共有する。



たったこれだけで、他人がでライブラリとしてGASプロジェクトを扱うことができます。
利用できるとは言ってもドキュメントが必要ですよね？実はJSDocを書いていれば、https://script.google.com/macros/library/d/スクリプトID/バージョン で見ることができます。公式がきちんとドキュメントまでサポート@<fn>{7}しているのはとても嬉しいことですね。


== おまけ: ES6 + ローカルで開発する


GASは個人でサーバを用意せず、気軽に開発できて便利です。
ただ、オンラインエディタ上でしか書けないため、普段使いしている慣れた環境で開発することができません。
そこでBabel + gulp + node-google-apps-script を使うことで、ES6かつローカルで開発しています。



ローカルのファイルをアップロードするための選択肢としていくつか方法がありますが、今回はGoogleDevelopersJapanで過去に紹介された node-google-apps-script@<fn>{8} を利用します。
セットアップはURL先に譲ります。



また、GASはES6に対応していないため、ES6で記述したJavaScriptをそのまま実行することはできません。
そのため https://babeljs.io を利用して、実行可能なJavaScriptのコードに変換します。
単純にBabelを利用して変換しても良いですが、コードを変更したら自動で変換+同期できるように、gulpを利用します。

 1. node-google-apps-scriptのセットアップ
 1. gulp + babelのインストール @<tt>{npm install -g gulp && npm install --save-dev gulp gulp-babel babel-preset-es2015}
 1. es6というディレクトリを作って、@<tt>{コード.js}を作成
 1. gulpfile.jsを作成



これで準備完了です。
@<tt>{gulp babel}と実行すると、src/コード.jsに変換されたJavaScriptが出力され、@<tt>{gapps upload}することでコード.jsが反映されます。
@<tt>{gulp}と実行すると、変更監視状態になりes6のコードを変更したら、Babelで変換し、アップロードまでを自動的に行います。


//emlist[][ディレクトリ構成]{
.
├── gapps.config.json
├── gulpfile.js
├── node_modules
├── es6
│   └── コード.js
└── src
    └── コード.js
//}

//emlist[][javascript:gulpfile.js]{
var gulp = require('gulp');
var babel = require('gulp-babel');
var spawn = require('child_process').spawn;

gulp.task('default', ['watch', 'upload']);

gulp.task('watch', () => {
  gulp.watch('es6/*.js', ['babel']);
});

gulp.task('babel', () => {
  return gulp.src('es6/*.js')
    .pipe(babel({
            presets: ['es2015']
        }))
    .pipe(gulp.dest('src'));
});

gulp.task('upload', () => {
  gulp.watch('src/*.js', function() {
    spawn('gapps', ['upload']);
    console.log('finished `gapps upload`');
  });
});
//}

//emlist[][javascript:es6のコード.js]{
class Bot {
  constructor(message) {
    this.message = message;
  }

  say() {
    Logger.log(this.message);
  }
}

function main() {
  var bot = new Bot("ようこそ技術書典へ");
  bot.say();
}
//}


これでローカルかつES6で開発しながら、GASに実行可能な状態で同期することができました。


== さいごに


どうでしたか？GoogleAppsScriptが大きく目立つことがない中、更にそのライブラリを作るというニッチなところまとめてみました。GoogleAppsScriptはAPI+DBとしてだけではなく、cronのように定期的に処理を実行することも可能です。また、個人ではなかなかサーバ側に手が出せないという人や、ちょっとした便利ツールを作る時にとても便利だと思っています。


//footnote[1][https://developers.google.com/apps-script]

//footnote[2][https://docs.google.com/spreadsheets]

//footnote[3][https://developers.google.com/fusiontables]

//footnote[4][https://developers.google.com/apps-script/reference/spreadsheet/]

//footnote[5][https://github.com/roana0229/spreadsheets-sql]

//footnote[6][http://usejsdoc.org]

//footnote[7][執筆時点2017/04/16時点ではCSSが読み込めない状態になっています。]

//footnote[8][https://github.com/danthareja/node-google-apps-script]
