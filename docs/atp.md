# BUMO ATP1.0协议 标准

## 简介

ATP1.0(Account based Tokenization Protocol) 指基于 BuChain的账号结构对资产进行发行、转移和增发Token的标准协议，Token在此文代表账号资产。  
发行Token：账户发行一笔数字Token，执行成功后账户的Token余额中会出现这一笔Token。  
转移Token：账户将一笔Token转给目标账户。  
增发Token：账户继续在原Token代码上发行一定数量的Token，执行成功后账户的Token余额会增加。  
查询Token：查询源账户的Token信息。

## 目标

标准协议可以让其它应用程序方便地调用sdk接口在 BUMO 上进行Token的发行、转移和增发操作。


## Token属性参数
发行的Token需要通过设置Token源账户的metadata来记录Token的相关属性。用于应用程序方便去管理和查询Token数据信息。  

| 变量        | 描述                    |  
| :----------- | --------------------------- |
|name          | Token 名称                 |
|code          | Token 代码                  |
|description   | Token 描述                  |
|decimals      | Token 小数位数              |
|totalSupply   | Token 总量                  |
|icon          | Token 图标                  |
|version       | ATP 版本                |



## 过程

### 发行Token  
1.客户端通过SDK发起一笔操作类型是`Issuing Assets`的交易。设置参数amount(发行的数量)、code(Token代码)。  
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

2.接着继续发送`Setting Metadata`的交易，设置Token metadata参数key、value和version。参见：  
- json格式

    ```JSON
    {
      "type": 4,
      "set_metadata": {
        "key": "pre_DT",
        "value": {
            "name":"DemonToken",
            "code":"DT",
            "totalSupply":"10000",//0~2^63-1。0表示不固定Token的上限
            "decimals":8,
            "description":"This is hello Token",
            "icon":"xxxxxxxxxxxxxx",
            "version":"1.0"
        },
        "version": 0
      }
    }
    ```



### 转移Token  
1.设置参数，发送`Transferring Assets`的交易。  

|参数|描述
|:--- | --- 
|pay_asset.dest_address |  目标账户
|pay_asset.asset.key.issuer|  Token发行方
|pay_asset.asset.key.code|  Token代码
|pay_asset.asset.amount|  要转移的数量
|pay_asset.input|  触发合约调用的入参，如果用户未输入，默认为空字符串  
json格式：
```JSON
    {
      "type": 3,
      "pay_asset": {
        "dest_address": "buQgmhhxLwhdUvcWijzxumUHaNqZtJpWvNsf",
        "asset": {
          "key": {
            "issuer": "buQgmhhxLwhdUvcWijzxumUHaNqZtJpWvNsf",
            "code": "CNY",
            "type": 0 //目前只能填0 或者不填
          },
          "amount": 100
        }
      }
    }
  ```
### 增发Token  
通过设置和之前`发行Token`相同的交易类型代码，继续发送[发行Token](#发行Token)的交易，进行Token增发。应用程序根据具体业务去控制增发Token数量是否超过totalSupply，增发成功后可以看到Token数量会有所增加。  
### 查询Token  
通过getAccount接口可以返回指定账号的信息及其所有Token、metadata。  

`HTTP GET /getAccount?address=buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3&key=hello&code=xxx&issuer=xxx`  

| 参数         | 描述                                                                                                                                                    |
| :----------- | ------------------------------------------------------------------------------------------------------------------------------------------------------- |
| address      | 账号地址， 必填                                                                                                                                         |
| key          | 账号的 metadata 中指定的key的值，如果不填写，那么返回结果中含有所有的metadata                                                                           |
| code, issuer,type | Token代码,Token发行商。这两个变量要么同时填写，要么同时不填写。若不填写，返回的结果中包含所有的Token。若填写，返回的结果中只显示由code和issuer,type指定的Token。目前type只能是0，可以不用填写 |  
返回内容如下例子：

```json
{
  "error_code" : 0,
  "result" : {
    "address" : "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3", //该账号的地址
    "assets" : [//该账号的所有Token
      {
        "amount" : 1400,
        "key" :
        {
          "code" : "CNY",
          "issuer" : "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3"
        }
      }, {
        "amount" : 1000,
        "key" :
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
        "key": "pre_DT",
        "value": {
            "name":"DemonToken",
            "code":"DT",
            "totalSupply":"10000",
            "decimals":8,
            "description":"This is hello Token",
            "icon":"xxxxxxxxxxxxxx",
            "version":"1.0"
        },
        "version": 0
      }
    ],
    "nonce" : 1, //账号当前作为交易源执行过的交易数量。若nonce为0，该字段不显示
    "priv" : {
      "master_weight" : 1,
      "thresholds" : {
        "tx_threshold" : 1
      }
    },
    "metadatas_hash" : "82c8407cc7cd77897be3100c47ed9d43ec4097ee1c00e2c13447187e5b1ac66c"
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






