# Bumo BTP1.0 代币标准

## 简介

代币的接口标准。

## 抽象

BTP1.0(Bumo Token Protocal) 允许在智能合约中实现的标准API。该标准提供了转移代币的基本功能，并允许代币被批准，以便他们可以由另一个在线第三方使用。

## 目标

标准接口可以让Bumo上的任何代币被其他应用程序重新使用：从钱包到去中心化交易所。


## 规则

bumo智能合约由javascript实现,包含两个入口函数main和query。main函数主要负责数据修改，query函数负责查询数据。


## 智能合约变量

| 变量        | 描述                    |  
| :--------- | ------------------------ |
|name        | 代币 名称                |
|symbol      | 代币 符号                |
|decimals    | 代币 小数位数             |
|totalSupply | 代币 总量                |


## 函数

### contractInfo

返回代币的基本信息。入口函数query。

E.g.

- 参数json结构:
```json
{
    "method":"contractInfo",
    "params":""
}
```
- 函数：function contractInfo()
- 返回值：
```json
{
    "result":{
        "symbol":"XXX",
        "decimals":5,
        "totalSupply":"10000000000000000000",
        "name":"XXXCOIN",
    }
} 
```

### name

返回代币的名称。入口函数query。

E.g.

- 参数json结构:
```json
{
    "method":"name",
    "params":""
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

返回代币的符号。入口函数query。

E.g.

- 参数json结构:
```json
{
    "method":"symbol",
    "params":""
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

返回token使用的小数点后几位， 比如 5,表示分配token数量为100000。入口函数query。

E.g.

- 参数json结构:
```json
{
    "method":"decimals",
    "params":""
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

返回代币的总供应量。入口函数query。

E.g.

- 参数json结构:
```json
{
    "method":"totalSupply",
    "params":""
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

返回owner账户的账户余额。入口函数query。

- 参数json结构:
```json
{
    "method":"balanceOf",
    "params":{
        "address":"buQnTmK9iBFHyG2oLce7vcejPQ1g5xLVycsj"
    }
}
```
参数：address账户地址

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

转移value的代币数量到的地址to，并且必须触发log事件。 如果from帐户余额没有足够的代币来支出，该函数应该被throw,from为发送交易的账户地址。入口函数main。

- 参数json结构:
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

从地址from发送数量为value的代币到地址to，必须触发log事件。 在transferFrom之前，form必须已经调用过approve向to授权了额度。如果from帐户余额没有足够的代币来支出或者from授权给to的额度不足，该函数应该被throw。入口函数main。

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
参数：from源账户地址；
参数：to目标账户地址；
参数：value转移数量（字符串类型）

- 函数：function transferFrom(from,to,value)
- 返回值：true或者抛异常


### approve

授权账户spender可以从交易发送者账户转出数量为value的代币。入口函数main。

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
参数：spender账户地址；
参数：value被授权可转移数量（字符串类型）

- 函数：function approve(spender, value)
- 返回值：true或者抛异常

### assign

合约代币拥有者向to分配数量为value的代币。入口函数main。

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
参数：to收账账户地址；
参数：value分配数量（字符串类型）

- 函数：function assign(to, value)
- 返回值：true或者抛异常

### changeOwner

将合约代币拥有权转移给address，只有合约代币拥有者才能执行此权限，入口函数main。

参数json结构:
```json
{
    "method":"changeOwner",
    "params":{
        "address":"buQnTmK9iBFHyG2oLce7vcejPQ1g5xLVycsj"
    }
}
```
参数：address账户地址；

- 函数：function changeOwner(address)
- 返回值：true或者抛异常

### allowance

返回spender仍然被允许从owner提取的金额。入口函数query。

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
参数：owner账户地址；
参数：spender账户地址；

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

入口函数必须严格按照以下规范进入个执行函数。

### 入口函数main

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
