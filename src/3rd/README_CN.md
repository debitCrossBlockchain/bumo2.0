# 三方库

## 基本介绍
三方库提供了丰富的功能，让开发者基于其提供的 API 接口快速开发各种应用。使用三方库有如下目的：
- 功能丰富
- 代码开源
- 减少重复开发量
- 维护成本低

## 模块结构

名称 | 目录 | 功能
|:--- | --- | ---
| `asio` | [asio](./asio) | 源自 `boost::asio` 模块，提供了跨平台的网络底层异步 IO 库。
| `bzip2` | [bzip2-1.0.6](./bzip2-1.0.6) | 实现了高效的压缩算法。
| `curl` | [curl](./curl) | 实现了 HTTP 等多种协议，并实现了上传、下载多种场景的应用。
| `ed25519` | [ed25519-donna](./ed25519-donna) | 实现了高性能的数字签名算法。
| `gtest` | [gtest](./gtest) | Google 提供的 C++ 单元测试框架，Google Test的简称。
| `http` | [http](./http) | 源自 `boost.asio` 模块，提供了 `HTTP` 客户端和服务端基本功能。
| `jsoncpp` | [jsoncpp](./jsoncpp) | 处理 JSON 文本的 C++  库。
| `scrypt` | [libscrypt](./libscrypt) | HASH 算法，时间长，占内存，暴力攻击难。
| `openssl` | [openssl](./openssl) | 提供了多种对称，非对称加密，摘要算法。如AES、DES等对称加密；RSA，EC等非对称加密；MD5、SHA1摘要算法
| `pcre` | [pcre-8.39](./pcre-8.39) | 提供 C/C++ 语言中使用正则表达式的功能。
| `protobuf` | [protobuf](./protobuf) | Google 公司开发的一种高效的数据描述语言，用于存储，通讯协议等方面。
| `rocksdb` | [rocksdb](./rocksdb) | Facebook 开发的可嵌入的，持久型的 key-value 数据库。
| `sqlite` | [sqlite](./sqlite) | 轻量级的关系数据库，消耗资源低，一般嵌入于产品使用。
| `websocketpp` | [websocketpp](./websocketpp) | C++ 的 WebSocket 客户端/服务器库。
| `zlib` | [zlib-1.2.8](./zlib-1.2.8) | 提供数据压缩功能的库，支持 LZ77 的变种算法，DEFLATE的算法。
| `asio` | [asio](./asio) | 源自 `boost::asio` 模块，提供了跨平台的网络底层异步 IO 库。
| `v8` | [v8](https://github.com/bumoproject/v8) | Google 开发的高性能 JavaScript 引擎。
