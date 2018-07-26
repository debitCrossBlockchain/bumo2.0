# Utils

## 基本介绍
Utils 模块实现了工程常用的工具集功能，同时屏蔽平台底层差异，基本功能如下：
- 获取时间戳。
- 线程管理。
- 系统状态管理。
- 字符串操作。
- `SM2、SM3、Base58` 等加解密
- 单实例类
- 线程锁
- 随机数
- 无拷贝构造函数基础类
- 写日志
- 读写文件
- 原子操作

## 模块结构
类名称 | 声明文件 | 功能
|:--- | --- | ---
| `utils` | [utils.h](./utils.h) | 实现了以下功能：一是定义了时间、字节宏的全局静态变量；二是实现了原子的加减函数；三是 `ObjectExit` 类，用于处理批量对象退出使用。以及其他小的函数功能：如 cpu 核数，sleep 函数，开机时间等。
| `Timestamp` | [timestamp.h](./timestamp.h) | 时间戳工具类。跨平台获取系统的时间戳，精确到微妙。
| `Timer` | [timer.h](./timer.h) | 定时器工具类。可以在设置的时间内定时执行某个指定函数。
| `Thread` | [thread.h](./thread.h) | 跨平台的线程工具类。实现了 `ThreadPool`线程池、`Mutex` 线程锁等
| `System` | [system.h](./system.h) | 跨平台的系统工具类。实现了查询硬盘，内存，主机名称，系统版本，日志大小，开机时间，cpu，硬件地址等功能。
| `String` | [strings.h](./strings.h) | 字符串处理类。实现字符串格式化，去空格，转数字，转二进制等多种操作
| `Sm3` | [sm3.h](./sm3.h) | SM3 加密算法的实现。
| `Singleton` | [singleton.h](./singleton.h) | 单实例模板类。保证该类的继承者为一个单实例类。
| `random` 相关| [random.h](./random.h) | 获取随机的字节。
| `NonCopyable` | [noncopyable.h](./noncopyable.h) | 无拷贝构造函数和赋值函数的基类。
| `Logger` | [logger.h](./logger.h) | 日志操作类。提供多种方式输出日志，如文件、控制台，同时提供不同级别的日志输出，如`NONE，TRACE，DEBUG，INFO，WARN，ERROR，FATAL，ALL`。日志支持自动管理，如超过指定大小或日期则新生成文件，定时清理过期的日志等。
| `File` | [file.h](./file.h) | 跨平台的文件读写工具类。实现了文件读写，目录操作等功能。
| `EccSm2` | [ecc_sm2.h](./ecc_sm2.h) | 实现了 SM2 算法
| `crypto` 相关 | [crypto.h](./crypto.h) | 加密库的合集。如`Base58，Sha256，MD5，Aes`
| `AtomMap` | [atom_map.h](./atom_map.h) | 保证批量的 Key-Value 数据操作具有原子性。
| `uint128_t` | [base_int.h](./base_int.h) | 对大数运算的封装操作类

