# 三方库

## 基本介绍
三方库以开源方式实现了各种功能，BUMO 集成并调用了这些库。使用三方库有如下优势：
- 功能丰富
- 代码开源
- 减少重复开发量
- 维护成本低

## 模块结构

名称 | 目录 | 功能
|:--- | --- | ---
| `asio` | [asio](./asio) | 源自 `boost::asio` 模块，提供跨平台的网络底层异步 IO 库。
| `bzip2` | [bzip2-1.0.6](./bzip2-1.0.6) | 实现高效的压缩算法。
| `curl` | [curl](./curl) | 实现 HTTP 等多种协议，以及上传、下载多种场景应用。
| `ed25519` | [ed25519-donna](./ed25519-donna) | 实现了高性能的数字签名算法。
| `gtest` | [gtest](./gtest) | Google 提供的 C++ 单元测试框架，Google Test的简称。
| `http` | [http](./http) | 源自 `boost.asio` 模块，提供 `HTTP` 客户端和服务端基本功能。
| `jsoncpp` | [jsoncpp](./jsoncpp) | 处理 JSON 文本的 C++  库。
| `scrypt` | [libscrypt](./libscrypt) | HASH 算法，时间长，占内存，暴力攻击难。
| `openssl` | [openssl](./openssl) | 提供对称加密算法(AES、DES等)、非对称加密算法(AES、DES等)和摘要算法（MD5、SHA1等）。
| `pcre` | [pcre-8.39](./pcre-8.39) | 提供 C/C++ 语言中使用正则表达式的功能。
| `protobuf` | [protobuf](./protobuf) | Google 公司开发的一种高效的数据描述语言，一般应用于存储，通讯协议等场景。
| `rocksdb` | [rocksdb](./rocksdb) | Facebook 开发的可嵌入，持久型的 key-value 数据库。
| `sqlite` | [sqlite](./sqlite) | 轻量级关系数据库，其消耗资源低，一般嵌入于产品使用。
| `websocketpp` | [websocketpp](./websocketpp) | C++ 的 WebSocket 客户端/服务器库。
| `zlib` | [zlib-1.2.8](./zlib-1.2.8) | 提供数据压缩功能的库，支持 LZ77 的变种算法，DEFLATE的算法。
| `asio` | [asio](./asio) | 源自 `boost::asio` 模块，实现跨平台网络底层异步 IO 功能。
| `v8` | [v8](https://github.com/bumoproject/v8) | Google 开发的高性能 JavaScript 引擎。
