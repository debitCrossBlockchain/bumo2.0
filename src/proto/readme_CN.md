## 模块简介
BUMO的序列化消息定义模块。在区块链程序的内部通信、节点通信和永久化存储等方面，都会用到数据的序列化和反序列化。BUMO使用protobuf工具作为序列化工具。在使用时，首先按照protobuf的标准定义message结构，然后调用protobuf的编译工具，将定义的message文件生成C++源文件，如此，就可以被BUMO的其他源文件直接掉了。


## 模块组成
类名称 | 声明文件 | 功能
|:--- | --- | ---
|`xxx`              | [xxx.h](./xxx.h)                           | 描述
|`xxx`              | [xxx.h](./xxx.h)                           | 描述
|`xxx`              | [xxx.h](./xxx.h)                           | 描述
|`xxx`              | [xxx.h](./xxx.h)                           | 描述
|`xxx`              | [xxx.h](./xxx.h)                           | 描述
|`xxx`              | [xxx.h](./xxx.h)                           | 描述
|`xxx`              | [xxx.h](./xxx.h)                           | 描述


## 框架流程


