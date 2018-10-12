- [Bumo CTP1.0 Token 标准](#bumo-ctp1.0-token-标准)
    - [简介](#简介)
    - [目标](#目标)
    - [规则](#规则)
    - [Token 属性](#token-属性)
    - [功能函数](#功能函数)
         - [transfer](#transfer)
         - [approve](#approve)
         - [transferFrom](#transferfrom)
         - [tokenInfo](#tokeninfo)
         - [allowance](#allowance)
         
    - [合约入口](#合约入口)
        - [init](#init)
        - [main](#main)
        - [query](#query)


# Bumo CTP1.0 Token 标准

## 简介

CTP 1.0(Contract Token Protocol) 指基于 BUMO 智能合约发行 token 的标准协议。该协议提供了转移 token 的基本功能，并允许 token 授权给第三方使用。

## 目标

基于这套标准接口，可以让发行的 token 被其他应用程序和第三方快速对接和使用，比如钱包和交易所。


## 规则

Bumo 智能合约由 JavaScript 语言实现, 包含初始化函数 init 和两个入口函数 main、query 。init 函数用于合约创建时初始化; main 函数主要负责数据写入，query 函数负责数据查询。


## Token 属性

Token 属性可以通过合约的 `tokenInfo` 功能函数查询到，存储在智能合约的账号里。包含以下内容

| 变量         | 描述                     |  
| :----------- | --------------------------- |
|name          | Token 名称                  |
|symbol        | Token 符号                  |
|decimals      | Token 小数位数              |
|totalSupply   | Token 总量，totalSupply = Token 个数(supply) * 10 ^ decimals |
|version       | Contract Token Protocol版本 |

注意：

- name：推荐使用单词全拼，每个首字母大写。如 Demo Token
- symbol：推荐使用大写首字母缩写。如 DT
- decimals：小数位在 0~8 的范围，0 表示无小数位
- totalSupply：范围是 1~2^63-1
- version：ctp 的版本。如 1.0

## 功能函数

### transfer

- 转移 value 数量的 token 到的地址 to，并且必须触发 log 事件。 如果资金转出账户余额没有足够的token来支出，该函数应该被throw。
- 入口函数 main。

- 参数 json 结构:
```json
{
    "method":"transfer",
    "params":{
        "to":"buQnTmK9iBFHyG2oLce7vcejPQ1g5xLVycsj",
        "value":"1000000"
    }
}
```
参数：to目标账户地址；
参数：value转移数量（字符串类型）

- 函数：function transfer(to, value)
- 返回值：true或者抛异常

### approve

- 授权账户 spender 可以从交易发送者账户转出数量为 value 的token。
- 入口函数 main。

参数json结构:
```json
{
    "method":"approve",
    "params":{
        "spender":"buQnTmK9iBFHyG2oLce7vcejPQ1g5xLVycsj",
        "value":"1000000"
    }
}
```
参数：spender 账户地址；
参数：value 被授权可转移数量（字符串类型）

- 函数：function approve(spender, value)
- 返回值：true 或者抛异常

### transferFrom

- 从from发送数量为 value 的 token 到 to，必须触发 log 事件。 在 transferFrom 之前，from 必须给当前交易的发起者进行授权额度(即approve操作)。如果 from 余额没有足够的 token 来支出或者 from 授权给当前交易的发起者的额度不足，该函数应该被 throw。
- 入口函数 main。

参数json结构:
```json
{
    "method":"transferFrom",
    "params":{
        "from":"buQnTmK9iBFHyG2oLce7vcejPQ1g5xLVycsj",
        "to":"buQYH2VeL87svMuj2TdhgmoH9wSmcqrfBner",
        "value":"1000000"
    }
}
```
参数：from 源账户地址；
参数：to 目标账户地址；
参数：value 转移数量（字符串类型）

- 函数：function transferFrom(from,to,value)
- 返回值：true或者抛异常

### balanceOf

- 返回指定账户的 token
- 入口函数 query。

- 参数json结构:
```json
{
    "method":"balanceOf",
    "params":{
        "address":"buQnTmK9iBFHyG2oLce7vcejPQ1g5xLVycsj"
    }
}
```
参数：address 账户地址

- 函数：function balanceOf(owner)
- 返回值：指定地址的余额
```json
{
    "result":{
        "balanceOf":"100000000000000",
    }
} 
```

### tokenInfo

- 返回 Token 的基本信息。
- 入口函数 query。

- 参数json结构:
```json
{
    "method":"tokenInfo"
}
```
- 函数：function tokenInfo()
- 返回值：
```json
{
    "result":{
        "type": "string",
        "value": {
            "tokenInfo": {
                "name": "DemoToken",
                "symbol": "DT",
                "decimals": 8,
                "totalSupply": "5000000000000",
                "version": "1.0"
            }
        }
    }
} 
```

### allowance

- 返回 spender 仍然被允许从 owner 提取的金额。
- 入口函数 query。

- 参数json结构:
```json
{
    "method":"allowance",
    "params":{
        "owner":"buQnTmK9iBFHyG2oLce7vcejPQ1g5xLVycsj",
        "spender":"buQYH2VeL87svMuj2TdhgmoH9wSmcqrfBner"
    }
}
```
参数：owner 账户地址；
参数：spender 账户地址；

- 函数：function allowance(owner, spender)
- 返回值：
```json
{
    "result":{
        "allowance":"1000000",
    }
} 
```


## 合约入口

### init

```js
function init(input_str){
}

```

创建合约时候，触发合约 `init` 入口函数，传递 `JSON` 参数格式如下：

```json
{
    "params":{
        "name":"DemoToken",
        "symbol":"DT",
        "decimals":8,
        "supply":"50000",
        "version": "1.0"
    }
}
```
- name: token 名称
- symbol: 资产符号
- decimals: 小数位数
- supply: 字符串格式，发行Token 个数(整数部分)。例如发行 50000 个 Token，其 totalSupply 总量为 50000 * 100000000
- version: 版本号

入口函数的返回值：true或者抛异常


### main

```js
function main(input_str){
    let input = JSON.parse(input_str);

    if(input.method === 'transfer'){
        transfer(input.params.to, input.params.value);
    }
    else if(input.method === 'transferFrom'){
        transferFrom(input.params.from, input.params.to, input.params.value);
    }
    else if(input.method === 'approve'){
        approve(input.params.spender, input.params.value);
    }
    else{
        throw '<Main interface passes an invalid operation type>';
    }
}
```

### query

```js
function query(input_str){
    globalAttribute = JSON.parse(storageLoad(globalAttributeKey));

    let result = {};
    let input  = JSON.parse(input_str);

    if(input.method === 'tokenInfo'){
        result.tokenInfo = globalAttribute;
    }
    else if(input.method === 'allowance'){
        result.allowance = allowance(input.params.owner, input.params.spender);
    }
    else{
       	throw '<Query interface passes an invalid operation type>';
    }
    return JSON.stringify(result);
}
```
