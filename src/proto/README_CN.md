[English](README.md) | 中文

## 模块简介
`BUMO` 的序列化消息定义模块。在区块链程序的内部通信、节点通信和永久化存储等方面，都会用到数据的序列化和反序列化。`BUMO` 采用 `protobuf` 作为序列化工具。在使用时，首先按照 `protobuf` 制定得语法标准定义 `message` 结构，然后调用 `protobuf` 的编译工具，将定义 `message` 结构的文件编译成 `C++` 源文件。然后，`BUMO` 的其他源文件就可以直接调用由 `protobuf` 编译的 `message` 源文件了。


## 模块组成
文件 | 功能
|:--- | ---
[chain.proto](./chain.proto)           | `ledger` 模块以及与 `ledger` 交互的模块用到的可序列化数据结构，包括账户、交易、区块头等相关结构。
[common.proto](./common.proto)         | 常用的基础可序列化数据结构，包括键值对、签名、`ping`、错误码等结构。
[consensus.proto](./consensus.proto)   | 共识和费用相关的可序列化数据结构， 包括 `pbft` 的各类共识消息、验证节点和费用结构。
[merkeltrie.proto](./merkeltrie.proto) | 默克尔前缀树用到的可序列化数据结构，包括节点、子节点和类型等。
[monitor.proto](./monitor.proto)       | 监控模块用到的各类可序列化数据结构，包括注册信息、区块链状态、计算机硬件资源等结构。
[overlay.proto](./overlay.proto)       | `overlay` 模块自用或者经由 `overlay` 通信的模块用到的可序列化数据结构，包括通信状态、`peer` 信息、账本信息等。

## 其他
在 `proto` 目录下还有一些其他的文件夹，比如 `php`、`go`、`python`等，它们都是由 `*.proto` 文件编译生成的不同语言的序列化源文件，以支持不同语言为 `BUMO` 开发的 `SDK`调用。