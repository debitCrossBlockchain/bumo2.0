# Third Party Library

## Introduction
The third party library implements various functions in an open source manner, and BUMO integrates and calls these libraries. Using a three-party library has the following advantages:
- Rich functions
- Open source code
- Reduce repeated development
- Low cost to maintain

## Module Structure

Name | Directory | Function
|:--- | --- | ---
| `asio` | [asio](./asio) | Originated from the `boost::asio` module, it provides a cross-platform network underlying asynchronous IO library.
| `bzip2` | [bzip2-1.0.6](./bzip2-1.0.6) | It implements efficient compression algorithms.
| `curl` | [curl](./curl) | It implements a variety of protocols such as HTTP, and supports uploading and downloading multiple applications.
| `ed25519` | [ed25519-donna](./ed25519-donna) | It implements a high performance digital signature algorithm.
| `gtest` | [gtest](./gtest) | Google's C++ unit testing framework, abbreviated for Google Test.
| `http` | [http](./http) | Originated from the `boost.asio` module, it provides the `HTTP` client and server basic functionality.
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
