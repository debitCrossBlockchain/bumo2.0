[English](README.md) | 中文

# Common

## 基本介绍
实现 C++ 工程里常用的操作，脱离于业务层，属于通用模块。功能如下：
- 参数解析。
- 配置文件解析。
- 守护进程辅助。
- 网络层封装。
- 其他功能。如定时器，状态器，proto 转 json，私钥生成，数据库等

## 模块结构
类名称 | 声明文件 | 功能
|:--- | --- | ---
| `Argument` | [argument.h](./argument.h) | 用于解析 `main` 函数的参数。实现签名、创建账号、管理 KeyStore、加解密、字节转换等功能。
| `ConfigureBase` | [configure_base.h](./configure_base.h) | 解析配置文件的基本类，提供加载和获取值的基本操作。头文件同时实现三个子配置加载类：`LoggerConfigure` 日志配置，`DbConfigure` 数据库配置，`SSLConfigure` SSL 配置。
| `Daemon` | [daemon.h](./daemon.h) | 守护进程辅助工具，向共享内存内写最新的时间戳，供守护程序监控。
| `General` | [general.h](./general.h) | 定义工程通用的全局静态变量；提供小的工具类：如`Result` 、 `TimerNotify` 、`StatusModule` 、`SlowTimer` 、`Global` 、`HashWrapper` 。
| `KeyStore` | [key_store.h](./key_store.h) | 实现创建和解析 KeyStore 的功能。
| `Network` | [network.h](./network.h) | 实现节点网络通信功能。使用 `asio::io_service` 做为异步 IO ，同时管理所有网络连接，如新建，关闭，保持心跳等，并负责分发和解析接收到的消息。其中 `Connection` 类是单个网络连接的封装者，使用 `websocketpp::server` 和 `websocketpp::client` 做为管理对象，实现发送数据、获取 TCP 状态等功能。
| `Json2Proto`、`Proto2Json` 函数| [pb2json.h](./pb2json.h) | 用于 Google Proto buffer 和 JSON 之间数据转换。
| `PublicKey`、`PrivateKey` | [private_key.h](./private_key.h) | `PublicKey` 是公钥数据转换和验签签名数据的工具类。`PrivateKey` 是私钥数据转换和签名数据的工具类。
| `Storage` | [storage.h](./storage.h) | `Storage` 是 key vaule 数据库的管理类。在头文件中还定义数据库操作的接口类`KeyValueDb`，并派生出两个子类 `LevelDbDriver` 和 `RocksDbDriver`，分别用于操作 LevelDb 和 RocksDB