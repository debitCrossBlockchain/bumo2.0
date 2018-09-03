[English](manual.md) | 中文

# BUMO区块链使用文档

<!-- TOC -->

- [编译](#编译)
    - [Linux](#linux)
    - [MAC](#mac)
    - [Windows](#windows)
- [部署](#部署)
    - [配置要求](#配置要求)
    - [Linux下的安装步骤](#linux下的安装步骤)
        - [Linux编译方式安装](#linux编译方式安装)
        - [Linux安装包安装](#linux安装包安装)
    - [MAC 下的安装步骤](#mac下的安装步骤)
        - [MAC编译方式安装](#mac编译方式安装)
        - [MAC安装包安装](#mac安装包安装)
    - [目录结构](#目录结构)
    - [切换运行环境](#切换运行环境)
    - [运行](#运行)
    - [运行状态](#运行状态)
    - [配置](#配置)
        - [数据存储](#数据存储)
        - [节点间网络通信](#节点间网络通信)
        - [WEB API 配置](#web-api-配置)
        - [WebSocket API 配置](#websocket-api-配置)
        - [区块配置](#区块配置)
        - [创世区块](#创世区块)
        - [日志配置](#日志配置)
        - [多节点配置示例](#多节点配置示例)
- [运维](#运维)
    - [服务启动与停止](#服务启动与停止)
    - [查看系统详细状态](#查看系统详细状态)
    - [清空数据库](#清空数据库)   
    - [创建硬分叉](#创建硬分叉)   

<!-- /TOC -->

## 编译

如果不想编译源码，可以直接使用安装包部署，[安装包下载]( https://github.com/bumoproject/bumo/releases/ "download")，下载完成后参考[Linux 安装包安装](#使用安装包安装)，[MAC 安装包安装](#使用安装包安装)

### Linux
支持 Ubuntu、Centos 等大多数操作系统编译，推荐使用版本Ubuntu 14.04，Centos 7。下面编译步骤以 Ubuntu 14.04 示例
- 安装依赖

```bash
sudo apt-get install automake
sudo apt-get install autoconf
sudo apt-get install libtool
sudo apt-get install g++
sudo apt-get install libssl-dev
sudo apt-get install cmake
sudo apt-get install libbz2-dev
sudo apt-get install python
sudo apt-get install unzip
sudo apt-get install wget
```
- 编译
```bash
##首次下载代码后，需要初始化开发环境，从服务器下载相关依赖库
cd bumo/build/
./install-build-deps-linux.sh
cd ../
make
```

生成的可执行文件目录：bumo/bin

### MAC
- 支持 MAC OS X 10.11.4 及以上系统版本
- 安装最新的Xcode 和 Command Tools，至少在8.0以上版本， 可以通过[苹果官网下载](https://developer.apple.com/download/more/)或者[苹果APP商城下载](https://itunes.apple.com/us/app/xcode/id497799835)
- 安装brew，参考 [brew安装](https://brew.sh/index_zh-cn.html)
- 使用brew执行安装依赖包

```bash
export HOMEBREW_NO_AUTO_UPDATE=true
brew install automake
brew install autoconf
brew install libtool
brew install cmake
brew install python
brew install m4
brew install wget
```


- 编译 

``` bash
#首次下载代码后，需要初始化开发环境，从服务器下载相关依赖库
cd bumo/build/
./install-build-deps-mac.sh
cd ../
make
```

生成的编译临时目录:bumo/bulid/mac/

生成的可执行文件目录：bumo/bin/

### Windows
- 支持 WinXP/2003/Vista/7/8/10平台编译，推荐使用 Win10
- 安装 Visual Studio Ulimate 2013
- 打开 buchain\build\win32\Bumo.vs12.sln, 使用 VS 自带编译器编译即可。生成的可执行文件在bumo\build\win32\dbin 目录下。
- 首次下载代码后，需要初始化开发环境，从服务器下载相关依赖库，进入 build目录，双击执行 install-build-deps-win32.bat 脚本。

## 部署
Windows 部署与 Linux 下部署基本类似，本示例以 Linux 为准。

### 配置要求
- 推荐配置：CPU 8 核，内存 32G，带宽 20 M，SSD 磁盘 500G。或更高配置
- 最低配置：CPU 4 核，内存 16G，带宽 10 M，SSD 磁盘 500G。

### Linux下的安装步骤

#### Linux编译方式安装
```bash
cd bumo
make install
```
服务将安装在/usr/local/buchain/目录下

安装完成。

#### Linux安装包安装

这里介绍另外一种安装方式，使用安装包安装：[安装包下载]( https://github.com/bumoproject/bumo/releases/ "download")

解压

拷贝 buchain-`1.0.0.x`-linux-x64.tar.gz 到 /usr/local/

    cd /usr/local/
    //需要注意用实际版本包 1.0.0.x 的名字
    tar xzvf buchain-1.0.0.x-linux-x64.tar.gz

注册服务

    ln -s /usr/local/buchain/scripts/bumo /etc/init.d/bumo 
    ln -s /usr/local/buchain/scripts/bumod /etc/init.d/bumod 

修改服务启动路径

打开 ./buchain/scripts/bumo 和 ./buchain/scripts/bumod 

将 `install_dir` 变量值修改成安装 Buchain 安装路径 

    install_dir=/usr/local/buchain 

设置开机启动

    #分别执行如下命令（级别1~5）
    ln -s -f /etc/init.d/bumod /etc/rc1.d/S99bumod 
    ln -s -f /etc/init.d/bumod /etc/rc2.d/S99bumod 
    ln -s -f /etc/init.d/bumod /etc/rc3.d/S99bumod 
    ln -s -f /etc/init.d/bumod /etc/rc4.d/S99bumod 
    ln -s -f /etc/init.d/bumod /etc/rc5.d/S99bumod 

在 `/etc/rc.local` 文件末尾追加如下命令

    /etc/init.d/bumod start

保存后添加执行可执行权限： 

    chmod +x /etc/rc.local

安装完成。

### MAC下的安装步骤

MAC 下安装需要注意两点：
- MAC 系统目录下 /usr/local/ 执行命令或者程序时，都需要加上 sudo 权限，后续的操作不再重复描述。
- MAC 系统一般是桌面系统，MAC 没有开机启动和 service 功能，因此不用使用 ln 命令设置开启和服务功能。

#### MAC编译方式安装
```bash
cd bumo
sudo make install
```
程序将安装在/usr/local/buchain/目录下

安装完成。
 
#### MAC安装包安装

这里介绍另外一种安装方式，使用安装包安装：[安装包下载]( https://github.com/bumoproject/bumo/releases/ "download")

解压

拷贝 buchain-`1.0.0.x`-macOS-x64.tar.gz 到 /usr/local/

```
cd /usr/local/
//需要注意用实际版本包的名字
sudo tar xzvf buchain-1.0.0.x-macOS-x64.tar.gz
```

解压完后，即安装完毕

### 目录结构

目录 | 描述 
|:--- | --- 
| bin | 存放可执行文件（编译后的bumo可执行程序）
|jslib| 存放第三方js库
| config | 配置文件目录包含：bumo.json
| data | 数据库目录，存放账本数据
| script | 启停脚本目录 (MAC 系统没有该目录)
| log | 运行日志存储目录


### 切换运行环境

安装完成后，如果要切换 BUMO 运行环境，则需要手动替换配置文件。步骤如下：

1、首先需要停止 bumo 程序，
```bash
    service bumod stop
    #MAC 系统没有 service 服务，直接终止 bumo 程序
```
2、替换配置文件
```bash
    cd /usr/local/buchain/config/
    #拷贝目标环境配置文件
    cp bumo-testnet.json bumo.json  

    #配置文件环境说明
    bumo.json           ##程序启动后默认加载的文件
    bumo-mainnet.json   ##主网环境配置文件
    bumo-testnet.json   ##测试网配置文件
    bumo-single.json    ##单机节点调试环境
```
3、并清空数据库并启动服务
```bash
    cd ../
    ./bin/bumo --dropdb
    service bumod start

    #MAC 系统没有 service 服务，直接使用 sudo ./bin/bumo
```
### 运行

```bash
    service bumod start
    #MAC 系统没有 service 服务，直接使用 sudo ./bin/bumo
```

### 运行状态

```bash
    service bumod status
    #MAC 系统没有 service 服务
```

### 配置

bumo.json

#### 数据存储

```json
    "db":
    {
        "account_path": "data/account.db", //用来存储账号数据
        "ledger_path": "data/ledger.db", //存储区块数据
        "keyvalue_path": "data/keyvalue.db" //存储共识数据
    }
```
#### 节点间网络通信
```json
    "p2p":
    {
        "network_id":30000,//网络 ID
        //共识网络
        "consensus_network":
        {
            "heartbeat_interval":60, //心跳周期，秒
            "listen_port":36001,//已监听的端口
            "target_peer_connection":50,  //最大主动连接节点数
            "known_peers":
            [
                "127.0.0.1:36001"//连接其他节点
            ]
        }
    }
```

#### WEB API 配置

```json
    "webserver":{
        "listen_addresses":"0.0.0.0:36002" 
    }
```

#### WebSocket API 配置 

```json
    "wsserver":
    {
        "listen_address":"0.0.0.0:36003"
    }
```

#### 区块配置

```json
    "ledger":
    {
        "validation_address":"buQmtDED9nFcCfRkwAF4TVhg6SL1FupDNhZY",//验证节点地址，同步节点或者钱包不需要配置
        "validation_private_key": "e174929ecec818c0861aeb168ebb800f6317dae1d439ec85ac0ce4ccdb88487487c3b74a316ee777a3a7a77e5b12efd724cd789b3b57b063b5db0215fc8f3e89", //验证节点私钥，同步节点或者钱包不需要配置
        "max_trans_per_ledger":1000,  //单个区块最大交易个数
        "tx_pool":                      //交易池配置
        {
            "queue_limit":10240,            //交易池总量限制
            "queue_per_account_txs_limit":64    //单个账号的交易缓冲最大值
        }
    }
```

`validation_address` 和 `validation_private_key` 可以通过 bumo 程序命令行工具获得，请妥善保存该账号信息，丢失后将无法找回。
```
    [root@bumo ~]# cd /usr/local/buchain/bin
    [root@bumo bin]#./bumo --create-account

    {
        "address" : "buQmtDED9nFcCfRkwAF4TVhg6SL1FupDNhZY", //地址
        "private_key" : "privbsZozNs3q9aixZWEUzL9ft8AYph5DixN1sQccYvLs2zPsPhPK1Pt", //私钥
        "private_key_aes" : "e174929ecec818c0861aeb168ebb800f6317dae1d439ec85ac0ce4ccdb88487487c3b74a316ee777a3a7a77e5b12efd724cd789b3b57b063b5db0215fc8f3e89", //AES 加密的私钥
        "public_key" : "b00108d329d5ff69a70177a60bf1b68972576b35a22d99d0b9a61541ab568521db5ee817fea6", //公钥
        "public_key_raw" : "08d329d5ff69a70177a60bf1b68972576b35a22d99d0b9a61541ab568521db5e", //原始公钥
        "sign_type" : "ed25519" //ed25519 加密方式
    }
```

### 创世区块
```json
   "genesis": 
   {
        "account": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3", //创世区块地址
        "slogan" : "a new era of value",  //存储在创世区块中的标语
        "fees": 
        {
            "base_reserve": 10000000,  //账号最低预留费
            "gas_price": 1000          //字节费
        },
        "validators": ["buQBwe7LZYCYHfxiEGb1RE9XC9kN2qrGXWCY"] //验证节点区块列表
    }
```
同一个区块链上的 `genesis` 配置，必须保持一致

`account`  可以通过 bumo 程序命令行工具 `./bumo --create-account` 获取，请妥善保存该账号信息，丢失后将无法找回。
#### 日志配置

```json
    "logger":
    {
        "path":"log/buchain.log", // 日志目录
        "dest":"FILE|STDOUT|STDERR", //输出文件分类
        "level":"TRACE|INFO|WARNING|ERROR|FATAL",//日志级别
        "time_capacity":1, //时间容量，天
        "size_capacity":10, //大小容量，兆
        "expire_days":10  //清理日志周期，天
    }
```

#### 多节点配置示例


以上章节介绍了配置文件的基本参数。本节以两个验证节点和一个同步节点为例，介绍多节点在一条链的配置，其中需要修改 p2p、validation 和 ledger 这三模块，具体示例如下

p2p 的 known_peers 必须为其他已知节点的 IP 和端口，用于节点之间相互连接
``` json
验证节点一：
"p2p":
{
    "network_id":30000,
    "consensus_network":
    {
        "heartbeat_interval":60,
        "listen_port":36001,
        "target_peer_connection":50,
        "known_peers":
        [
            "192.168.1.102:36001",   //节点二的 IP 和端口
            "192.168.1.103:36001"   //节点三的 IP 和端口
        ]
    }
}

验证节点二：
"p2p":
{
    "network_id":30000,
    "consensus_network":
    {
        "heartbeat_interval":60,
        "listen_port":36001,
        "target_peer_connection":50,
        "known_peers":
        [
            "192.168.1.101:36001",   //节点一的 IP 和端口
            "192.168.1.103:36001"   //节点三的 IP 和端口
        ]
    }
}

同步节点三：
"p2p":
{
    "network_id":30000,
    "consensus_network":
    {
        "heartbeat_interval":60,
        "listen_port":36001,
        "target_peer_connection":50,
        "known_peers":
        [
            "192.168.1.101:36001",   //节点一的 IP 和端口
            "192.168.1.102:36001"    //节点二的 IP 和端口
        ]
    }
}
```

验证节点的 ledger 的 validation_address 和 validation_private_key 必须要匹配。并且需要把所有验证节点的 validation_address 填写到 genesis.validators 里

``` json
验证节点一：
"ledger":
{
    "validation_address":"buQBwe7LZYCYHfxiEGb1RE9XC9kN2qrGXWCY",//验证节点一地址，同步节点或者钱包不需要配置
    "validation_private_key": "66932f19d5be465ea9e7cfcb3ea7326d81953b9f99bc39ddb437b5367937f234b866695e1aae9be4bae27317c9987f80be882ae3d2535d4586deb3645ecd7e54", //验证节点一的私钥，同步节点或者钱包不需要配置
    "max_trans_per_ledger":1000,
    "tx_pool":
    {
        "queue_limit":10240,
        "queue_per_account_txs_limit":64
    }
}

验证节点二：
"ledger":
{
    "validation_address":"buQqkp5SDcsxpwWXQ2QFQbvHKnZ199HY3dHm",//验证节点二地址，同步节点或者钱包不需要配置
    "validation_private_key": "1cb0151ec2b23cb97bf94d86ee1100582f9f5fbfdfe40a69edae2d2b8711395c40c1da859ac0bc93240a8a70c4a06779ed06d299880417d71fc51c1a0267875f", //验证节点二的私钥，同步节点或者钱包不需要配置
    "max_trans_per_ledger":1000,
    "tx_pool":
    {
        "queue_limit":10240,
        "queue_per_account_txs_limit":64
    }
}

同步节点三：
"ledger":
{
    "max_trans_per_ledger":1000,
    "tx_pool":
    {
        "queue_limit":10240,
        "queue_per_account_txs_limit":64
    }
}
```

同一个区块链上的 `genesis` 配置，必须保持一致

```json
验证节点一：
"genesis": 
{
    "account": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3",
    "slogan" : "a new era of value",
    "fees": 
    {
        "base_reserve": 10000000,
        "gas_price": 1000
    },
    "validators": ["buQBwe7LZYCYHfxiEGb1RE9XC9kN2qrGXWCY", "buQqkp5SDcsxpwWXQ2QFQbvHKnZ199HY3dHm"]  //需要配置所有的验证节点地址，如果有两个验证节点，则配置两个地址。
}

验证节点二：
"genesis": 
{
    "account": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3",
    "slogan" : "a new era of value",
    "fees": 
    {
        "base_reserve": 10000000,
        "gas_price": 1000
    },
    "validators": ["buQBwe7LZYCYHfxiEGb1RE9XC9kN2qrGXWCY", "buQqkp5SDcsxpwWXQ2QFQbvHKnZ199HY3dHm"]  //需要配置所有的验证节点地址，如果有两个验证节点，则配置两个地址。
}

同步节点三：
"genesis": 
{
    "account": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3",
    "slogan" : "a new era of value",
    "fees": 
    {
        "base_reserve": 10000000,
        "gas_price": 1000
    },
    "validators": ["buQBwe7LZYCYHfxiEGb1RE9XC9kN2qrGXWCY", "buQqkp5SDcsxpwWXQ2QFQbvHKnZ199HY3dHm"]  //需要配置所有的验证节点地址，如果有两个验证节点，则配置两个地址。
}
```

注意：运行前请确保每个节点的初始数据是一致，否则无法达成共识产生区块。

`account`，`validation_address` 可以通过 bumo 程序命令行工具 `./bumo --create-account` 获取，请妥善保存该账号信息，丢失后将无法找回。


## 运维
### 服务启动与停止
``` bash
启动    :service bumod start
关闭    :service bumod stop
运行状态:service bumod status

#MAC 系统没有 service 服务，直接使用命令行启动和关闭 sudo ./bin/bumo 

```
### 查看系统详细状态

```bash
[root@bumo ~]# curl 127.0.0.1:36002/getModulesStatus
{
    "glue_manager":{
        "cache_topic_size":0,
        "ledger_upgrade":{
            "current_states":null,
            "local_state":null
        },
        "system":{
            "current_time":"2017-07-20 10:32:22", //当前系统时间
            "process_uptime":"2017-07-20 09:35:06", //bumo启动时间
            "uptime":"2017-05-14 23:51:04"
        },
        "time":"0 ms",
        "transaction_size":0
    },
    "keyvalue_db":Object{...},
    "ledger_db":Object{...},
    "ledger_manager":{
        "account_count":2316,  //账户数
        "hash_type":"sha256",
        "ledger_sequence":12187,
        "time":"0 ms",
        "tx_count":1185   //交易数
    },
    "peer_manager":Object{...},
    "web server":Object{...},

```

### 清空数据库
```bash
buchain/bin/bumo --dropdb
```
### 创建硬分叉
```bash
buchain/bin/bumo --create-hardfork
buchain/bin/bumo --clear-consensus-status
```
当已经加入其他区块链网络的节点想单独运行一条链时，可以执行以上命令创建硬分叉。
执行后，新的区块链网络只有一个验证节点为本节点。
- 执行硬分叉命令后获取到如下Hash值
```bash
Create hard fork ledger successful, seq(20), consensus value hash(**7aa332f05748e6ce9ad3d059c959a50675109bcaf0a4ba2c5c6adc6418960197**)
```
- 把上述 Hash 值配置到本节点或者同步节点的 bumo.json 的hardfork_points

```json
    "ledger":
    {
       	"genesis_account": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3",
        "max_trans_per_ledger": 1000,
        "hardfork_points" : 
        [
        	"7aa332f05748e6ce9ad3d059c959a50675109bcaf0a4ba2c5c6adc6418960197"
        ]
    },
```

- 启动节点服务即可生效

