[English](README.md) | 中文

# API 接口模块

## 功能介绍
该模块实现多样化的接口服务功能，具体如下：
- HTTP 服务。该服务有两种方式：一是 API 接口，通过这些接口可以与节点区块链账本交互，如查询账号信息，查询交易，提交交易，调用合约等；二是提供访问 HTTP 页面的功能。
- WebSocket 接口服务。提供发起交易、订阅交易的功能。
- Console 接口。提供用户使用控制台方式进行操作 Wallet，获取 BU 值，支付 BU 等操作。

## 模块结构

类名称 | 声明文件 | 功能
|:--- | --- | ---
| `WebServer` | [web_server.h](./web_server.h) | HTTP 服务的提供者。使用 `http::server::server`提供HTTP 服务(参考[server.hpp](../3rd/http/server.hpp))，在该 `WebServer` 类中实现路由功能，用于调用 HTTP 接口和访问 HTTP 页面。
| `WebSocketServer` | [websocket_server.h](./websocket_server.h) | Web Socket 服务的提供者。该类继承自 `Network` 类(参考[network.h](../common/network.h))，`Network` 使用 `asio::io_service` 异步 IO 监听网络事件，并管理所有的网络连接。`WebSocketServer` 的功能如下：对外部节点提供发起交易和交易订阅服务；对内其他模块提供广播交易的接口，用于把相关交易通知给订阅者。
| `Console` | [console.h](./console.h) | 控制台命令服务的提供者。拥有独立的线程执行环境，启动后监听输入操作，并按照指令执行操作，执行过程中会调用到其他模块如 `KeyStore` (参考 [key_store.h](../common/key_store.h))，`GlueManager` (参考 [glue_manager.h](../glue/glue_manager.h)) 等

## 接口列表

代码中实现的接口包括但不限于如下
### HTTP
```
createKeyPair   #创建公私钥对，仅用于调试使用，不推荐在生产环境使用
getAccount  #获取账号信息
getAccountBase  #获取账号基本信息
getGenesisAccount   #获取创世账号
getAccountMetaData  #获取账号的 metadata 值
getAccountAssets    #获取账号的资产
getTransactionBlob  #转换交易字符串为二进制值格式
getTransactionHistory   #获取历史交易记录
getTransactionCache     #获取交易池缓存记录
getStatus   #获取交易数和账号数
getLedger   #获取区块信息
getModulesStatus    #获取模块信息
getConsensusInfo    #获取共识消息
updateLogLevel      #设置日志级别
getAddress          #获取地址信息
getTransactionFromBlob  #从二进制对象中获取交易信息
getPeerAddresses    #获取节点信息
getLedgerValidators #获取验证节点信息
submitTransaction   #发起交易
callContract        #调用合约
testTransaction     #检测交易
```
### WebSocket
```
protocol::CHAIN_SUBMITTRANSACTION  #发起交易
protocol::CHAIN_SUBSCRIBE_TX    #订阅交易
```
协议定义文件 [overlay.proto](../proto/overlay.proto)

### Console
```
createWallet     #创建钱包
openWallet      #打开钱包
closeWallet     #关闭钱包
restoreWallet   #重置钱包
getBalance      #获取 BU 余额
getBlockNumber  #获取最后的区块信息
help            #帮助
payCoin     #支付 bu coin
showKey     #展示钱包私钥
getState    #获取节点状态
exit   #退出控制台
```

