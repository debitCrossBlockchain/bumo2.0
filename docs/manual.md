# __BUMO区块链使用文档__

[TOC]


## __编译__
### Linux
支持 Ubuntu、Centos 等大多数操作系统编译，下面编译步骤以 Ubuntu 14.04 示例
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
- 安装最新的Xcode 和 Command Tools，至少在8.0以上版本， 可以通过[苹果官网下载](https://developer.apple.com/download/more/)或者[苹果APP商城下载](https://itunes.apple.com/us/app/xcode/id497799835)
- 安装brew，参考 [brew安装](https://brew.sh/index_zh-cn.html)
- 使用brew执行安装依赖包

 ```
bash
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


 ```
bash
##首次下载代码后，需要初始化开发环境，从服务器下载相关依赖库
cd bumo/build/
./install-build-deps-mac.sh
cd ../
make
```

生成的编译临时目录:bumo/bulid/mac/
生成的可执行文件目录：bumo/bin/

### Windows
- 安装 Visual Studio Ulimate 2013
- 打开 bumo\build\win32\Bumo.vs12.sln, 使用 VS 自带编译器编译即可。生成的可执行文件在bumo\build\win32\dbin 目录下。
- 首次下载代码后，需要初始化开发环境，从服务器下载相关依赖库，进入 build目录，双击执行 install-build-deps-win32.bat 脚本。

## __部署__
Windows 部署与 Linux 下部署基本类似，本示例以 Linux 为准。

### Linux下的安装步骤
```bash
cd bumo
make install
```

服务将安装在/usr/local/bumo/目录下

### __目录结构__

目录 | 描述 
|:--- | --- 
| bin | 存放可执行文件（编译后的bumo可执行程序）
|jslib| 存放第三方js库
| config | 配置文件目录包含：bumo.json
| data | 数据库目录，存放账本数据
| script | 启停脚本目录
| log | 运行日志存储目录


### __切换测试环境__

如果需要切换 BUMO 的运行环境，需要手动替换配置文件。步骤如下：

1、首先需要停止 bumo 程序，
```bash
    service bumo stop
```
2、替换配置文件
```bash
    cd /usr/local/bumo/config/
    #拷贝目标环境配置文件
    cp bumo-testnet.json bumo.json  

    #配置文件环境说明
    bumo.json           ##默认调试环境
    bumo-mainnet.json   ##主网环境配置文件，尚未启用
    bumo-testnet.json   ##测试网配置文件
```
3、并清空数据库并启动服务
```bash
    cd ../
    ./bin/bumo --dropdb
    service bumo start
```
### __运行__

```bash
    service bumo start
```

### __运行状态__

```bash
    service bumo status
```

### __配置__

#### config.json 
##### 数据存储

```json
    "db":{
		"account_path": "data/account.db", //用来存储账号数据
		"ledger_path": "data/ledger.db", //存储区块数据
		"keyvalue_path": "data/keyvalue.db" //存储共识数据
    }
```
##### 节点间网络通信
```json
    "p2p":{
        "network_id":10000,//网络ID,区分测试路和主网
        //共识网络
        "consensus_network":{
            "heartbeat_interval":60,
            "listen_port":16001,//已监听的端口
            "target_peer_connection":50,
            "known_peers":[
                "127.0.0.1:16001"//连接其他节点
            ]
        }
    }
```

##### WEB API 配置

```json
    "webserver":{
        "listen_addresses":"0.0.0.0:16002"
    }
```

##### WebSocket API 配置 

```json
    "wsserver":{
        "listen_address":"0.0.0.0:16003"
    }
```

##### 区块配置

```json
    "ledger":{
        "genesis_account":"buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3",//创世账号，同一条链上的每一个节点都必须相同
        "max_trans_per_ledger":1000,
        "max_ledger_per_message":5,
        "max_trans_in_memory":2000,
        "max_apply_ledger_per_round":3
    }
```

##### 日志配置

```json
    "logger":{
        "path":"log/bumo.log", // 日志目录
        "dest":"FILE|STDOUT|STDERR", //输出文件分类
        "level":"TRACE|INFO|WARNING|ERROR|FATAL",//日志级别
        "time_capacity":1,
        "size_capacity":10,
        "expire_days":10
    }
```

##### 共识配置

```json
    "validation":{
		"address": "buQdEwCroM2PaMSTWFK4XQ63AVMtKDqRdz3g",
		"node_private_key": "01ab35a14935d51faf48f51c3bfd1e0478e83d21d378f3bc92332dbbee5b180b9e122475517da2651227ce4a8bb37e2621c4611a889a8d7e45679efe881d56f2",
		"validators": [
			"buQdEwCroM2PaMSTWFK4XQ63AVMtKDqRdz3g"
		]
    }
```

#### 多节点配置说明

- 下面示例是配置多个节点在一条链上运行示例，配置多节点主要修改p2p、validation和ledger这三块的设置

##### 节点间网络通信

- config.p2p.consensus_network.known_peers 填写其他节点的 ip 以及 port

##### 共识配置

- validators 填写每个节点 validation 的 address
- address 与 node_private_key是成对应关系

##### 区块配置
- config.ledger.genesis_account 是创世账号，同一条链上，每个节点配置中 genesis_account 的值必须一致

注意：运行前请确保每个节点的初始数据是一致，否则无法达成共识产生区块

#### 配置同步节点
 - 配置同步节点与验证节点有一点不同的是共识配置中validators不需要填写同步节点validation的address
 
##### 加密数据配置
配置文件中所有隐私数据都是加密存储的，解密密钥都是被硬编码在程序中。所以拿到密码明文后需要经过如下转换才可配置：

- 命令./bin/bumo --aes-crypto [参数]

```bash
[root@localhost bumo]# ./bin/bumo --aes-crypto root e2ba44bf0b27f0acbe7b5857e3bc6348
```
- 需加密配置项 

名称 | 描述 
|:--- | --- 
| config.validation.node_private_key | 共识节点私钥

## __运维__
### 服务启动与停止
```
启动    :service bumo start
关闭    :service bumo stop
运行状态:service bumo status
```
### 查看系统详细状态

```bash
[root@centos7x64-201 ~]# curl 127.0.0.1:19333/getModulesStatus
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
### 查看具体数据信息

```bash
[root@centos7x64-201~]#curl 127.0.0.1:19333/getAccount?address=a0024111d1cc90ac8ee0abd5f957e08e3e1b442b581e88
{
  "error_code": 0,
  "result": {
    "address": "a0024111d1cc90ac8ee0abd5f957e08e3e1b442b581e88",
    "assets": null,
    "assets_hash": "ad67d57ae19de8068dbcd47282146bd553fe9f684c57c8c114453863ee41abc3",
    "contract": null,
    "metadatas": null,
    "priv": {
      "master_weight": 1,
      "thresholds": {
        "tx_threshold": 1
      }
    },
    "storage_hash": "ad67d57ae19de8068dbcd47282146bd553fe9f684c57c8c114453863ee41abc3"
  }
} [root@centos7x64-201 ~]#

```
### 清空数据库
```bash
bumo/bin/bumo --dropdb
```
### 创建硬分叉
```bash
bumo/bin/bumo --create-hardfork
bumo/bin/bumo --clear-consensus-status
```
当已经加入其他区块链网络的节点想单独运行一条链时，可以执行以上命令创建硬分叉
执行后，新的区块链网络只有一个验证节点为本节点。
- 执行硬分叉命令后获取到如下Hash值
```bash
Create hard fork ledger successful, seq(20), consensus value hash(**7aa332f05748e6ce9ad3d059c959a50675109bcaf0a4ba2c5c6adc6418960197**)
```
- 把上述 Hash 值配置到本节点或者同步节点的 bumo.json 的hardfork_points

```json
    "ledger": {
       	"genesis_account": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3",
        "max_trans_per_ledger": 1000,
        "max_ledger_per_message": 5,
        "max_trans_in_memory": 2000,
        "max_apply_ledger_per_round": 3,
        "hardfork_points" : 
        [
        	"7aa332f05748e6ce9ad3d059c959a50675109bcaf0a4ba2c5c6adc6418960197"
        ]
    },
```

- 启动节点服务即可生效

### 数据库存储
BUMO 区块链存储的数据默认是存放在 bumo/data 目录下，如有需要可修改配置文件中数据存储部分
