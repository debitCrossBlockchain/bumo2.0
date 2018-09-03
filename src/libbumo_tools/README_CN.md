[English](README.md) | 中文

# libbumo_tools

## 功能介绍
bumo 工具库，以三方库的方式提供，实现钱包控制、账号管理和交易签名等功能。有如下特性：
- 跨平台。c++ 实现，封装成三方库后供开发者使用。
- 轻量级工具。不含节点数据同步功能。
- 操作安全。使用过程中，不会泄露私钥信息。

## 模块结构

模块名称 | 声明文件 | 功能
|:--- | --- | ---
| `libbumo_tools` | [lib_bumo_tools.h](./lib_bumo_tools.h) | 以三方库的方式实现钱包控制、账号管理和交易签名等功能。
## 接口列表

代码中实现的接口包括但不限于如下
```
CreateAccountAddress   #创建公私钥对
CheckAccountAddressValid    #检测账号是否正确
CreateKeystore  #创建私钥存储器
CheckKeystoreValid  #检测私钥存储器是否正确
SignData    #使用私钥签名
SignDataWithKeystore    #使用私钥存储器签名
CheckSignedData     #检测签名数据
CreateKeystoreFromPrivkey   #使用私钥创建一个私钥存储器
GetAddressFromPubkey    #从公钥地址中获取账号地址和原始公钥地址
GetPrivatekeyFromKeystore   #从私钥存储器里获取私钥
```


