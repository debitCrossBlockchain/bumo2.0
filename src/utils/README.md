English | [中文](README_CN.md) 

# Utils

## Introduction
The Utils module provides a common set of tools for C++ projects and shields the underlying platform differences. The specific functions are as follows:
- Get timestamp
- Thread management
- System state management
- String manipulation
- `SM2、SM3、Base58` etc,. encryption
- Single instance class
- Thread lock
- Random number
- Copyless constructor base class
- Write log
- Read and write files
- Atomic operation

## Module structure
Class name | Statement file | Function
|:--- | --- | ---
| `utils` | [utils.h](./utils.h) | The following functions are implemented: one is to define the time, the global static variable of the byte unit; the second is to implement the atomic addition and subtraction function; the third is to implement the `ObjectExit` class for batch processing object and automatically release usage; the fourth is to implement other small functions, such as cpu core number, sleep function, boot time and so on.
| `Timestamp` | [timestamp.h](./timestamp.h) | Timestamp tool class. Get the timestamp of the system, precise to microsecond, and cross-platform.
| `Timer` | [timer.h](./timer.h) | Timer tool class. A function can be executed periodically during the set time.
| `Thread` | [thread.h](./thread.h) | Cross-platform threading tool class. Implement `ThreadPool` thread pool, `Mutex` thread lock, etc.
| `System` | [system.h](./system.h) | A cross-platform system tool class. Implement the function of querying hardware information. Such as hard disk, memory, host name, system version, log size, boot time, cpu, hardware address.
| `String` | [strings.h](./strings.h) | String processing class. Implement a variety of string manipulation features. Such as formatting, removing spaces, converting numbers, converting binary, etc.
| `Sm3` | [sm3.h](./sm3.h) | Implements SM3 encryption algorithm
| `Singleton` | [singleton.h](./singleton.h) | Single instance template class. Ensure that the successor of this class is a single instance class.
| `random` 相关| [random.h](./random.h) | Get random bytes.
| `NonCopyable` | [noncopyable.h](./noncopyable.h) | The base class for copyless constructors and assignment functions.
| `Logger` | [logger.h](./logger.h) | Log operation class. It has the following features: first, it provides diversified output methods, such as files and consoles; second, it provides different levels of log output, such as `NONE, TRACE, DEBUG, INFO, WARN, ERROR, FATAL, ALL`; third, automatically manage log files, such as generating a file if it exceeds the specified size or date, periodically cleans up expired log files, and so on.
| `File` | [file.h](./file.h) | File read and write classes, cross-platform. Implement file read and write, directory operations and other functions.
| `EccSm2` | [ecc_sm2.h](./ecc_sm2.h) | Implements SM2 algorithm.
| `crypto` 相关 | [crypto.h](./crypto.h) | A collection of cryptographic libraries. It implements encryption algorithms such as `Base58, Sha256, MD5, Aes`, etc.
| `AtomMap` | [atom_map.h](./atom_map.h) | Atomically operational data set. A large amount of non-repeating Key-Value data can be stored to ensure atomicity of operations on the data set.
| `uint128_t` | [base_int.h](./base_int.h) | The wrapper class for large numbers of operations.

