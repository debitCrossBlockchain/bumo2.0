English | [中文](README_CN.md)

# Monitor 监控

## 功能介绍
该模块实现了 监控 区块链节点状态，节点信息及异常告警功能，具体如下：
- 区块链节点状态。包括节点的在线和离线状态。
- 节点信息。包括系统信息，区块信息，交易信息，用户信息。
- 异常告警。当发现节点状态或节点系统状态异常时，提供实时告警。包括离线告警、CPU告警、内存告警、磁盘告警、区块不同步告警

## 模块结构

类名称 | 声明文件 | 功能
|:--- | --- | ---
| `MonitorManager` | [monitor_manager.h](./monitor_manager.h) | 监控模块的管理者，继承自 `Network` 类。该类有三个功能：一是管理节点连接；二是处理接收到的消息，包括握手消息，注册消息，节点状态消息，区块状态消息，节点系统消息和心跳信息；三是提供监控重连和异常检测的定时器。
|`Monitor`|  [monitor.h](./monitor.h) | 保存节点的监控状态，包括监控Session，是否在线，监控Id，监控节点的版本信息等。
|`SystemManager`|  [system_manager.h](../common/system_manager.h)  | 节点系统信息的管理者。提供CPU的定时器和系统监控信息的获取。

## 协议定义
监控 消息使用 Google protocol buffer 定义，参考文件 [monitor.proto](../proto/monitor.proto)，类型如下：
```
MONITOR_MSGTYPE_HELLO  #握手消息，发送监控ID
MONITOR_MSGTYPE_REGISTER  #注册消息，验证监控ID
MONITOR_MSGTYPE_BUMO   #获取节点信息
MONITOR_MSGTYPE_LEDGER    #获取区块信息
MONITOR_MSGTYPE_SYSTEM   #获取节点系统信息
MONITOR_MSGTYPE_ALERT #告警信息
```
