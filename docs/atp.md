# BUMO ATP1.0协议 标准

## 简介

ATP1.0(Account Token Protocol) 指基于 BUMO 发行资产、转移资产和增发资产的标准协议。  
发行资产：账户发行一笔数字资产，执行成功后账户的资产余额中会出现这一笔资产。  
转移资产：账户将一笔资产转给目标账户。  
增发资产：账户继续在原资产代码上发行一定数量的资产，执行成功后账户的资产余额会增加。
Token在此文代表资产。
## 目标

标准协议可以让其它应用程序方便地调用sdk接口在 BUMO 上进行token的发行、转移和增发操作。


## 资产属性参数
发行的token需要通过设置metadata来标记token的相关属性。用于应用程序方便去管理和查询token数据信息
| 变量        | 描述                    |  
| :----------- | --------------------------- |
|name          | Token 名称                 |
|code          | Token 代码                  |
|description   | Token 描述                  |
|decimals      | Token 小数位数              |
|totalSupply   | Token 总量                  |
|icon          | Token 图标                  |
|version       | Account Token Protocol版本                |



## 过程

### 发行token  
1.客户端通过SDK发起一笔操作类型是‘发行资产’的交易。设置参数amount(发行的数量)、code(资产代码)。  
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

2.接着继续发送‘设置metadata’的交易，设置资产metadata参数key、value和version。参见：  
- json格式

    ```JSON
    {
      "type": 4,
      "set_metadata": {
        "key": "pre_DT",
        "value": {
            "name":"DemonToken",
            "code":"DT",
            "totalSupply":"10000",
            "decimals":8,
            "description":"This is hello token",
            "icon":"xxxxxxxxxxxxxx",
            "version":"1.0"
        },
        "version": 0
      }
    }
    ```



### 转移token  
1.设置参数，发送‘转移资产’的交易
|参数|描述
|:--- | --- 
|pay_asset.dest_address |  目标账户
|pay_asset.asset.key.issuer|  资产发行方
|pay_asset.asset.key.code|  资产代码
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
### 增发token  
通过设置和之前‘发行token’相同的资产代码，继续发送[发行token](###发行token)的交易，进行资产增发。增发成功后可以看到资产数量会有所增加。






