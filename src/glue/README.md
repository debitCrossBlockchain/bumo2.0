English | [中文](README_CN.md) 

## Introduction
`glue` is the glue module of the `BUMO` blockchain, which is responsible for bonding the important modules together and acting as an intermediary to provide interactive information transferservices for each module. It mainly includes the following functions:
- Connect the `console` module so that users can submit transactions to the `glue` trading pool via the command line
- Connect the `api` module so that users can submit transactions to the `glue` transaction pool via the light client or the `http` tool;
- Connect the `overlay` module so that `glue` can be connected to other modules to call the 'overlay` `p2p` network via `glue` for node communication
- Connect the `consensus` module so that `consensus` can send and receive consensus messages via `glue` which calls `overlay`, or submit the consensus proposal to `ledger` via `glue`
- Connect the `ledger` module so that the consensus proposal for `consensus` can be submitted to `ledger` via `glue` to generate a block; the set of authentication nodes updated by `ledger` or the upgrade information can be submitted to `consensus` to update and take effect; it also enables `ledger` to synchronize blocks via `overlay`

## 模块组成
类名称 | 声明文件 | 功能
|:--- | --- | ---
|`GlueManager`      | [glue_manager.h](./glue_manager.h)            | 胶水管理类，`GlueManager` 提供的接口主要是各模块对外接口的包装，各模块再通过调用 `GlueManager` 提供的包装接口互相通信交互。
|`LedgerUpgradeFrm` | [glue_manager.h](./glue_manager.h)            | 负责 `BUMO` 的账本升级功能，`BUMO` 区块链提供向下兼容性，每个验证节点升级后，会对外广播自己的升级信息，在升级的验证节点达到一定比率之后，所有验证节点按照新版本生成区块，否则按照旧版本生成区块。`LedgerUpgradeFrm` 即负责处理 `BUMO` 升级的各项流程。
|`TransactionQueue` | [transaction_queue.h](./transaction_queue.h)  | 交易池。将用户提交的交易放入交易缓存队列中，并按照账户 `nonce` 值和 `gas_price` 对交易双重排序，供 `GlueManager` 打包共识提案。
