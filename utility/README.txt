# 1. Utilityディレクトリ概要
ここsensocrd/utilityディレクトリにはSensCord Coreから独立した
ユーティリティ ライブラリやツールが配置されます。

以下のディレクトリが配置されています。
- synchronizer
  - TimeStamp同期機能のディレクトリです。
  - SYNCHRONIZER_SAMPLE_ENABLE=ONでビルドが有効になります。

目次
<!-- TOC -->

- [1. Utilityディレクトリ概要](#1-utilityディレクトリ概要)
- [2. TimeStamp同期](#2-timestamp同期)
    - [2.1. 概要](#21-概要)
    - [2.2. 動作環境](#22-動作環境)
    - [2.3. ビルド手順](#23-ビルド手順)
    - [2.4. ビルドオプション](#24-ビルドオプション)

<!-- /TOC -->

-------------------------------------------------------------------
<!-- ---------------------- TimeStamp同期 --------------------- -->

# 2. TimeStamp同期
## 2.1. 概要
SensCordから取得した複数のFrameを、TimeStampを基に同期してまとめる機能を提供します。

- 主に以下のコンテンツがあります。
  - synchronizer
    - TimeStamp同期のライブラリ(synchronizer)です。
    - SYNCHRONIZER_ENABLE=ONでビルドが有効になります。
    - make & make installすると次のファイルが生成されます。
      - header: include/senscord/synchronizer/*.h
      - Linux binaly: lib/senscord/utility/libsynchronizer.so 
      - Windows binaly: bin\utility\synchronizer.dll

  - SyncPolicyMasterSlave
    - TimeStamp同期の「同期方式」の実装です。
    - 同期の基準とするStream(Master)を定め、そのFrameのTimeStampを基に他のStream(Slave)のFrameを同期します。
    - この同期方式はsynchronizerライブラリに内蔵されます。

  - SynchronizerSample
    - TimeStamp同期のサンプル・デモプログラムです。
    - Polling版とCallback版があります。
    - SYNCHRONIZER_SAMPLE_ENABLE=ONでビルドが有効になります。
    - make & make installすると次のファイルが生成されます。
      - binaly: 
        - bin/utility/SynchronizerSamplePolling
        - bin/utility/SynchronizerSampleCallback


## 2.2. 動作環境
- ターゲットのプラットフォームはLinux PC/Windows PC/VP1です。
- ビルド環境等もSensCordのREADME.txtをご覧ください。


## 2.3. ビルド手順
CMakeオプションSYNCHRONIZER_ENABLE=ONでビルドします。
具体的なビルド手順についてはsenscord/README.txtをご覧ください。


## 2.4. ビルドオプション
Synchronizer固有のCMakeオプションについて説明します。

- SYNCHRONIZER_ENABLE=**ON or OFF**
  - SynchronizerのビルドのON/OFFを設定します。

- SYNCHRONIZER_SAMPLE_ENABLE=**ON or OFF**
  - SynchronizerSampleのビルドのON/OFFを設定します。

-------------------------------------------------------------------

