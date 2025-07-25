[準備]
1. senscord(C/C++)を以下のオプションをONにしてビルドします
  - SENSCORD_API_CSHARP
  - SENSCORD_SAMPLE  ※

  ※ C# API には直接関係ありませんが[Sample実行手順]を行う場合に必要になります


--------------------------------------------------------------------------------
 Windows, Visual Studio
--------------------------------------------------------------------------------
[C# APIビルド手順]
1. Visual Studio 2017 で csharp/senscord_csharp.sln を開きます

2. simple_stream プロジェクトを選択してビルドします


[Sample実行手順]
1. 以下の環境変数を設定します

  PATH=${PATH};[senscord.dllとsenscord_osal.dllが存在するディレクトリ]

  SENSCORD_FILE_PATH=[senscordのコンフィグ(.xml)が存在するディレクトリ];[senscordでビルドしたcomponentが存在するディレクトリ];[senscordでビルドしたallocatorが存在するディレクトリ];[senscordでビルドしたrecorderが存在するディレクトリ]

  ※ 区切り文字はセミコロン ';' です
  ※ sample/simple_stream/Program.cs : L.26付近のコードを有効にして
     プログラム上から環境変数を設定することもできます


2. simple_stream プロジェクトを実行します


--------------------------------------------------------------------------------
 Linux, MonoDevelop
--------------------------------------------------------------------------------
[C# APIビルド手順]
1. MonoDevelop で csharp/senscord_csharp.sln を開きます

2. simple_stream プロジェクトを選択してビルドします


[Sample実行手順]
1. 以下の環境変数を設定します

  SENSCORD_FILE_PATH=[senscordのコンフィグ(.xml)が存在するディレクトリ]:[senscordでビルドしたcomponentが存在するディレクトリ]:[senscordでビルドしたallocatorが存在するディレクトリ]:[senscordでビルドしたrecorderが存在するディレクトリ]

  ※ 区切り文字はコロン ':' です
  ※ sample/simple_stream/Program.cs : L.26付近のコードを有効にして
     プログラム上から環境変数を設定することもできます


2. senscord_csharp.dll.config を作成して senscord_csharp.dll と同じディレクトリに配置します

  senscord_csharp.dll.config の内容
  ----
  <configuration>
    <dllmap dll="senscord" target="[libsenscord.soのフルパス]" />
    <dllmap dll="senscord_osal" target="[libsenscord_osal.soのフルパス]" />
  </configuration>


3. simple_stream プロジェクトを実行します

