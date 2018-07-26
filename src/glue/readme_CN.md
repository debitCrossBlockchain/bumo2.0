## 模块简介
glue是BUMO区块链的胶水模块，负责将各重要模块黏合在一起，并充当中间媒介为各模块提供信息中转交互服务。主要包括：
- 连接console和ledger模块，使用户可以通过命令行提交交易；
- 连接api和ledger模块，使用户可以通过轻客户端或者http工具提交交易；
- 连接overlay和ledger模块，使节点之间可以通过overlay的p2p网络进行区块同步；
- 连接consensus和ledger模块，将consensus达成的共识提案提交ledger模块执行生成区块，将ledger模块更新的验证节点集合或者升级信息提交给consensus更新生效。
- 连接overlay和consensus模块，使consensus模块通过overlay的p2p网络发送和接收共识消息；

## 模块组成
类名称 | 声明文件 | 功能
|:--- | --- | ---
|`GlueManager`      | [glue_manager.h](./glue_manager.h)            | 胶水管理类，GlueManager提供的接口主要是对各模块对外接口的包装，各模块再通过调用GlueManager提供的包装接口互相通信交互。
|`LedgerUpgradeFrm` | [glue_manager.h](./glue_manager.h)            | 负责BUMO的账本升级功能，BUMO区块链提供向下兼容性，每个验证节点升级后，会对外广播自己的升级信息，在升级的验证节点达到一定比率之后，所有验证节点按照新版本生成区块，否则按照旧版本生成区块。LedgerUpgradeFrm即负责处理BUMO升级的各项流程。
|`TransactionQueue` | [transaction_queue.h](./transaction_queue.h)  | 交易池。将用户提交的交易放入交易缓存队列中，并按照账户nonce值和gas_price双重排序，供GlueManager打包共识提案。
