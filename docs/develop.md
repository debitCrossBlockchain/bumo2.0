# BUMO 区块链开发文档

<!-- TOC -->

- [BUMO区块链开发文档](#BUMO区块链开发文档)
    - [基础知识](#基础知识)
        - [了解protocol buffer3](#了解protocol-buffer3)
        - [protocol buffer 3和json](#protocol-buffer-3和json)
        - [websocket和http](#websocket和http)
        - [端口配置](#端口配置)
        - [交易执行的基本过程](#交易执行的基本过程)
        - [试一试](#试一试)
    - [HTTP接口](#http接口)
        - [查询账号](#查询账号)
        - [查询交易](#查询交易)
        - [查询区块头](#查询区块头)
        - [提交交易](#提交交易)
        - [序列化交易](#序列化交易)
        - [调试合约](#调试合约)
        - [评估费用](#评估费用)
    - [定义交易](#定义交易)
        - [交易的基本结构](#交易的基本结构)
        - [操作](#操作)
            - [操作码](#操作码)
            - [创建账号](#创建账号)
            - [发行资产](#发行资产)
            - [转移资产](#转移资产)
            - [设置metadata](#设置metadata)
            - [设置权重](#设置权重)
            - [设置门限](#设置门限)
            - [转移bu资产](#转移bu资产)
    - [高级功能](#高级功能)
        - [控制权的分配](#控制权的分配)
        - [版本化控制](#版本化控制)
        - [表达式](#表达式)
        - [合约](#合约)
            - [内置函数](#内置函数)
            - [内置变量](#内置变量)
            - [异常处理](#异常处理)
        - [验证者节点选举](#验证者节点选举)
            - [创建选举合约账户](#创建选举合约账户)
            - [申请成为验证节点候选人](#申请成为验证节点候选人)
            - [对验证节点候选人申请者投票](#对验证节点候选人申请者投票)
            - [收回押金](#收回押金)
            - [废止恶意节点](#废止恶意节点)
            - [取消废止恶意节点](#取消废止恶意节点)
            - [对废止恶意节点投票](#对废止恶意节点投票)
            - [查询功能](#查询功能)
        - [费用选举合约](#费用选举合约)
            - [费用结构](#费用结构)
            - [费用种类](#费用种类)
            - [查询历史费用](#查询历史费用)
            - [查询费用提案](#查询费用提案)
            - [费用提案](#费用提案)
            - [费用选取](#费用选取)
    - [错误码](#错误码)
    - [示例](#示例)

<!-- /TOC -->

## 基础知识

### 了解protocol buffer3

BUMO区块链是用protocol buffer 3序列化数据的，protocol buffer 3是google推出的数据序列化协议，您如果不了解protocol buffer 3，请点击[https://developers.google.com/protocol-buffers/docs/proto3](https://developers.google.com/protocol-buffers/docs/proto3)了解更多。
我们使用的所有数据格式都能在源码的```src\proto```目录中找到。其中chain.proto文件中定义的数据是和交易、区块、账号密切相关的。 

### protocol buffer 3和json

http接口中的数据都是json格式的，这些格式都是由protocolbuffer自动转化的。由于json中无法直接使用不可见字符, 凡是protocolbuffer结构定义中为**bytes**的，在json中都是16进制格式。

### websocket和http

BUMO 区块链提供了websocket和http 两种API接口。您可以在 安装目录/config/bumo.json 文件种找到`"webserver"`和`"wsserver"`两个对象,它们指定了http服务端口和websocket服务端口。

```json
    "webserver":
    {
        "listen_addresses": "0.0.0.0:36002"
    },
    "wsserver":
    {
        "listen_address": "0.0.0.0:36003"
    }
```

### 端口配置

| 网络类型        | 网络ID（network_id）|WebServer |P2P  | WebSocket |
| :------------- | -------|--------- |-----|-----------|
| mainnet        | 10000|16002 |16001 |16003
| testnet | 20000|26002 | 26001 | 26003
| 内测版本 | 30000 | 36002 | 36001 | 36003 


### 交易执行的基本过程

1. 根据意愿组装交易对象`Transaction`
1. 交易对象序列化(protocol buffer 3格式)为字节流 `transaction_blob`
1. 用私钥`skey`对`transaction_blob`签名得到`sign_data`，`skey`的公钥为`pkey`
1. 提交交易，见[提交交易](#提交交易)
1. 查询以确定交易是否成功或接收推送（websocket API）判断交易是否成功

### 试一试

如果您的区块链刚刚部署完成，那么目前区块链系统中只有创世账号。您可以通过http接口查询创世账号
`HTTP GET host:36002/getGenesisAccount`
您会得到类似这样的返回内容

```json
{
   "error_code" : 0,
   "result" : {
      "address" : "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3",
      "assets" : null,
      "balance" : 100000000000000000,
      "metadatas" : null,
      "priv" : {
         "master_weight" : 1,
         "thresholds" : {
            "tx_threshold" : 1
         }
      }
   }
}
```

返回结果中的`address`的值就是创世账号。
您还可以通过[查询账号](#查询账号)接口查询任意账号

```text
HTTP GET host:29333/getAccount?address=buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3
```

## HTTP接口

### 查询账号

```text
HTTP GET /getAccount?address=buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3&key=hello&code=xxx&issuer=xxx
```

功能:返回指定账号的信息及其所有资产、metadata

| 参数         | 描述                                                                                                                                                    |
| :----------- | ------------------------------------------------------------------------------------------------------------------------------------------------------- |
| address      | 账号地址， 必填                                                                                                                                         |
| key          | 账号的 metadata 中指定的key的值，如果不填写，那么返回结果中含有所有的metadata                                                                           |
| code, issuer | 资产代码,资产发行商。这两个变量要么同时填写，要么同时不填写。若不填写，返回的结果中包含所有的资产。若填写，返回的结果中只显示由code和issuer指定的资产。 |

返回内容

```json
{
  "error_code" : 0,
  "result" : {
    "address" : "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3", //该账号的地址
    "assets" : [//该账号的所有资产
      {
        "amount" : 1400,
        "property" :
        {
          "code" : "CNY",
          "issuer" : "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3"
        }
      }, {
        "amount" : 1000,
        "property" :
        {
          "code" : "USD",
          "issuer" : "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3"
        }
      }
    ],
    "assets_hash" : "9696b03e4c3169380882e0217a986717adfc5877b495068152e6aa25370ecf4a",
    "contract" : null,
    "metadatas" : [//该账号的所有metadata
      {
        "key" : "123",
        "value" : "123_value",
        "version" : 1
      }, {
        "key" : "456",
        "value" : "456_value",
        "version" : 1
      }, {
        "key" : "abcd",
        "value" : "abcd_value",
        "version" : 1
      }
    ],
    "nonce" : 1, //账号当前作为交易源执行过的交易数量。若nonce为0，该字段不显示
    "priv" : {
      "master_weight" : 1,
      "thresholds" : {
        "tx_threshold" : 1
      }
    },
    "storage_hash" : "82c8407cc7cd77897be3100c47ed9d43ec4097ee1c00e2c13447187e5b1ac66c"
  }
}

```

- 如果该账号不存在,则返回内容

```json
{
   "error_code" : 4,
   "result" : null
}
```

### 查询账号基本信息

```text
HTTP GET /getAccountBase?address=buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3
```

 - 返回指定账号的基本信息

| 参数         | 描述                                                                                                                                                    |
| :----------- | ------------------------------------------------------------------------------------------------------------------------------------------------------- |
| address      | 账号地址， 必填                                                                                                                                         |

返回内容

```json
{
  "error_code" : 0,
  "result" : {
    "address" : "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3", //该账号的地址
    "assets_hash" : "9696b03e4c3169380882e0217a986717adfc5877b495068152e6aa25370ecf4a",
    "balance" : 899671600,
    "contract" : null,
    "nonce" : 1, //账号当前作为交易源执行过的交易数量。若nonce为0，该字段不显示
    "priv" : {
      "master_weight" : 1,
      "thresholds" : {
        "tx_threshold" : 1
      }
    },
    "storage_hash" : "82c8407cc7cd77897be3100c47ed9d43ec4097ee1c00e2c13447187e5b1ac66c"
  }
}

```

- 如果该账号不存在,则返回内容

```json
{
   "error_code" : 4,
   "result" : null
}
```

### 查询交易

```text
GET /getTransactionHistory?hash=ad545bfc26c440e324076fbbe1d8affbd8a2277858dc35927d425d0fe644e698&ledger_seq=2
```

| 参数       | 描述                     |
| :--------- | ------------------------ |
| hash       | 用交易的唯一标识hash查询 |
| ledger_seq | 查询指定区块中的所有交易 |
上述两个参数产生的约束条件是逻辑与的关系，如果您同时指定两个参数，系统将在指定的区块中查询指定的交易

返回示例

```json
{
    "error_code": 0,
    "result": {
        "total_count": 1, //本次查询结果，交易的数量
        "transactions": [ //每一个交易
            {
              "close_time": 1516350588062394,
              "error_code": 0,
              "error_desc": "",
              "hash": "3c2ebac2baec701730bec4665aab0cd045838b16a6b78f940fdd1af7715de100",
              "ledger_seq": 14212,
              "signatures": [{
                "public_key": "b00180c2007082d1e2519a0f2d08fd65ba607fe3b8be646192a2f18a5fa0bee8f7a810d011ed",
                "sign_data": "e950d36daaa51481f6aa7579d2ee8e893cb532d6ea536c716e89a529dd017f07ed8c0a130e897138a90b4bb44c11d4d105e65dc0070d28eb0cb5810c1f94f50a"
              }],
              "transaction": {
                "fee": 24900,
                "nonce": 11,
                "operations": [{
                  "pay_coin": {
                    "amount": 10000,
                    "dest_address": "buQiAAYKvQ7ZozHLShvBZe5ChTfK2eR6hKcS"
                  },
                  "type": 7
                }],
                "source_address": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3"
              }
            }
        ]
    }
}
```

如果没有查到交易则返回

```json
{
  "error_code": 4,
  "result":
  {
    "total_count": 0,
    "transactions": []
  }
}
```

### 查询区块头

```text
GET /getLedger?seq=xxxx&with_validator=true&with_consvalue=true
```

| 参数           | 描述                                      |
| :------------- | ----------------------------------------- |
| seq            | ledger的序号， 如果不填写，返回当前ledger |
| with_validator | 默认false，不显示验证节点列表       |
| with_consvalue | 默认false，不显示共识值             |
| with_fee       | 默认false，不显示费用配置             |


- 如果查询到ledger则返回内容:

```json
{
   "error_code" : 0,
   "result" : {
      "consensus_value" : {//当with_consvalue=true时才会显示
         "close_time" : 1506478618406358,//共识结果产生的时间
         "ledger_seq" : 136,//共识出来的区块号
         "previous_ledger_hash" : "f7096c0d7b85e766a459c9d0eb8756268d560270c7a1cf51fc0371baaa6573f4",//上一个区块序号
         "previous_proof" : "0aff010a2d080110022a2708011087012220f38b06d6fb8a825b537105e60e089f474b90f41de01ef43367c64553604d198612cd010a8801623030323034316136633636343233373139373465613061663862366636393136353230343964366262396539653363663739383363363763613362336563616538363337346565306465616665666563393432383138303936613335313335343635373861346632643761623664353133396339323737656264306634633931623563643739641240032efd35b8ea647dc9e687d7a79dc43344d8f58e9329070334778c70b2d7574b40fd26a9660f9f5240c93a123baaff70c6df812ac095e1b5e44790ca29ed4e1e",
         "previous_proof_plain" : {
            "commits" : [
               {
                  "pbft" : {
                     "commit" : {
                        "sequence" : 135,
                        "value_digest" : "f38b06d6fb8a825b537105e60e089f474b90f41de01ef43367c64553604d1986",
                        "view_number" : 1
                     },
                     "round_number" : 1,
                     "type" : 2
                  },
                  "signature" : {
                     "public_key" : "b002041a6c6642371974ea0af8b6f691652049d6bb9e9e3cf7983c67ca3b3ecae86374ee0deafefec942818096a3513546578a4f2d7ab6d5139c9277ebd0f4c91b5cd79d",
                     "sign_data" : "032efd35b8ea647dc9e687d7a79dc43344d8f58e9329070334778c70b2d7574b40fd26a9660f9f5240c93a123baaff70c6df812ac095e1b5e44790ca29ed4e1e"
                  }
               }
            ]
         },
         "txset" : null
      },
      "header" : {
         "account_tree_hash" : "d8a04da7daad5eb911c285ec56a4a5434d66af22f29f42b74373908c3efe64e0", //账号树的hash
         "close_time" : 1506478618406358, //本区块生成的时间
         "consensus_value_hash" : "5067eda92d887ce90f7f96b8ed1a000bd69755905286ed092fda00175e781a07", //共识结果的hash
         "hash" : "9a3c4dc78b40eaa18b962688b24a57b160d93703e9cfaef88c0a27bdff369bee",//本区块的hash
         "previous_hash" : "f7096c0d7b85e766a459c9d0eb8756268d560270c7a1cf51fc0371baaa6573f4",//上一个区块的hash
         "seq" : 136, //区块的序号
         "tx_count" : 1, //截止到当前区块，所有交易的数量
         "validators_hash" : "b744ccae0ecd1b0c09923564bc1556949e96f78755bdbcec06a82ddad420cca4", //共识节点集合的hash
         "version" : 1000 //区块版本
      },
      "validators" : [ "a002953b57b025662c9f9de93df4c319cc39bec181c2b5" ]//with_validator=true时才显示，共识节点集合
   }
}
```

- 如果没有查询到ledger返回的内容:

``` json
{
   "error_code" : 4,
   "result" : null
}
```

### 提交交易

```text
POST /submitTransaction
```

数据格式

```json
{
  "items" : [{
      "transaction_blob" : "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",//一个交易序列化之后的16进制表示
      "signatures" : [{//第一个签名
          "sign_data" : "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",//签名数据
          "public_key" : "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"//公钥
        }, {//第二个签名
          "sign_data" : "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",//签名数据
          "public_key" : "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"//公钥
        }
      ]
    }
  ]
}
```

| 参数             | 描述|
| :--------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| transaction_blob | 交易序列化之后的16进制格式。您可以参照[定义交易](#定义交易)来组装自己的交易数据                                                                                 |
| sign_data        | 签名数据， 16进制格式。其值为对transaction_blob进行签名(动词)得到的签名数据。__注意，签名时要先将transaction_blob转成字节流再签名，不要对16进制字符串直接签名__ |
| public_key       | 公钥， 16进制格式。|


### 序列化交易

如果您手中还没有protocol buffer工具，您可以使用本接口将交易序列化。

| 参数           | 描述                                                                                                    |
| :------------- | ------------------------------------------------------------------------------------------------------- |
| source_address | 必填，交易的发起方账号地址                                                                              |
| nonce          | 必填，交易序号，必须等于发起方账号的nonce+1。您可以通过[查询账号](#查询账号)返回的结果中得到账号的nonce |
| expr_condition | 表达式字段, 可选                                                                                        |
| metadata       | 可选, 用户自定义交易的备注数据, 必须为16进制表示                                                        |

关于operations中的数据格式，参照[操作](#操作)下面各种不同的操作的**json格式**

```http
POST /getTransactionBlob
```

```json
{
    "source_address":"xxxxxxxxxxx",//交易源账号，即交易的发起方
    "nonce":2, //nonce值
    "expr_condition":"", //可选，表达式。参见高级功能:表达式
    "fee":1000, //交易支付的费用
    "metadata":"0123456789abcdef", //可选，用户自定义给交易的备注，16进制格式
    "operations":[
    {
      //根据不同的操作填写
    },
    {
      //根据不同的操作填写
    }
    ......
    ]
}
```

返回内容

```json
{
    "error_code": 0,
    "error_desc": "",
    "result": {
        "hash": "474210d69cf0a797a24be65e187eddc7f15de626d38f8b49446b21ddd12247f8",//交易的hash
        "transaction_blob": "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" //交易序列化之后的16进制表示
    }
}
```

>例如，定义了1个json格式的交易，该交易只有1个操作，创建一个账号
`POST /getTransactionBlob`

```json
{
  "source_address": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3",
  "nonce": 1,
  "fee": 1000000,
  "operations": [{
      "type": 1,
      "create_account": {
        "dest_address": "buQgmhhxLwhdUvcWijzxumUHaNqZtJpWvNsf",
        "metadatas": [{
            "key": "hello",
            "value": "这是创建账号的过程中设置的一个metadata"
          }
        ],
        "priv": {
          "master_weight": 10,
          "signers": [],
          "thresholds": {
            "tx_threshold": 7
          }
        },
        "contract": {
          "payload": "function main(inputStr){\n /*这是合约入口函数*/ }"
        }
      }
    }
  ]
}
```

返回

```json
{
   "error_code" : 0,
   "error_desc" : "",
   "result" : {
      "hash" : "106345c939250c26495d461957a17cd58d9f69dc5d21c772e20e1ff337c7c2ce",
      "transaction_blob" : "0a2462755173396e70614371396d4e465a473138717538385a636d5859716436627170545533100122af0108012aaa010a24627551676d6868784c77686455766357696a7a78756d5548614e715a744a7057764e7366123a123866756e6374696f6e206d61696e28696e707574537472297b0a202f2ae8bf99e698afe59088e7baa6e585a5e58fa3e587bde695b02a2f207d1a06080a1a020807223e0a0568656c6c6f1235e8bf99e698afe5889be5bbbae8b4a6e58fb7e79a84e8bf87e7a88be4b8ade8aebee7bdaee79a84e4b880e4b8aa6d65746164617461"
   }
}
```
### 调试合约
```text
   POST /testContract
```
- post内容如下
```http
{
  "code" : "\"use strict\";log(undefined);function query() { return 1; }",
  "input" : "{}",
  "contract_balance" : "100009000000",
  "fee" : 100000000000000000,
  "exe_or_query" : false,
  "source_address" : ""
}
```
  - contract_address: 调用的智能合约地址，如果从数据库查询不到则返回错误。
  - code：需要调试的合约代码，如果 contract_address 为空，则使用code 字段，如果code字段你也为空，则返回错误。
  - input： 给被调用的合约传参。
  - fee : 执行合约传递的参数。
  - contract_balance : 赋予合约的初始 BU 余额。
  - exe_or_query: true :准备调用合约的读写接口main，false :调用只读接口query。
  - source_address：模拟调用合约的原地址。

  - 返回值如下：

```json
  {
   "error_code" : 0,
   "error_desc" : "",
   "result" : {
      "logs" : {
         "0-buQVkReBYUPUYHBinVDrrb9FQRpo49b9YRXq" : [ "undefined" ]
      },
      "query_rets" : [
         {
            "result" : {
               "type" : "number",
               "value" : "000000000000f03f",
               "valuePlain" : 1
            }
         }
      ],
      "real_fee" : 0,
      "stat" : {
         "apply_time" : 6315,
         "memory_usage" : 886176,
         "step" : 3
      },
      "txs" : null
   }
}
```

### 评估费用
```text
   POST /testTransaction
```
- post内容如下
```json
{
  "items": [
    {
      "transaction_json": {
        "source_address": "buQBDf23WtBBC8GySAZHsoBMVGeENWzSRYqB",
        "nonce": 6,
        "fee":0,
        "operations": [
          {
            "type": 7,
            "pay_coin": {
              "dest_address": "buQft4EdxHrtatWUXjTFD7xAbMXACnUyT8vw",
              "amount": 10000
            }
          }
        ]
      }
    }
  ]
}
```
  评估费用不改变账号余额基础上进行的评估，交易中涉及的原账号和目标账号都必须在系统中存在，创建账号的目标地址除外。
  - source_address：模拟交易的原地址。
  - nonce : 在原账号基础上加1。
  - fee : 费用填0系统会自动填写一个大概费用来进行交易，计算出实际费用，如果费用不足，交易终止；非0不填写费用字段，直接交易计算出实际费用，如果费用不足，交易终止。
  - operations : 可以是任何一种操作类型。

  - 返回值如下：

```json
{
    "error_code": 0,
    "error_desc": "",
    "result": {
        "hash": "63109579662d7165ee3c4de0a00932d8b721651101d3255e2326de10eea6de15",
        "logs": null,
        "query_rets": null,
        "real_fee": 1000,
        "stat": null,
        "txs": [
            {
                "transaction_env": {
                    "transaction": {
                        "fee": 99999999974939995,
                        "nonce": 6,
                        "operations": [
                            {
                                "pay_coin": {
                                    "amount": 10000,
                                    "dest_address": "buQft4EdxHrtatWUXjTFD7xAbMXACnUyT8vw"
                                },
                                "type": 7
                            }
                        ],
                        "source_address": "buQBDf23WtBBC8GySAZHsoBMVGeENWzSRYqB"
                    }
                }
            }
        ]
    }
}
```

## 定义交易

### 交易的基本结构

- json格式
  ```json
  {
      "source_address":"xxxxxxxxxxx",//交易源账号，即交易的发起方
      "nonce":2, //nonce值
      "fee" : 1000000, //愿为交易花费的手续费
      "expr_condition":"", //可选，表达式。参见高级功能:表达式
      "metadata":"0123456789abcdef", //可选，用户自定义给交易的备注，16进制格式
      "operations":[
      {
      //根据不同的操作填写
      },
      {
      //根据不同的操作填写
      }
      ......
      ]
  }
  ```
- protocol buffer 结构
  ```text
  message Transaction
  {
      enum Limit{
              UNKNOWN = 0;
              OPERATIONS = 1000;
      };
      string source_address = 1;
      int64 nonce = 2;
      string expr_condition = 3;
      repeated Operation operations = 4;
      bytes metadata = 5;
      int64  fee = 6;
  }
  ```

  交易Transaction有5个关键字段

  - source_address: 交易源账号，即交易发起方的账号。当这笔交易成功后，交易源账号的nonce字段会自动加1。账号中的nonce意义是本账号作为交易源执行过的交易数量。
  - nonce:其值必须等于交易源账号的当前nonce+1，这是为了防止重放攻击而设计的。如何查询一个账号的nonce可参考[查询账号](#查询账号)。若查询账号没有显示nonce值，说明账号的当前nonce是0.
  - expr_condition:针对本交易的表达式，高级功能。详见[表达式](#表达式)
  - operations:操作列表。本交易的有效负载，即本交易想要做什么事情。见[操作](#操作)
  - metadata:用户自定义字段，可以不填写，备注用。

  Operation是所有操作的父类:

  ```text
  message Operation
  {
      enum Type {
          UNKNOWN = 0;
          CREATE_ACCOUNT = 1;
          ISSUE_ASSET = 2;
          PAYMENT = 3;
          SET_METADATA = 4;
          SET_SIGNER_WEIGHT = 5;
          SET_THRESHOLD = 6;
          PAY_COIN = 7;
      };
      Type type = 1;
      string source_address = 2;
      bytes metadata = 3;
      string expr_condition = 4;

      OperationCreateAccount create_account = 5;
      OperationIssueAsset issue_asset = 6;
      OperationPayment payment = 7;
      OperationSetMetadata set_metadata = 9;
      OperationSetSignerWeight set_signer_weight = 10;
      OperationSetThreshold set_threshold = 11;
      OperationPayCoin pay_coin = 12;
  }
  ```

  - type: 操作类型的枚举。如其值为ISSUE_ASSET（发行资产），那么本操作中的issue_asset字段就会被使用；如果其值为PAYMENT，那么本操作中payment字段就会被使用……详见[操作码](#操作码)
  - source_address:操作源，即本操作针对哪个账号生效。若不填写，则默认本操作源等于本操作源。
  - metadata:本操作的备注，用户自定义，可以不填写
  - expr_condition:针对本操作的表达式，若您用不着这个功能，可以不填写。详见[表达式](#表达式)

### 操作

#### 操作码

| 代码值 | 枚举名            | 说明         |
| :----- | ----------------- | ------------ |
| 1      | CREATE_ACCOUNT    | 创建账号     |
| 2      | ISSUE_ASSET       | 发行资产     |
| 3      | PAYMENT           | 转移资产     |
| 4      | SET_METADATA      | 设置metadata |
| 5      | SET_SIGNER_WEIGHT | 设置权重     |
| 6      | SET_THRESHOLD     | 设置门限     |
| 7      | PAY_COIN          | 支付BU COIN  |

#### 创建账号

|参数|描述
|:--- | --- 
|dest_address |  账号的地址
|contract|  如果不填写，那么这是一个普通的账号。如果填写，那么这是一个合约账号
| priv|  该账号的权限信息
|init_balance | 初始化账户 BU 值 
|init_input | 给合约传初始化参数

- 功能
  在区块链上创建一个新的账号
- 成功条件
  - 各项参数合法
  - 要创建的账号不存在
- **注意：如果目标为合约账户，则priv配置必须符合 {"master_weight" : 0 , "thresholds": {"tx_threshold":1}}**
- json格式


```json
    {
      "type": 1,
      "create_account": {
        "dest_address": "buQgmhhxLwhdUvcWijzxumUHaNqZtJpWvNsf",
        "contract": {
          "payload": "function main(input) { /*do what ever you want*/ }"
        },
        "init_balance": 100000,  //give the init_balance to this account
        "init_input" : "",  // if create contract , then init with this input
        "metadatas": [{
            "key": "111",
            "value": "hello 111!",
            "version": 0
          }, {
            "key": "222",
            "value": "hello 222!",
            "version": 0
          }
        ],
        "priv": {
          "master_weight": 10,
          "signers": [{
              "address": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3",
              "weight": 6
            }
          ],
          "thresholds": {
            "tx_threshold": 7,
            "type_thresholds": [{
                "type": 1,
                "threshold": 8
              }, {
                "type": 2,
                "threshold": 5
              }
            ]
          }
        }
      }
    }
```

- protocol buffer 结构

  ```text
  message OperationCreateAccount
  {
      string dest_address = 1;
      Contract contract = 2;
      AccountPrivilege priv = 3;
      repeated KeyPair metadatas = 4;
      int64    init_balance = 5;
      string  init_input = 6;
  }
  ```

  - dest_address:要创建的账号的地址
  - contract:合约。若你想要创建一个不具有合约功能的账号，可以不填写这部分。若您想创建具有合约功能的账号，请参照[合约](#合约)
  - priv: 账号的初始权力分配。相关的数据结构定义:
      ```text
        message OperationTypeThreshold
        {
            Operation.Type type = 1;
            int64 threshold = 2;
        }

        message AccountPrivilege
        {
            int64 master_weight = 1;
            repeated Signer signers = 2;
            AccountThreshold thresholds = 3;
        }
        message Signer
        {
            enum Limit
            {
                SIGNER_NONE = 0;
                SIGNER = 100;
            };
            string address = 1;
            int64 weight = 2;
        }
        message AccountThreshold
        {
            int64 tx_threshold = 1; //required, [-1,MAX(INT64)] -1: 表示不设置
            repeated OperationTypeThreshold type_thresholds = 2; //如果这个设置，则操作门限以这个为准
        }
      ```

    若你想创建一个不受其他账号控制的账号。将priv.master_weight设置为1，将`priv.thresholds.tx_threshold`设为1即可。若您想创建一个受其他账号控制的账号，参见[控制权的分配](#控制权的分配)

  - metadatas:metadata列表。您可以为新建的账号设置一批初始的metadata。其数据类型为KeyPair,结构如下

    ```text
    message KeyPair
    {
        string key = 1;
        string value = 2;
        int64 version = 3;
    }
    ```

    这是一个版本化的键值对数据库，如果您不需要，可以不填写这部分。
  - init_balance: 给创建的账户初始化的 BU 数。
  - init_input:  如果被创建的账户是合约账户，则传参执行初始化合约操作。

#### 发行资产

|参数|描述
|:--- | --- 
|amount |  发行的数量
|code|  资产代码

- 功能
  操作源账号发行一笔数字资产，执行成功后操作源账号的资产余额中会出现这一笔资产
- 成功条件
  - 各项参数合法
  - 资产数量没有溢出（int64）
- json格式

    ```json
    {
      "type": 2,
      "issue_asset": {
        "amount": 1000,
        "code": "CNY"
      }
    }
    ```
- protocol buffer 结构
    ```text
    message OperationIssueAsset
    {
        string code = 1;
        int64 amount = 2;
    }
    ```
    - code:要发行的资产代码，长度范围[1, 64]
    - 发行的数量。数值范围(0,MAX(int64))
    - 执行成功后，操作源的资产表中会出现这部分新发行的资产

#### 转移资产

|参数|描述
|:--- | --- 
|payment.dest_address |  目标账户
|payment.asset.property.issuer|  资产发行方
|payment.asset.property.code|  资产代码
|payment.asset.amount|  要转移的数量
|payment.input|  触发合约调用的入参

- 功能
  操作源账号将一笔资产转给目标账号
- 成功条件
  - 各项参数合法
  - 源账号该类型的资产数量足够
- json格式


  ```JSON
    {
      "type": 3,
      "payment": {
        "dest_address": "buQgmhhxLwhdUvcWijzxumUHaNqZtJpWvNsf",
        "asset": {
          "property": {
            "issuer": "buQgmhhxLwhdUvcWijzxumUHaNqZtJpWvNsf",
            "code": "CNY"
          },
          "amount": 100
        },
        "input": "{\"bar\":\"foo\"}"
      }
    }
  ```

- protocol buffer 结构
    ```text
    message OperationPayment
    {
        string dest_address = 1;
        Asset asset = 2;
        string input = 3;
    }
    ```
    - dest_address: 资产接收方账号地址
    - asset: 要转移的资产
    ```text
    message Asset
    {
         AssetProperty property = 1; //资产属性
         int64 amount = 2; //数量
    }

    message AssetProperty
    {
         string issuer = 1; //资产发行方
         string code = 2; //资产代码
    }
    ```
    - input: 本次转移触发接收方的合约，合约的执行入参就是input

#### 设置metadata
|参数|描述
|:--- | --- 
| set_metadata.key  |required，length:(0, 1024]
| set_metadata.value  |optional，length:[0, 256K]
| set_metadata.version |optional，default 0, 0：不限制版本，>0 : 当前 value 的版本必须为该值， <0 : 非法

- 功能
  操作源账号修改或添加一个metadata到自己的metadata表中
- 成功条件
  - 各项参数合法
- json格式

    ```JSON
    {
      "type": 4,
      "set_metadata": {
        "key": "abc",
        "value": "hello abc!",
        "version": 0
      }
    }
    ```
    
- protocol buffer 结构
    ```text
    message OperationSetMetadata
    {
        string key = 1;
        string value = 2;
        int64 version = 3;
    }
    ```
    - key: 主键，账号内唯一。长度范围[1,1024]
    - value: 值。长度范围[0,256K]
    - version: 版本号，可以不填写。若您想使用这个高级功能，参见[版本化控制](#版本化控制)

#### 设置权重
|参数|描述
|:--- | --- 
|master_weight |optional，default 0， -1 ： 不设置该值，0：设置master权重值为0， >0 && <= MAX(UINT32)：设置权重值为该值，其他：非法
|address |需要操作的 signer 地址，符合地址校验规则。
|weight | optional，default 0, 0 ：删除该signer，>0 && <= MAX(UINT32)：设置权重值为该值，其他：非法

- 功能
  设置签名者拥有的权重
- 成功条件
  - 各项参数合法
- json格式
  ```json
  {
    "type": 5,
    "set_signer_weight": {
      "master_weight": 2,
      "signers": [{
          "address": "buQgmhhxLwhdUvcWijzxumUHaNqZtJpWvNsf",
          "weight": 2
        }
      ]
    }
  }
    ```

- protocol buffer 结构
    ```text
    message OperationSetSignerWeight
    {
         int64 master_weight = 1; //required, [-1,MAX(UINT32)] -1: 表示不设置
         repeated Signer signers = 2; //address:weight, 如果weight 为0 表示删除这个signer
    }
    ```
    - master_weight:本账号地址拥有的权力值
    - 各个签名者的权力值, Signer的定义如下
    ```text
    message Signer
    {
    enum Limit{
            SIGNER_NONE = 0;
            SIGNER = 100;
    };
         string address = 1;
         int64 weight = 2;
    }
    ```

#### 设置门限
|参数|描述
|:--- | --- 
|tx_threshold |optional，default 0, 表示该账号的最低权限，-1: 表示不设置该值，>0 && <= MAX(INT64)：设置权重值为该值，其他：非法
|type |表示某种类型的操作  (0, 100]
|threshold | optional，default 0, 0 ：删除该类型操作，>0 && <= MAX(INT64)：设置权重值为该值，其他：非法

- 功能
  设置各个操作所需要的门限
- 成功条件
  - 各项参数合法
- json格式
    ```json
    {
      "type": 6,
      "set_threshold": {
        "tx_threshold": 7,
        "type_thresholds": [{
            "type": 1,
            "threshold": 8
          }, {
            "type": 2,
            "threshold": 5
          }
        ]
      }
    }
    ```

- protocol buffer 结构

  ```text
  message OperationSetThreshold
  {
          int64 tx_threshold = 1;
          repeated OperationTypeThreshold type_thresholds = 4; //type:threshold ，threshold:0 表示删除这个类型的type
  }
  ```

  OperationTypeThreshold的结构定义如下

  ```text
  message OperationTypeThreshold{
        Operation.Type type = 1;  //操作码，哪种操作
        int64 threshold = 2;    //代表这种操作所需的权重门限
  }
  ```

#### 转移bu资产

|参数|描述
|:--- | --- 
|pay_coin.dest_address |  目标账户
|pay_coin.amount|  要转移的数量
|pay_coin.input|  触发合约调用的入参

- 功能
  操作源账号将一笔资产转给目标账号
- 成功条件
  - 各项参数合法
  - 源账号该类型的资产数量足够
- json格式


  ```JSON
    {
      "type": 7,
      "pay_coin": {
          "dest_address": "buQgmhhxLwhdUvcWijzxumUHaNqZtJpWvNsf",
          "amount": 100,
          "input": "{\"bar\":\"foo\"}"
        }
      }
    }
  ```

- protocol buffer 结构
    ```text
    message OperationPayCoin
    {
        string dest_address = 1;
        int64 amount = 2;
        string input = 3;
    }
    ```
    - dest_address: BU接收方账号地址
    - amount: 要转移的BU数量
    - input: 本次转移触发接收方的合约，合约的执行入参就是input

## 高级功能

### 控制权的分配

您在创建一个账号时，可以指定这个账号的控制权分配。您可以通过设置priv的值设置。下面是一个简单的例子

```json
{
    "master_weight": 70,//本地址私钥拥有的权力值 70
    "signers": [//分配出去的权力
        {
            "address": "buQc39cgJDBaFGiiAsRtYKuaiSFdbVGheWWk",
            "weight": 55    //上面这个地址拥有权力值55
        },
        {
            "address": "buQts6DfT5KavtV94JgZy75H9piwmb7KoUWg",
            "weight": 100    //上面这个地址拥有权力值100
        }
    ],
    "thresholds"://不同的操作所需的权力阈值
    {
        "tx_threshold": 8,//发起交易需要权力值 8
        "type_thresholds": [
            {
                "type": 1,//创建账号需要权利值 11
                "threshold": 11
            },
            {//发行资产需要权利值 21
                "type": 2,
                "threshold": 21
            },
            {//转移资产需要权力值 31
                "type": 3,
                "threshold": 31
            },
            {//设置metadata需要权利值 41
                "type": 4,
                "threshold": 41
            },
            {//变更控制人的权力值需要权利值 51
                "type": 5,
                "threshold": 51
            },
            {//变更各种操作的阈值需要权利值 61
                "type": 6,
                "threshold": 61
            }
        ]
    }
}
```

### 版本化控制

每一个账号的metadata都是一个版本化的小型数据库。版本化的特点是可以避免修改冲突的问题。

### 表达式

该表达式字段，用于自定义交易有效规则，比如设置交易在某个账户的master_weight 大于 100 有效，则填:

```JavaScript
jsonpath(account("buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3"), ".priv.master_weight") > 100
```

表达式可用的函数

| 参数                            | 描述                                 |
| :------------------------------ | ------------------------------------ |
| account(address)                | 获取账户信息, 返回 json 序列化字符串 |
| jsonpath(json_string, path)     | 获取json对象的属性值                 |
| LEDGER_SEQ                      | 内置变量，代表最新的区块高度         |
| LEDGER_TIME                     | 内置变量，代表最新的区块生成时间     |
| `( )` `=`                       | 嵌套括号，函数调用如sqrt(x)          |
| `*` `/`                         | 乘法、除法                           |
| `+` `-`                         | 加法、减法                           |
| `<` `<=`  `>`  `>=`  `==`  `!=` | 比较运算                             |
| `&&`                            | 与 运算                              |
| \|\|                              | 或运算                               |
| ,                               | 逗号运算                             |

>例: Alice发起了一笔交易，她想要使这笔交易在5秒内有效。她发现当前时间戳(微秒)是 `1506393720000000`，5秒之后也就是 `1506393725000000`。那么她就可以在交易中的`expr_condition`字段写上下面这句`LEDGER_TIME >= 1506393720000000 && LEDGER_TIME <= 1506393725000000`这句话的意思是，该交易只在时间范围`[1506393720000000, 1506393725000000]`内有效。过段时间之后，Alice发现当前时间已经超过1506393725000000，那么Alice就可以断定这笔交易要么已经被处理，要么已经彻底失效。

### 合约

合约是一段JavaScript代码,标准(ECMAScript as specified in ECMA-262)。合约的初始化函数是init, 执行的入口函数是main函数，您写的合约代码中必须有init和main函数的定义。该函数的入参是字符串input，是调用该合约的时候指定的。
下面是一个简单的例子

```javascript
function init(bar)
{
  /*init whatever you want*/

}

function main(input)
{
  var para = JSON.parse(input);
  if (para.do_foo)
  {
    var x = {
      'hello' : 'world'
    };
  }
}

function query(input)
{ 
  return input;
}
```

系统提供了几个全局函数, 这些函数可以获取区块链的一些信息，也可驱动账号发起所有交易，除了设置门限和权重这两种类型的操作。
**注意，自定义的函数和变量不要与内置变量和全局函数重名，否则会造成不可控的数据错误。**

#### 内置函数

- ##### 返回值介绍
   所有内部函数的调用，如果失败则 返回 false ，成功则为其他对象。

- ##### 获取账号信息(不包含metada和资产)

    `getBalance(address);`
     address: 账号地址

    例如
    ```javascript
    let balance = getBalance('buQsZNDpqHJZ4g5hz47CqVMk5154w1bHKsHY');
    /*
    balance 具有如下格式
     '9999111100000'
    */
    ```

- ##### 存储合约账号的metadata信息
  `storageStore(metadata_key, metadata_value);`
  - metadata_key: metadata的key
  ```javascript
  storageStore('abc', 'values');
  /*
    参数字符串格式
    执行成功或者失败抛异常
  */

  ```

- ##### 获取合约账号的metadata信息
  `storageLoad(metadata_key);`
  - metadata_key: metadata的key
  ```javascript
  let value = storageLoad('abc');
  /*
    value 的值是如下的格式
    'values'
    失败返回false
  */

  ```
    即可得到本合约账号中自定数据的abc的值

- ##### 删除合约账号的metadata信息
  `storageDel(metadata_key);`
  - metadata_key: metadata的key
  ```javascript
  storageDel('abc');
  /*
    参数字符串格式
    执行成功或者失败抛异常
  */

  ```
    即可删除本合约账号中自定数据的abc的值

- ##### 获取某个账号的资产信息

    `getAccountAsset(account_address, asset_property);`

    - account_address: 账号地址
    - asset_property: 资产属性

    例如
    ```javascript
    let asset_property =
    {
      'issuer' : 'buQsZNDpqHJZ4g5hz47CqVMk5154w1bHKsHY',
      'code' : 'CNY'
    };
    let bar = getAccountAsset('buQsZNDpqHJZ4g5hz47CqVMk5154w1bHKsHY', asset_property);

    /*
     1

    失败返回false
    */
    ```

- ##### 获取区块信息

    `getBlockHash(offset_seq);`
    - offset_seq: 距离最后一个区块的偏移量，最大1024

    例如
    ```javascript
    let ledger = getBlockHash(4);
    /*
    'c2f6892eb934d56076a49f8b01aeb3f635df3d51aaed04ca521da3494451afb3'
    失败返回false
    */

    ```

- ##### 64位加法

    `int64Plus(left_value, right_value);`
    - left_value: 左值
    - right_value：右值

    例如
    ```javascript
    let ret = int64Plus('12345678912345', 1);
    /*
    成功：'12345678912346'
    失败：抛异常
    */

    ```

- ##### 64位减法

    `int64Sub(left_value, right_value);`
    - left_value: 左值
    - right_value：右值

    例如
    ```javascript
    let ret = int64Sub('12345678912345', 1);
    /*
    成功：'123456789123464'
    失败：抛异常
    */

    ```
    
- ##### 64位乘法

    `int64Mul(left_value, right_value);`
    - left_value: 左值
    - right_value：右值

    例如
    ```javascript
    let ret = int64Mul('12345678912345', 2);
    /*
    成功：'24691357824690'
    失败：抛异常
    */

    ```

 - ##### 64位除法

    `int64Div(left_value, right_value);`
    - left_value: 左值
    - right_value：右值

    例如
    ```javascript
    let ret = int64Div('12345678912345', 2);
    /*
    成功：'6172839456172'
    失败：抛异常
    */

    ```

 - ##### 64位取模

    `int64Mod(left_value, right_value);`
    - left_value: 左值
    - right_value：右值

    例如
    ```javascript
    let ret = int64Mod('12345678912345', 2);
    /*
    成功：'1'
    失败：抛异常
    */

    ```

 - ##### 64位比较
    `int64Compare(left_value, right_value);`

    - 返回值 1：左值大于右值，0：等于，-1 ：小于
    - left_value: 左值
    - right_value：右值

    例如
    ```javascript
    let ret = int64Compare('12345678912345', 2);
    /*
    成功：1
    失败：抛异常
    */

    ```
      
 - ##### 变换单位
    `toSatoshi(value);`

    - 返回值: 乘以 10^8
    - value: 左值

    例如
    ```javascript
    var ret = toSatoshi('12345678912');
    /*
    '1234567891200000000'
    */

    ```

- ##### 输出日志

    `log(info);`
     info: 日志内容

    例如
    ```javascript
    let ret = log('buQsZNDpqHJZ4g5hz47CqVMk5154w1bHKsHY');
    /*
     成功不返回,失败返回false
    */
    ```
- #### 输出交易日志

    `tlog(topic,args...);`
     - tlog会产生一笔交易写在区块上
     - topic: 日志主题，必须为字符串类型,参数长度(0,128]
     - args...: 最多可以包含5个参数，参数类型可以是字符串、数值或者布尔类型,每个参数长度(0,1024]

    例如
    ```javascript
    tlog('transfer',sender +' transfer 1000',true);
    /*
     成功不返回,失败抛异常
    */
    ```

- ##### 做交易
    ```javascript
    doTransaction(transaction)
    ```

    令合约账号做一笔交易，即里面的任意一个操作的`source_address`都会自动变成合约账号。所以`source_address`是不需要填写的。

  - 入参: transaction, 必须为字符串，且可反序列化
  - 返回值: true/false

    例如
    ```javascript
    let transaction =
    {
      'operations' :
      [
        {
          "type" : 2,
          "issue_asset" :
          {
            "amount" : 1000,
            "code" : "CNY"
          }
        }
      ]
    };

    doTransaction(JSON.stringify(transaction));
    /*失败抛异常*/
    ```

- ##### 转账

    `payCoin(address, amount[, input]);`
     - address: 发送BU的目标地址
     - amount: 发送BU的数量
     - input: 可选，合约参数

    例如
    ```javascript
    payCoin("buQsZNDpqHJZ4g5hz47CqVMk5154w1bHKsHY", "10000", "{}");
    /*失败抛异常*/
    ```

- ##### 断言

    `assert(condition[, message]);`
     - condition: 断言变量
     - message: 可选，失败时抛出异常的消息

    例如
    ```javascript
    assert(1===1, "Not valid");
    /*失败抛异常*/
    ```

#### 内置变量

- #####  该合约账号的地址
   thisAddress

    全局变量`thisAddress`的值等于该合约账号的地址。

    例如账号x发起了一笔交易调用合约Y，本次执行过程中，thisAddress的值就是Y合约账号的地址。

    ```text
    var bar = thisAddress;
    /*
    bar的值是Y合约的账号地址。
    */
    ```

- ##### 本次支付操作的 BU coin
    payCoinAmount

- ##### 本次支付操作的Asset
    payAssetAmount

    为对象类型{"amount": 1000, "asset_property" : {"issuer": "buQsZNDpqHJZ4g5hz47CqVMk5154w1bHKsHY", "code":"CNY"}}
 
- ##### 当前区块高度
    blockNumber
 
- ##### 当前区块时间戳
    blockTimestamp

- ##### 调用者的地址
    sender
    ```sender``` 的值等于本次调用该合约的账号。

    例如某账号发起了一笔交易，该交易中有个操作是调用合约Y（该操作的source_address是x），那么合约Y执行过程中，sender的值就是x账号的地址。

    ```javascript
    let bar = sender;
    /*
    那么bar的值是x的账号地址。
    */
    ```

- ##### 触发本次合约调用的操作的序号
    triggerIndex

    ```triggerIndex``` 的值等于触发本次合约的操作的序号。

    例如某账号A发起了一笔交易tx0，tx0中第0（从0开始计数）个操作是给某个合约账户转移资产(调用合约), 那么```triggerIndex```的值就是0。

    ```javascript
    let bar = triggerIndex;
    /* bar 是一个非负整数*/
    ```

#### 异常处理

- JavaScript异常

  当合约运行中出现未捕获的JavaScript异常时，处理规定：

  1. 本次合约执行失败，合约中做的所有交易都不会生效。
  1. 触发本次合约的这笔交易为失败。错误代码为`151`。
  >例: Bob的合约是这么写的
  ```javascript
  function main(inputStr) {
    let recvAsset = trigger.transaction.operations[triggerIndex].payment.asset;

    if (recvAsset.property.code != 'CNY' || recvAsset.property.issuer != 'buQsZNDpqHJZ4g5hz47CqVMk5154w1bHKsHY') {
      throw '不支持的资产类型';
    }
    let tx = {
      'operations': [{
          'type': 3,
          'payment': {
            'dest_address': sender,
            'asset': {
              'property': {
                'issuer': thisAddress,
                'code': 'IOU'
              },
              'amount': recvAsset.amount
            }
          }
        }
      ]
    };

    doTransaction(JSON.stringify(tx));

  }
  ```
  这段合约的功能就是，Bob以1：1的价格收`buQsZNDpqHJZ4g5hz47CqVMk5154w1bHKsHY`发行的`CNY`,售卖自己的IOU借条(I owe you )。Alice向Bob转移一笔资产，触发了Bob的合约，如果此时Bob的IOU已经卖完，那么会执行到`throw 'IOU卖完了';`这一步。未捕获的异常会导致JavaScript代码执行出错，那么本次合约执行失败。Alice转给Bob的资产会自动回退到Alice的账户中，同时，Alice的这笔交易执行状态为失败，错误代码为`151`。

- 执行交易失败
  <font color=red>合约中可以执行多个交易，只要有一个交易失败，就会抛出异常，导致整个交易失败</font>

### 验证者节点选举

#### 创建选举合约账户
验证节点选举账户创建成功后，才可以进行后续的操作, 且该账户是全局唯一的, 不能重复创建。

- 创建一个合约账户（参见[创建账号](#创建账号)），账户的地址必须是buQtxgoaDrVJGtoPT66YnA2S84yE8FbBqQDJ。

>例

```
"validators_vote_account": "buQoR4qiEEEB1LQkeQCcpFQLqBEzKYjTXftY",
```

- 将 src\ledger\validators_vote.js 文件中的源码全部拷贝作为账户中 payload 字段的值。

>例

```
     "contract" :
     {
       "contract_id" : "something identify this contract",
       "payload" : "拷贝 src\ledger\validators_vote.js 中全部代码至此处"
     },
```

 在 validators_vote.js 文件指定的合约代码中, 以下变量可根据需要修改。但一旦验证节点选举账户创建成功，则不能再修改。

 ```
   let validatorSetSize       = 100;
   let votePassRate           = 0.8;
   let effectiveVoteInterval  = 15 * 24 * 60 * 60 * 1000 * 1000;
   let minPledgeAmount        = 100 * 100000000;
   let minSuperadditionAmount = 100 * 100000000;
```
 
 - validatorSetSize 指定网络内验证节点的个数；
 - votePassRate 设置投票通过值，只有验证节点有投票权限，投票数 / 验证节点总数 >= votePassRate 则投票通过;
 - effectiveVoteInterval 设置投票有效期，单位为微秒，超过有效期，则提案和投票作废；
 - minPledgeAmount 设置最小押金数额，低于该额度则拒绝；
 - minSuperadditionAmount 设置押金最小追加数额，低于该数额将被拒绝。

#### 申请成为验证节点候选人

任意一个拥有网络节点的账户可以通过向验证节点选举账户转移一笔 coin 作为押金，申请成为验证节点候选人。但能否成为候选节点，需经过验证节点投票决定。

- 申请者向验证节点选举账户转移一笔 coin 作为押金（参见[转移BU资产](#转移BU资产)），该押金可通过 ‘[收回押金](#收回押金)’ 操作收回。
- ‘转移货币’操作的 input 字段填入 { "method" : "pledgeCoin"}，注意使用转义字符。

>例

```
  "pay_coin" :
  {
    "dest_address" : "buQoR4qiEEEB1LQkeQCcpFQLqBEzKYjTXftY",
    "amount" :100000,
    "input":
    "{\"method\":\"pledgeCoin\"}"
  }
``` 

验证节点将对申请者投票(请参见‘[对验证节点候选人申请者投票](#对验证节点候选人申请者投票)’)，投票通过后将成为验证节点候选人。
候选人能否成为真正的验证节点取决于押金数额。假设网络需要 100 个验证节点（ validatorSetSize 值为 100），那么押金数额排在前 100 位以内的申请者将成为验证节点。押金数额可以追加，且不限次数，追加方式即再次执行‘[申请成为验证节点候选人](#申请成为验证节点候选人)’操作。申请者可通过‘[查询功能](#查询功能)’查询候选人名单和他们所交纳的押金数额。

注意：申请成为验证节点候选人的账户必须拥有节点，且节点和账户为同一地址。

#### 对验证节点候选人申请者投票

验证节点对申请者投票通过后，则该节点成为验证节点候选人。如果投票有效期内未通过，可以[收回押金](#收回押金)。

- 向验证节点选举账户转移任意一笔资产或者一笔数额为 0 的 coin。
- ‘转移资产’或‘转移货币’操作的 input 字段填入 { "method" : "voteForApplicant", "params" : { "address" : "填入申请者地址" } }，注意使用转义字符。

>例

```
  "pay_coin" :
  {
    "dest_address" : "buQoR4qiEEEB1LQkeQCcpFQLqBEzKYjTXftY",
    "amount" :0,
    "input":
    "{ 
        \"method\":\"voteForApplicant\",
        \"params\":
        { 
          \"address\":\"buQtZrMdBQqYzfxvqKX3M8qLZD3LNAuoSKj4\"
        }
    }"
  }
```

注意：只有验证节点有投票权，且每个验证节点只能投票一次。如果在有效投票期内投票未通过，则申请提案和已投票数全部作废。

#### 收回押金

验证节点、验证节点候选人和候选人申请者可通过此操作收回全部押金。

- 向验证节点选举账户转移任意一笔资产或者一笔数额为 0 的 coin。
- ‘转移资产’或‘转移货币’操作的 input 字段填入 { "method":"takebackCoin" }，注意使用转义字符。

>例

```
  "pay_coin" :
  {
    "dest_address" : "buQoR4qiEEEB1LQkeQCcpFQLqBEzKYjTXftY",
    "amount" :0,
    "input":"{\"method\":\"takebackCoin\"}"
  }
```

操作成功后，选举账户会将所有押金退回原账户，如果当前节点已经是验证节点，将丢失验证节点身份。选举合约将选出新的验证节点替代退出验证节点的位置。

#### 废止恶意节点

如果某验证节点发现有另一个验证节点为恶意节点，或者不再适合作为验证节点，可以申请废止该恶意节点。发起‘废止恶意节点’提案后，需要所有验证节点投票决定是否执行废止操作。

- 废止者向验证节点选举账户转移任意一笔资产或者一笔数额为 0 的 coin。
- ‘转移资产’或‘转移货币’操作的 input 字段填入 { "method" : "abolishValidator",  "params" : { "address" : "此处填入恶意验证节点地址", "proof" : "此处填入废止该验证节点的原因"} }，注意使用转义字符。

>例

```
  "pay_coin" :
  {
    "dest_address" : "buQoR4qiEEEB1LQkeQCcpFQLqBEzKYjTXftY",
    "amount" :0,
    "input":
    "{
      \"method\":\"abolishValidator\",
      \"params\":
      {
        \"address\":\"buQmvKW11Xy1GL9RUXJKrydWuNykfaQr9SKE\"，
        \"proof\":\"I_saw_it_uncomfotable.\"
      }
    }"
  }
```

注意：申请废止者和被废止者必须都是验证者节点。

#### 取消废止恶意节点
如果发起废止操作的验证节点后来发现被废止节点并非恶意节点，可以取消废止操作。但如果该废止操作已经被其他验证节点投票通过，则无法取消。

- 废止者向验证节点选举账户转移任意一笔资产或者一笔数额为 0 的 coin。
- ‘转移资产’或‘转移货币’操作的 input 字段填入 { "method" : "quitAbolish",  "params" : { "address" : "此处填入恶意验证节点地址" } }。

>例

```
  "pay_coin" :
  {
    "dest_address" : "buQoR4qiEEEB1LQkeQCcpFQLqBEzKYjTXftY",
    "amount" :0,
    "input":
    "{ 
      \"method\":\"quitAbolish\",
      \"params\":
      { 
        \"address\":\"buQmvKW11Xy1GL9RUXJKrydWuNykfaQr9SKE\"
      }
    }"
  }
```

注意：只有申请废止者才可以取消，其他节点和验证者节点无权取消。

#### 对废止恶意节点投票
投票通过后，恶意节点将被废止。恶意节点被废止后，选举账户只将该节点 90% 的押金退回原账户，10% 的押金将被罚没，且平均分给剩余其他所有验证节点作为押金, 取模的余数奖励给废止恶意节点提案者。

- 验证节点向选举账户转移任意一笔资产或者一笔数额为 0 的 coin。
- ‘转移资产’或‘转移货币’操作的 input 字段填入 { "method" : "voteForAbolish", "params" : { "address" : "此处填入被投票的恶意验证节点地址" } }，注意使用转移字符。

>例

```
  "pay_coin" :
  {
    "dest_address" : "buQoR4qiEEEB1LQkeQCcpFQLqBEzKYjTXftY",
    "amount" :0,
    "input":
    "{
       \"method\":\"voteForAbolish\",
      \"params\":
      {
         \"address\":\"buQmvKW11Xy1GL9RUXJKrydWuNykfaQr9SKE\"
      }
    }"
  }
```

注意：只有验证节点拥有投票权。若有效期内该废止提案未投票通过，则提案作废，申请被废止的节点将继续作为验证节点参与共识。

#### 查询功能

用户通过向查询接口提供指定参数，可以查看相关信息。

- 用户向验证节点选举账户转移任意一笔资产或者一笔数额为 0 的 coin。
- ‘转移资产’或‘转移货币’操作的 input 字段填入  { "method" : "查询方法", "params" : { "查询参数" } }，注意使用转义字符。

##### 查询当前验证节点集合

>例

```
  "pay_coin" :
  {
    "dest_address" : "buQoR4qiEEEB1LQkeQCcpFQLqBEzKYjTXftY",
    "amount" :0,
    "input": "{\"method\":\"getValidators\"}"
  }
```

##### 查询验证节点候选人名单和押金数额；

>例

```
  "pay_coin" :
  {
    "dest_address" : "buQoR4qiEEEB1LQkeQCcpFQLqBEzKYjTXftY",
    "amount" :0,
    "input": "{\"method\":\"getCandidates\"}"
  }
```

##### 查询指定的候选节点申请者提案信息
input 中的 address 字段填入申请者地址。

>例

```
  "pay_coin" :
  {
    "dest_address" : "buQoR4qiEEEB1LQkeQCcpFQLqBEzKYjTXftY",
    "amount" :0,
    "input":
    "{
       \"method\":\"getApplicantProposal\",
      \"params\":
      {
         \"address\":\"buQmvKW11Xy1GL9RUXJKrydWuNykfaQr9SKE\"
      }
    }"
  }
```

##### 查询指定的废止恶意节点提案的信息
input 中的 address 字段填入指定的恶意节点地址。

>例

```
  "pay_coin" :
  {
    "dest_address" : "buQoR4qiEEEB1LQkeQCcpFQLqBEzKYjTXftY",
    "amount" :0,
    "input":
    "{
       \"method\":\"getAbolishProposal\",
      \"params\":
      {
         \"address\":\"buQmvKW11Xy1GL9RUXJKrydWuNykfaQr9SKE\"
      }
    }"
  }
```

### 费用选举合约

此合约为交易费用标准制定合约，每项费用标准由共识节点提出议案，所有共识节点投票选举，获胜的议案作为新的费用收取标准，在下一个区块实施，当提案提出后超过15天仍未胜出，提案作废。
此合约地址：buQiQgRerQM1fUM3GkqUftpNxGzNg2AdJBpe

#### 费用结构

  ```text
  message FeeConfig
  {
	enum Type {
		UNKNOWN = 0;
		BYTE_FEE = 1;
		BASE_RESERVE_FEE = 2;
		CREATE_ACCOUNT_FEE = 3;
		ISSUE_ASSET_FEE = 4;
		PAYMENT_FEE = 5;
		SET_METADATA_FEE = 6;
		SET_SIGNER_WEIGHT_FEE = 7;
		SET_THRESHOLD_FEE = 8;
		PAY_COIN_FEE = 9;
	};
	int64 byte_fee = 1;
	int64 base_reserve = 2;
	int64 create_account_fee = 3;
	int64 issue_asset_fee = 4;
	int64 pay_fee = 5;	
	int64 set_metadata_fee = 6;
	int64 set_sigure_weight_fee = 7;
	int64 set_threshold_fee = 8;
	int64 pay_coin_fee = 9;
  }
  ```

#### 费用种类

| 代码值 | 枚举名            | 说明                     |
| :----- | --------------------- | ------------------- |
| 1      | BYTE_FEE              | 字节费               |
| 2      | BASE_RESERVE_FEE      | 预留费用             |
| 3      | CREATE_ACCOUNT_FEE    | 创建账号费用         |
| 4      | ISSUE_ASSET_FEE       | 发行资产费用         |
| 5      | PAYMENT_FEE           | 转移资产费用         |
| 6      | SET_METADATA_FEE      | 设置metadata费用     |
| 7      | SET_SIGNER_WEIGHT_FEE | 设置权重费用         |
| 8      | SET_THRESHOLD_FEE     | 设置门限费用         |
| 9      | PAY_COIN_FEE          | 支付费用             |

#### 查询历史费用

```text
GET /getLedger?seq=xxxx&with_fee=true
```

| 参数           | 描述                                      |
| :------------- | ----------------------------------------- |
| seq            | ledger的序号， 如果不填写，返回当前ledger |
| with_fee | 默认false，不显示费用列表       |

- 如果查询到ledger则返回内容:

```json
{
    "error_code": 0,
    "result": {
        "fees": {
            "base_reserve": 5,
            "byte_fee": 1,
            "create_account_fee": 1,
            "issue_asset_fee": 1,
            "pay_coin_fee": 1,
            "pay_fee": 1,
            "set_metadata_fee": 1,
            "set_sigure_weight_fee": 1,
            "set_threshold_fee": 1
        },
        "header": {
            "account_tree_hash": "c4e74a027e7fb7a96fa2c028a24fd8296df049decf928ee173ec4106ae016b0c",
            "consensus_value_hash": "2779905317234108f76cf5d65125cc0e9a3fd58e2140226bf79e8917028c8c9c",
            "fees_hash": "d7618678ae72e7154fc4366f0a43154ee6bb786b3cefd46287e608e90e13a651",
            "hash": "0c1b9d4b5a72d42f73917114c71672f78a3d951b7f7b89663d10138ea409c308",
            "seq": 1,
            "validators_hash": "cc4b0517295210c24ab037e3e9e122fd0c71e3c87994b7f913860de0a1930c36",
            "version": 1000
        }
    }
}
```

#### 查询费用提案

通过发送接口testContract接口查询。合约入参input参数json格式

```json
{
    "method":"queryProposal",
    "params":""
}
```

json格式需转换成字符串形式填写到testContract接口结构

```json
{
    "contract_address" : "buQebeTXVPA8mTt2fmBi51GifPbsqDPPURK1",
    "code" : "",
    "input" : "{\"method\":\"queryProposal\",\"param\":\"\"}"},
    "exe_or_query" : false,
    "source_address" : "",
    "fee":100000
}
```

contract_address赋值为区块上的费用选举合约地址，exe_or_query 为false代表查询


- 如果查询到则返回内容:

```json
{
    "error_code": 0,
    "error_desc": "",
    "result": {
        "logs": {
            "0-buQebeTXVPA8mTt2fmBi51GifPbsqDPPURK1": null
        },
        "query_rets": [
            {
                "result": {
                    "type": "string",
                    "value": "{\"buQft4EdxHrtatWUXjTFD7xAbMXACnUyT8vw1\":{\"accountId\":\"buQft4EdxHrtatWUXjTFD7xAbMXACnUyT8vw\",\"proposalId\":\"buQft4EdxHrtatWUXjTFD7xAbMXACnUyT8vw1\",\"feeType\":1,\"price\":\"5\",\"voteCount\":0,\"time\":1517470155872949}}"
                }
            }
        ],
        "real_fee": 0,
        "stat": {
            "apply_time": 11342,
            "memory_usage": 1325072,
            "step": 16
        },
        "txs": null
    }
}
```

result 域的value值为返回结果，反序列化为json格式即可得到所有的费用提案

#### 查询投票

通过发送接口testContract接口查询，可根据提案id进行查询单项。合约入参input参数json格式

```json
{
    "method":"queryVote",
    "params":{
      "proposalId": "buQft4EdxHrtatWUXjTFD7xAbMXACnUyT8vw1"
    }
}
```

json格式需转换成字符串形式填写到testContract接口结构

```json
{
    "contract_address" : "buQebeTXVPA8mTt2fmBi51GifPbsqDPPURK1",
    "code" : "",
    "input" :"{\"method\":\"queryVote\",\"params\":{\"proposalId\":\"buQft4EdxHrtatWUXjTFD7xAbMXACnUyT8vw1\"}}"},
    "exe_or_query" : false,
    "source_address" : "",
    "fee":100000
}
```

contract_address赋值为区块上的费用选举合约地址，exe_or_query 为false代表查询


- 如果查询到则返回内容:

```json
{
    "error_code": 0,
    "error_desc": "",
    "result": {
        "logs": {
            "0-buQebeTXVPA8mTt2fmBi51GifPbsqDPPURK1": null
        },
        "query_rets": [
            {
                "result": {
                    "type": "string",
                    "value": "{\"buQft4EdxHrtatWUXjTFD7xAbMXACnUyT8vw\":1}"
                }
            }
        ],
        "real_fee": 0,
        "stat": {
            "apply_time": 18020,
            "memory_usage": 1326456,
            "step": 19
        },
        "txs": null
    }
}
```
result 域的value值为返回结果，反序列化为json格式即可得到所有的某项费用提案的投票

#### 费用提案

通过发送转移资产交易或者付币交易来给合约发起费用提案。合约入参input参数json格式。一个账户只能发起一类费用提案，如果再次发起提案，会销毁上次提案及相关投票。

```json
{
  "method":"proposalFee",
    "params":{
        "feeType": 1, //费用种类
        "price": "5"    //费用价格
    }
}
```

json格式需转换成字符串形式填写到paycoin接口结构

```json
{
    "type" : "PAY_COIN",
    "pay_coin" : {
       "dest_address" :"buQebeTXVPA8mTt2fmBi51GifPbsqDPPURK1",
       "amount":0,
        "input":"{\"method\":\"proposalFee\",\"params\":{\"feeType\":1,\"price\":\"5\"}}";
    }
}
```

#### 费用选取

通过发送转移资产交易或者付币交易来给合约发起费用选举。合约入参input参数json格式

```json
{
  "method":"voteFee",
  "params":{
      "proposalId": "buQft4EdxHrtatWUXjTFD7xAbMXACnUyT8vw1"
   }
}
```

json格式需转换成字符串形式填写到paycoin接口结构

```json
{
   "type" : "PAY_COIN",
    "pay_coin" : {
        "dest_address" :"buQebeTXVPA8mTt2fmBi51GifPbsqDPPURK1",
        "amount":0,
        "input":"{\"method\":\"voteFee\",\"params\":{\"proposalId\":\"buQft4EdxHrtatWUXjTFD7xAbMXACnUyT8vw1\"}}"
    }
}
```

## 错误码

  错误由两部分构成:
- error_code : 错误码，大概的错误分类
- error_desc : 错误描述，一般能从错误描述准确发现错误具体信息

错误列表如下:

| error_code/错误码 | 枚举名                                 | 错误描述                                                                                     |
| :---------------- | -------------------------------------- | -------------------------------------------------------------------------------------------- |
| 0                 | ERRCODE_SUCCESS                        | 操作成功                                                                                     |
| 1                 | ERRCODE_INTERNAL_ERROR                 | 服务内部错误                                                                                 |
| 2                 | ERRCODE_INVALID_PARAMETER              | 参数错误                                                                                     |
| 3                 | ERRCODE_ALREADY_EXIST                  | 对象已存在， 如重复提交交易                                                                  |
| 4                 | ERRCODE_NOT_EXIST                      | 对象不存在，如查询不到账号、TX、区块等                                                       |
| 5                 | ERRCODE_TX_TIMEOUT                     | TX 超时，指该 TX 已经被当前节点从 TX 缓存队列去掉，**但并不代表这个一定不能被执行**          |
| 20                | ERRCODE_EXPR_CONDITION_RESULT_FALSE    | 指表达式执行结果为 false，意味着该 TX 当前没有执行成功，**但这并不代表在以后的区块不能成功** |
| 21                | ERRCODE_EXPR_CONDITION_SYNTAX_ERROR    | 指表达式语法分析错误，代表该 TX 一定会失败                                                   |
| 90                | ERRCODE_INVALID_PUBKEY                 | 公钥非法                                                                                     |
| 91                | ERRCODE_INVALID_PRIKEY                 | 私钥非法                                                                                     |
| 92                | ERRCODE_ASSET_INVALID                  | 资产issue 地址非法                                                                           |  |  |
| 93                | ERRCODE_INVALID_SIGNATURE              | 签名权重不够，达不到操作的门限值                                                             |
| 94                | ERRCODE_INVALID_ADDRESS                | 地址非法                                                                                     |
| 97                | ERRCODE_MISSING_OPERATIONS             | 交易缺失操作                                                                                 |
| 99                | ERRCODE_BAD_SEQUENCE                   | 交易序号错误，nonce错误                                                                      |
| 100               | ERRCODE_ACCOUNT_LOW_RESERVE            | 余额不足                                                                                     |
| 101               | ERRCODE_ACCOUNT_SOURCEDEST_EQUAL       | 源和目的账号相等                                                                             |
| 102               | ERRCODE_ACCOUNT_DEST_EXIST             | 创建账号操作，目标账号已存在                                                                 |
| 103               | ERRCODE_ACCOUNT_NOT_EXIST              | 账户不存在                                                                                   |
| 104               | ERRCODE_ACCOUNT_ASSET_LOW_RESERVE      | 支付操作，资产余额不足                                                                       |
| 105               | ERRCODE_ACCOUNT_ASSET_AMOUNT_TOO_LARGE | 资产数量过大，超出了int64的范围                                                              |
| 114               | ERRCODE_OUT_OF_TXCACHE                 | TX 缓存队列已满                                                                              |
| 120               | ERRCODE_WEIGHT_NOT_VALID               | 权重值不在有效范围内                                                                         |
| 121               | ERRCODE_THRESHOLD_NOT_VALID            | 门限值不在有效范围内                                                                         |
| 144               | ERRCODE_INVALID_DATAVERSION            | metadata的version版本号不与已有的匹配（一个版本化的数据库）                                  |
| 151               | ERRCODE_CONTRACT_EXECUTE_FAIL          | 合约执行失败                                                                                 |
| 152               | ERRCODE_CONTRACT_SYNTAX_ERROR          | 合约语法分析失败                                                                             |

## 示例

下面我们用`buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3`发起一笔交易，这笔交易只有1个操作:创建一个账号。

1. 组装交易，将交易序列化

    ```text
    POST getTransactionBlob
    ```

    ```json
    {
      "source_address": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3",
      "nonce": 1,
      "fee":1000,
      "operations": [{
          "type": 1,
          "create_account": {
            "dest_address": "buQts6DfT5KavtV94JgZy75H9piwmb7KoUWg",
            "metadatas": [{
                "key": "hello",
                "value": "这是创建账号的过程中设置的一个metadata"
              }
            ],
            "priv": {
              "thresholds": {
                "tx_threshold": 1
              }
            },
            "contract": {
              "payload": "function main(inputStr){\n /*这是合约入口函数*/ }"
            }
          }
        }
      ]
    }
    ```
    得到结果

    ```json
    {
        "error_code": 0,
        "error_desc": "",
        "result": {
            "hash": "8e97ab885685d68b8fa8c7682f77ce17a85f1b4f6c8438eda8ec955890919405",
            "transaction_blob": "0a2e61303032643833343562383964633334613537353734656234393736333566663132356133373939666537376236100122b90108012ab4010a2e61303032663836366337663431356537313934613932363131386363353565346365393939656232396231363461123a123866756e6374696f6e206d61696e28696e707574537472297b0a202f2ae8bf99e698afe59088e7baa6e585a5e58fa3e587bde695b02a2f207d1a06080a1a020807223e0a0568656c6c6f1235e8bf99e698afe5889be5bbbae8b4a6e58fb7e79a84e8bf87e7a88be4b8ade8aebee7bdaee79a84e4b880e4b8aa6d65746164617461"
        }
    }
    ```

1. 我们用对`buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3`的私钥对`transaction_blob`的值进行签名。得到签名数据
 `2f6612eaefbdadbe792201bb5d1e178aff118dfa0a640edb2a8ee91933efb97c4fb7f97be75195e529609a4de9b890b743124970d6bd7072b7029cfe7683ba2d`。

1. 将签名数据和公钥和交易blob一起提交
    ```text
    POST /submitTransaction
    ```

    ```json
    {
      "items" : [{
          "transaction_blob" : "0a2e61303032643833343562383964633334613537353734656234393736333566663132356133373939666537376236100122b90108012ab4010a2e61303032663836366337663431356537313934613932363131386363353565346365393939656232396231363461123a123866756e6374696f6e206d61696e28696e707574537472297b0a202f2ae8bf99e698afe59088e7baa6e585a5e58fa3e587bde695b02a2f207d1a06080a1a020807223e0a0568656c6c6f1235e8bf99e698afe5889be5bbbae8b4a6e58fb7e79a84e8bf87e7a88be4b8ade8aebee7bdaee79a84e4b880e4b8aa6d65746164617461",
          "signatures" : [{
              "sign_data" : "2f6612eaefbdadbe792201bb5d1e178aff118dfa0a640edb2a8ee91933efb97c4fb7f97be75195e529609a4de9b890b743124970d6bd7072b7029cfe7683ba2d",
              "public_key" : "b00204e1c7dddc36d3153adcaa451b0ab525d3def48a0a10fdb492dc3a7263cfb88e80ee974ca4da0e1f322aa84ff9d11340c764ea756ad148e979c121619e9fe52e9054"
            }
          ]
        }
      ]
    }
    ```
