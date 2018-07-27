English | [中文](README_CN.md) 

## 模块简介
`glue` 是 `BUMO` 区块链的胶水模块，负责将各重要模块黏合在一起，并充当中间媒介为各模块提供信息中转交互服务。主要包括：
- 连接 `console` 模块，使用户可以通过命令行提交交易到 `glue` 的交易池；
- 连接 `api` 模块，使用户可以通过轻客户端或者 `http` 工具提交交易到 `glue` 的交易池；
- 连接 `overlay` 模块，使 `glue` 连接得其他模块可以经由 `glue` 调用 `overlay` 的 `p2p` 网络进行节点通信；
- 连接 `consensus` 模块，使 `consensus` 可以经由 `glue` 调用 `overlay` 发送和接收共识消息，或者经由 `glue` 将共识提案提交给 `ledger` 执行；
- 连接 `ledger` 模块，使 `consensus` 的共识提案可以经由 `glue` 提交给 `ledger` 执行生成区块，将 `ledger` 更新的验证节点集合或者升级信息提交给 `consensus` 更新生效，并且可以使 `ledger` 可以通过 `overlay` 同步区块；

## 模块组成
类名称 | 声明文件 | 功能
|:--- | --- | ---
|`GlueManager`      | [glue_manager.h](./glue_manager.h)            | 胶水管理类，`GlueManager` 提供的接口主要是各模块对外接口的包装，各模块再通过调用 `GlueManager` 提供的包装接口互相通信交互。
|`LedgerUpgradeFrm` | [glue_manager.h](./glue_manager.h)            | 负责 `BUMO` 的账本升级功能，`BUMO` 区块链提供向下兼容性，每个验证节点升级后，会对外广播自己的升级信息，在升级的验证节点达到一定比率之后，所有验证节点按照新版本生成区块，否则按照旧版本生成区块。`LedgerUpgradeFrm` 即负责处理 `BUMO` 升级的各项流程。
|`TransactionQueue` | [transaction_queue.h](./transaction_queue.h)  | 交易池。将用户提交的交易放入交易缓存队列中，并按照账户 `nonce` 值和 `gas_price` 对交易双重排序，供 `GlueManager` 打包共识提案。
