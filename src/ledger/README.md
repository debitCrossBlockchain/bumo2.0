English | [中文](README_CN.md) 

## Introduction
The ledger module is mainly responsible for the execution of the ledger and block generation.It includes the following features:
- Generate the genesis account and genesis block based on configuration
- Execute transactions in the proposal after a consensus is reached
- Generate a new block based on the completed transaction package
- Synchronize blocks from the blockchain network regularly

## Module Structure
Class name | Statement file | Function
|:--- | --- | ---
|`Trie`                  | [trie.h](./trie.h)                                   | Dictionary tree base class. The dictionary tree is the underlying data query and access structure of `BUMO`. In addition to the dictionary features, `BUMO` also adds Merkel root's features to `Trie`. Trie defines the framework functionality of the dictionary tree and implements some of the interfaces.
|`KVTrie`                | [kv_trie.h](./kv_trie.h)                             | The derived class of `Trie` implements the function of the Merkel prefix tree.
|`Environment`           | [environment.h](./environment.h)                     | The execution container of the transaction, which provides transactional features for the transaction. The data that changes during the execution of the transaction will be written to the cache of `Environment`. After all the operations in the transaction have been executed, the update will be submitted uniformly.
|`FeeCalculate`          | [fee_calculate.h](./fee_calculate.h)                 | The cost calculation class defines the fee standard for various transaction operations and provides an external fee calculation interface.
|`AccountFrm`            | [account.h](./account.h)                             | Account class. The user's behavioral body on the `BUMO` chain records all user data including account attributes, account status, and content assets. All operations of the user are based on `AccountFrm`.
|`TransactionFrm`        | [transaction_frm.h](./transaction_frm.h)             | The transaction execution class is responsible for processing and executing transactions, and the specific operations within the transaction are executed by `OperationFrm`.
|`OperationFrm`          | [operation_frm.h](./operation_frm.h)                 | The operation execution class performs the operations in the transaction according to the operation type.
|`ContractManager`       | [contract_manager.h](./contract_manager.h)           |Smart contract management class. Provide code execution environment and management for smart contracts. This includes loading code interpreters, providing built-in variables and interfaces, contract code and parameter checking, code execution, and more. Primarily triggered by the operations of creating account and money transfering of `OperationFrm`.
|`LedgerManager`         | [ledger_manager.h](./ledger_manager.h)               | Ledger management class. Coordinate the execution management of the block, schedule each sub-module under `ledger` to generate a new block, write to database, and synchronize the latest block from the network regularly after executing the transaction in the consensus proposal
|`LedgerContext`         | [ledgercontext_manager.h](./ledgercontext_manager.h) | The execution context of the ledger, which carries the content data and attribute status data of the ledger.
|`LedgerContextManager`  | [ledgercontext_manager.h](./ledgercontext_manager.h) | The management class of `LedgerContext` is convenient for multi-thread execution scheduling.
|`LedgerFrm`             | [ledger_frm.h](./ledger_frm.h)                       | The ledger execution class is responsible for the specific processing of the ledger. The main task is to transfer the transactions in the ledger one by one to `TransactionFrm` to execute.
## 框架流程
- 程序启动时，`LedgerManager` 初始化，并根据配置文件创建创世账户和创世区块。
- 区块链网络开始运行后，`LedgerManager` 接收到经由 `glue` 模块传递过来的共识提案，对提案做合法性检查。
- 通过合法检查后，将共识提案交给 `LedgerContextManager`。
- `LedgerContextManager` 为共识提案的处理生成执行上下文 `LedgerContext` 对象，`LedgerContext` 将提案交由 `LedgerFrm` 具体处理。
- `LedgerFrm` 创建 `Environment` 对象，为执行提案内的交易提供事务容器，然后将提案的交易逐一取出，交由 `TransactionFrm` 处理。
- `TransactionFrm` 再将交易内的操作逐一取出，交由 `OperationFrm` 执行。
- `OperationFrm` 根据类型分别执行交易内的不同操作，并将操作变更的数据写入 `Environment` 的缓存。其中， `OperationFrm` 执行的创建账户操作如果是创建合约账户，或者执行转账操作（包括转移资产和转移BU币），会触发 `ContractManager` 加载并执行合约代码，合约执行对数据的变更也会写入 `Environment` 中。
- 在交易执行过程中，会调用 `FeeCalculate` 计算实际费用。
- 每笔交易内的所有操作完成后，会将 `Environment` 中的变更缓存统一提交更新。
- 等提案内的所有交易执行完成后，`LedgerManager` 对提案打包生成新的区块，并将新区块和更新的数据写入数据库。
- 此外，`LedgerManager` 会通过定时器定时从区块链网络同步最新区块。

