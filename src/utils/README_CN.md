[English](README.md) | 中文

# Utils

## 基本介绍
Utils 模块提供 C++ 工程常用的工具集，并且屏蔽底层平台差异，具体功能如下：
- 获取时间戳
- 线程管理
- 系统状态管理
- 字符串操作
- `SM2、SM3、Base58` 等加解密
- 单实例类
- 线程锁
- 随机数
- 无拷贝构造函数基类
- 写日志
- 读写文件
- 原子操作

## 模块结构
类名称 | 声明文件 | 功能
|:--- | --- | ---
| `utils` | [utils.h](./utils.h) | 实现以下功能：一是定义时间、字节单位的全局静态变量；二是实现原子性加减函数；三是实现 `ObjectExit` 类，用于批量处理对象自动释放使用；四是实现其他小函数：如 cpu 核数，sleep 函数，开机时间等。
| `Timestamp` | [timestamp.h](./timestamp.h) | 时间戳工具类。能获取系统的时间戳，精确到微秒，并具有跨平台性。
| `Timer` | [timer.h](./timer.h) | 定时器工具类。可以在设置时间内，定时执行某函数。
| `Thread` | [thread.h](./thread.h) | 跨平台的线程工具类。实现 `ThreadPool`线程池、`Mutex` 线程锁等。
| `System` | [system.h](./system.h) | 跨平台的系统工具类。实现查询硬件信息的功能。如硬盘、内存、主机名称、系统版本、日志大小、开机时间、cpu、硬件地址。
| `String` | [strings.h](./strings.h) | 字符串处理类。实现多种字符串操作功能。如格式化、去空格、转数字、转二进制等。
| `Sm3` | [sm3.h](./sm3.h) | 实现 SM3 加密算法。
| `Singleton` | [singleton.h](./singleton.h) | 单实例模板类。保证该类的继承者为单实例类。
| `random` 相关| [random.h](./random.h) | 获取随机字节。
| `NonCopyable` | [noncopyable.h](./noncopyable.h) | 无拷贝构造函数和赋值函数的基类。
| `Logger` | [logger.h](./logger.h) | 日志操作类。有如下特点：一是提供多样化输出方式，如文件、控制台；二是提供不同级别的日志输出，如`NONE，TRACE，DEBUG，INFO，WARN，ERROR，FATAL，ALL`；三是自动化管理日志文件，如超过指定大小或日期则新生成文件、定时清理过期的日志文件等。
| `File` | [file.h](./file.h) | 文件读写类，具有跨平台性。实现文件读写、目录操作等功能。
| `EccSm2` | [ecc_sm2.h](./ecc_sm2.h) | 实现 SM2 算法。
| `crypto` 相关 | [crypto.h](./crypto.h) | 加密库的合集。实现加密算法如：`Base58，Sha256，MD5，Aes` 等。
| `AtomMap` | [atom_map.h](./atom_map.h) | 原子操作性的数据集。可以存储大量非重复的 Key-Value 数据，用于保证对数据集的操作具有原子性。
| `uint128_t` | [base_int.h](./base_int.h) | 大数运算的封装类。

