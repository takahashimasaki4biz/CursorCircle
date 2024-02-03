## 前提

Visual Studio 2022 がインストールされていること（コミュニティエディション(無償)で可）
  * https://visualstudio.microsoft.com/ja/vs/community/
  * インストール時、「C++によるデスクトップ開発」のチェックボックスをONにしていること

## ビルド方法

* 本リポジトリからソースコードをcloneするか、ZIPファイルをダウンロード＆展開します。
* `CursorCircle.sln` をダブルクリックし、Visual Studio 2022 で開きます。
* 構成=Release、プラットフォーム=x64 に設定し、ビルドメニューから「ソリューションのリビルド」を実行します。  
  配下の `\x64\Release` ディレクトリに `CursorCircle.exe` が生成されていれば成功です。
