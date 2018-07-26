# Common

## 基本介绍
Common 模块封装实现了工程里常用的操作，基本功能如下：
- 参数解析。
- 配置文件解析。
- 守护进程辅助。
- 网络层封装。
- 其他基本功能。如定时器，状态器，proto 转 json，私钥生成，数据库等

## 模块结构
类名称 | 声明文件 | 功能
|:--- | --- | ---
| `Argument` | [argument.h](./argument.h) | 用于解析 `main` 函数的参数。实现了签名，创建账号，管理KeyStore，加解密，字节转换等功能。
| `ConfigureBase` | [configure_base.h](./configure_base.h) | 解析配置文件的基本类，提供了加载和获取值得基本操作。`LoggerConfigure` 类实现了日志配置加载功能，`DbConfigure` 类实现了数据库配置加载功能，`SSLConfigure` 类实现了 SSL 配置加载功能。
| `Daemon` | [daemon.h](./daemon.h) | 守护进程辅助工具，向共享内存内写最新的时间戳，供守护程序监控。
| `General` | [general.h](./general.h) | 定义了工程通用的全局静态变量；提供了普通的小工具或者小函数。如`Result` 类， `TimerNotify` 类，`StatusModule` 类，`SlowTimer` 类，`Global` 类，`HashWrapper` 类。
| `KeyStore` | [key_store.h](./key_store.h) | 实现创建和解析 KeyStore 的功能。
| `Network` | [network.h](./network.h) | 节点网络通信的实现者。使用 `asio::io_service` 异步 IO 监听网络事件，管理所有的网络连接，如新建连接，关闭连接，保持连接心跳，分发解析接收到的消息。`Connection` 类是单个网络连接的封装者，提供发送数据的基本接口，提供当前 TCP 的状态，使用 `websocketpp::server` 和 `websocketpp::client` 做为管理对象。
| `Json2Proto`、`Proto2Json` 函数| [pb2json.h](./pb2json.h) | Google Proto buffer 数据和 JSON 数据之间的相互转换。
| `PublicKey`、`PrivateKey` | [private_key.h](./private_key.h) | `PublicKey` 是公钥数据转换和验签签名数据的工具类。`PrivateKey` 是私钥数据转换和签名数据的工具类。
| `Storage` | [storage.h](./storage.h) | Key vaule 数据库的管理类。`LevelDbDriver` 实现了对 LevelDb 数据库操作的功能。`RocksDbDriver` 实现了对 RocksDb 数据库操作的功能。`KeyValueDb` 数据库操作的接口类。