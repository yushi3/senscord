マルチプロセス構成(Shared Memory)

1. 以下のCMakeオプションでSensCordをビルドする

```
$ cd <senscord-repository>
$ mkdir -p build && cd build
$ cmake .. \
    -DSENSCORD_SERVER=ON \
    -DSENSCORD_API_WASM=ON \
    -DSENSCORD_ALLOCATOR_SHARED_MEMORY=ON \
    -DSENSCORD_SAMPLE=ON
$ make -j4
```

2. 以下のコマンドで SensCordServer プロセスを起動する

```
$ cd <senscord-repository>
$ lib/wamr/example/multi_process_shared/run_server.sh
```

3. 別のターミナルを開き、以下のコマンドで Client (Native) プロセスを起動する

```
$ cd <senscord-repository>
$ lib/wamr/example/multi_process_shared/run_client_native.sh
```

4. 別のターミナルを開き、以下のコマンドで Client (WebAssembly) プロセスを起動する

```
$ cd <senscord-repository>
$ lib/wamr/example/multi_process_shared/run_client_wasm.sh
```
