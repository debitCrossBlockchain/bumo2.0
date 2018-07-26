# main

## 基本介绍
主程序。bumo 进程入口，启动后加载配置文件，启动主线程，初始化所有模块，启动定时器。

## 模块结构
名称 | 实现文件 | 功能
|:--- | --- | ---
| `main` | [main.cpp](./main.cpp) | 主程序，进程入口。启动过程参考 [启动过程](##启动过程) 章节
| `Configure` | [configure.h](./configure.h) | 解析主程序的配置文件，继承自 `ConfigureBase` 类，实现了对所有模块配置文件的加载功能。如数据库、日志、WebServer、WebSocketServer、P2P、账本、创世区块、监控等。


## 启动过程

主程序启动后做了以下几件事情：
- 解析参数。判断参数是否有指令含义，如果有则进行对应的指令操作。参考文件 [argument.h](../common/argument.h)
- 初始化所有模块。如 `net`， `Timer`， `Configure`， `Storage`， `Global`， `SlowTimer`， `Logger`， `Console`， `PeerManager`， `LedgerManager`， `ConsensusManager`， `GlueManager`， `WebSocketServer`， `WebServer`， `MonitorManager`， `ContractManager`。
- 如果是 Linux 版本的程序，则启动 `Daemon` 模块，用于向共享内存写时间戳，供守护进程监控使用。
- 将自己作为主线程，并启动定时器调度。