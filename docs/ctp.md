# Bumo CTP1.0 Token 标准

## 简介

CTP1.0(Contract Token Protocol) 指基于 BUMO 合约发行 token 的标准协议。该协议提供了转移 token 的基本功能，并允许 token 授权给第三方使用。

## 目标

标准接口可以让 Bumo 上的任何 token 被其他应用程序使用，比如钱包和交易所。


## 规则

Bumo 智能合约由 javascript 实现,包含初始化函数 init 和两个入口函数 main、query 。init 函数用于合约创建时初始化、main 函数主要负责数据写入，query 函数负责数据查询。


## 智能合约变量

| 变量        | 描述                    |  
| :----------- | --------------------------- |
|ctp           | Contract Token Protocol版本 |
|name          | Token 名称                  |
|symbol        | Token 符号                  |
|decimals      | Token 小数位数              |
|totalSupply   | Token 总量                  |
|contractOwner | Token 所有者                |

## 函数

### contractInfo

返回 Token 的基本信息。入口函数 query。

E.g.

- 参数json结构:
```json
{
    "method":"contractInfo"
}
```
- 函数：function contractInfo()
- 返回值：
```json
{
    "result":{
		"type": "string",
		"value": {
			"contractInfo": {
				"ctp": "1.0.0.1",
				"name": "cccpt-bu",
				"symbol": "CBG",
				"decimals": 0,
				"totalSupply": "100000",
				"contractOwner": "buQBv4pqtNMs6ueBhx7mJULhAFYV3rSHo2Zg",
				"balance": "100000"
			}
		}
    }
} 
```

### name

返回 token 的名称。入口函数 query。

E.g.

- 参数json结构:
```json
{
    "method":"name"
}
```
- 函数：function name()
- 返回值：
```json
{
    "result":{
        "name":"XXXCOIN"
    }
} 
```

### symbol

返回 token 的符号。入口函数 query。

E.g.

- 参数json结构:
```json
{
    "method":"symbol"
}
```
- 函数：function symbol()
- 返回值：
```json
{
    "result":{
        "symbol":"XXX"
    }
} 
```

### decimals

返回 token 使用的小数点后几位， 比如 5,表示分配 token 数量为100000。入口函数query。

E.g.

- 参数json结构:
```json
{
    "method":"decimals"
}
```
- 函数：function decimals()
- 返回值：
```json
{
    "result":{
        "decimals":5
    }
} 
```

### totalSupply

返回 token 的总供应量。入口函数 query。

E.g.

- 参数 json 结构:
```json
{
    "method":"totalSupply"
}
```
- 函数：function totalSupply()
- 返回值：
```json
{
    "result":{
        "totalSupply":"10000000000000000000"
    }
} 
```

### balanceOf

返回 owner 账户的账户余额。入口函数 query。

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
- 返回值：
```json
{
    "result":{
        "balanceOf":"100000000000000",
    }
} 
```

### transfer

转移 value 的token数量到的地址 to，并且必须触发 log 事件。 如果资金转出账户余额没有足够的token来支出，该函数应该被throw, from 为发送交易的账户地址。入口函数 main。

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

### transferFrom

从地址from发送数量为 value 的token到地址 to，必须触发 log 事件。 在 transferFrom 之前，from 必须已经调用过 approve 向 to 授权了额度。如果 from 账户余额没有足够的token来支出或者 from 授权给 to 的额度不足，该函数应该被 throw。入口函数 main。

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


### approve

授权账户 spender 可以从交易发送者账户转出数量为 value 的token。入口函数 main。

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

### assign

合约 token 拥有者向 to 分配数量为 value 的 token。入口函数 main。

参数json结构:
```json
{
    "method":"assign",
    "params":{
        "to":"buQnTmK9iBFHyG2oLce7vcejPQ1g5xLVycsj",
        "value":"1000000"
    }
}
```
参数：to 收账账户地址；
参数：value 分配数量（字符串类型）

- 函数：function assign(to, value)
- 返回值：true 或者抛异常

### changeOwner

将合约 token 拥有权转移给 address，只有合约 token 拥有者才能执行此权限，入口函数 main。

参数 json 结构:
```json
{
    "method":"changeOwner",
    "params":{
        "address":"buQnTmK9iBFHyG2oLce7vcejPQ1g5xLVycsj"
    }
}
```
参数：address 账户地址；

- 函数：function changeOwner(address)
- 返回值：true或者抛异常

### allowance

返回 spender 仍然被允许从 owner 提取的金额。入口函数 query。

参数json结构:
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


## 入口函数

### 入口函数 init

```js
function init(input_str){
}

```

参数 json 结构:
```json
{
    "params":{
        "name":"RMB",
        "symbol":"CNY",
        "decimals":8,
        "totalSupply":"1500000000",
        "contractOwner":"buQnTmK9iBFHyG2oLce7vcejPQ1g5xLVycsj",
        "ctp":"1.0.0.0"
    }
}
```
参数：name 资产名称；
参数：symbol 资产符号；
参数：decimals 小数位数；
参数：totalSupply 发型总量；
参数：contractOwner 合约资产归属人；
参数：ctp 合约资产协议(Contract Token Protocol)版本。

- 返回值：true或者抛异常

### 入口函数 main

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
    else if(input.method === 'assign'){
        assign(input.params.to, input.params.value);
    }
    else if(input.method === 'changeOwner'){
        changeOwner(input.params.address);
    }
    else{
        throw '<undidentified operation type>';
    }
}
```

### 入口函数query

```js
function query(input_str){
    loadGlobalAttribute();

    let result = {};
    let input  = JSON.parse(input_str);
    if(input.method === 'name'){
        result.name = name();
    }
    else if(input.method === 'symbol'){
        result.symbol = symbol();
    }
    else if(input.method === 'decimals'){
        result.decimals = decimals();
    }
    else if(input.method === 'totalSupply'){
        result.totalSupply = totalSupply();
    }
    else if(input.method === 'contractInfo'){
        result.contractInfo = contractInfo();
    }
    else if(input.method === 'balanceOf'){
        result.balance = balanceOf(input.params.address);
    }
    else if(input.method === 'allowance'){
        result.allowance = allowance(input.params.owner, input.params.spender);
    }
    else{
       	throw '<unidentified operation type>';
    }

    log(result);
    return JSON.stringify(result);
}
```
