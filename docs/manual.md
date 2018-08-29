English | [中文](manual_CN.md) 

# BUMO Blockchain Manual Doc

<!-- TOC -->
- [Compile](#compile)
    - [Linux](#linux)
    - [MAC](#mac)
    - [Windows](#windows)
- [Deployment](#deployment)
    - [Requirement](#requirement)
    - [Installation on Linux](#installation-on-linux)
        - [Linux use compile mode deploy](#linux-use-compile-mode-deploy)
        - [Linux use archive packet deploy](#linux-use-archive-packet-deploy)
    - [Installation on MAC](#installation-on-mac)
        - [MAC use compile mode deploy](#mac-use-compile-mode-deploy)
        - [MAC use archive packet deploy](#mac-use-archive-packet-deploy)
    - [Catalog Structure](#catalog-structure)
    - [Switch to Target Network](#switch-to-target-network)
    - [Operation](#operation)
    - [Operating Status](#operating-status)
    - [Configuration](#configuration)
	    - [Data Storage](#data-storage)
	    - [Network Communication between Nodes](#network-communication-between-nodes)
	    - [WEB API Configuration](#web-api-configuration)
	    - [WebSocket API Configuration](#websocket-api-configuration)
	    - [Block Configuration](#block-configuration)
	    - [Initial Block](#initial-block)
	    - [Log Configuration](#log-configuration)
	    - [Multi-nodes Cluster Configuration](#multi-nodes-cluster-configuration)
- [Operation and Maintenance](#operation-and-maintenance)
    - [Service Start and Stop](#service-start-and-stop)
    - [Get Modules Status](#get-modules-status)
    - [Get Account Information](#get-account-information)
    - [Drop Database](#drop-database)   
    - [Create Hard Fork](#create-hard-fork)   


<!-- /TOC -->

## Compile 

If you don't want to compile the source code, you can use the archive package directly, [archive package download](https://github.com/bumoproject/bumo/releases/ "download")

[Linux use archive packet deploy](#linux-use-archive-packet-deploy)

[MAC use archive packet deploy](#mac-use-archive-packet-deploy)

### Linux
BUMO is currently available for Ubuntu, Centos and most of the operating systems, it is recommended that you have Ubuntu 14.04 or Centos 7. The following demo is based on Ubuntu 14.04 .

- Dependencies

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
- Compilation
```bash
##After the first download of the code, you have to install related dependencies from the server for initializing the development environment.
cd bumo/build/
./install-build-deps-linux.sh
cd ../
make
```

Executable program dir:bumo/bin

### MAC
- Depending on MAC OS X 10.11.4 or later
- Install the latest version of Xcode and Command Tools(8.0 version or later). You can get it from [Apple's website](https://developer.apple.com/download/more/) or [Apple's app store](https://itunes.apple.com/us/app/xcode/id497799835).
- Install **brew**: Get more information [here](https://brew.sh/index.html)
- Install dependencies with **brew**

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

- Compilation


 ```
bash
#Downloading the code at the first time, you have to install related dependencies from the server for initializing the development environment.
cd bumo/build/
./install-build-deps-mac.sh
cd ../
make
 ```

Temporary executable program dir:bumo/bulid/mac/

Executable program dir:bumo/bin/

### Windows
- Supports WinXP/2003/Vista/7/8/10 to building，Recommended Win10
- Install Visual Studio Ultimate 2013
- Compile `buchain\build\win32\Bumo.vs12.sln` with VS, then get the executable program in the dir `bumo\build\win32\dbin`
- After the first download of the code, you have to install related dependencies from the server to initialize the development environment. Enter the dir `build`, and double click the following:`install-build-deps-win32.bat`.

## Deployment
The deployment on Windows is almost identical to Linux. (Subject to Linux)

### Requirement
- Recommended configuration: CPU 8 core, memory 32 G, bandwidth 20 M, SSD disk 500 G. Or higher.
- Minimum configuration: CPU 4 core, memory 16 G, bandwidth 10 M, SSD disk 500 G.

### Installation on Linux
#### Linux use compile mode deploy
```bash
cd bumo
make install
```

Install under `/usr/local/buchain/`

Deploy ok!

#### Linux use archive packet deploy

This is another deployment, using the archive packet.

Extract files

Copy buchain-`1.0.0.x`-linux-x64.tar.gz to /usr/local/

    cd /usr/local/
    //Note the name of the actual version of the package 1.0.0.x.
    tar xzvf buchain-1.0.0.x-linux-x64.tar.gz

Registration service

    ln -s /usr/local/buchain/scripts/bumo /etc/init.d/bumo 
    ln -s /usr/local/buchain/scripts/bumod /etc/init.d/bumod 

Modify startup path

Open ./buchain/scripts/bumo 和 ./buchain/scripts/bumod 

Modify `install_dir` to Buchain's deployment path

    install_dir=/usr/local/buchain 

Setup startup

    #Execute the following commands separately.（level: 1~5）
    ln -s -f /etc/init.d/bumod /etc/rc1.d/S99bumod 
    ln -s -f /etc/init.d/bumod /etc/rc2.d/S99bumod 
    ln -s -f /etc/init.d/bumod /etc/rc3.d/S99bumod 
    ln -s -f /etc/init.d/bumod /etc/rc4.d/S99bumod 
    ln -s -f /etc/init.d/bumod /etc/rc5.d/S99bumod 

Add the following command at the end of the `/etc/rc.local` file.

    /etc/init.d/bumod start

Save and add executable permissions.： 

    chmod +x /etc/rc.local

Deploy ok!

### Installation on MAC
There are two points to note:

- sudo permissions are required, and subsequent operations are not described repeatedly.
- The MAC does not have startup and service functions, so you don't need to use the ln command to set up and service functions.

#### MAC use compile mode deploy

```bash
cd bumo
sudo make install
```

Install under `/usr/local/buchain/`

Deploy ok!

#### MAC use archive packet deploy


This is another deployment, using the archive packet.

[archive package download](https://github.com/bumoproject/bumo/releases/ "download")

Extract files

Copy buchain-`1.0.0.x`-macOS-x64.tar.gz to /usr/local/

```
cd /usr/local/
//Note the name of the actual version of the package 1.0.0.x.
tar xzvf buchain-1.0.0.x-macOS-x64.tar.gz
```

```
cd /usr/local/
sudo tar xzvf buchain-1.0.0.x-macOS-x64.tar.gz
```

Deploy ok!

### Catalog Structure

Catalog | Description 
|:--- | --- 
| bin | Executable program（Compiled bumo exe）
| jslib| Third-party js libraries
| config | Configuration profile, including `bumo.json`
| data | Warehouse of ledger data
| script | Activate script (The MacOS does not have this directory)
| log | Running log


### Switch to Target Network

Switch runtime environment of BUMO manually:

1. Stop bumo program

```bash
    service bumod stop
    #The MacOS does not have service and terminates the bumo program
```
2. Replace configuration profile

```bash
    cd /usr/local/buchain/config/
    #Copy the target configuration profile
    cp bumo-testnet.json bumo.json  

    #About configuration profile
    bumo.json           ##debugging environment is default
    bumo-mainnet.json   ##configuration profile of main network
    bumo-testnet.json   ##configuration profile of test network
    bumo-single.json    ##configuration profile of single network for debug
```
3. Drop database and restart service

```bash
    cd ../
    ./bin/bumo --dropdb
    service bumo start
    #The MacOS does not have a service and use sudo. /bin/bumo directly.
```
### Operation

```bash
    service bumo start
    #The MacOS does not have a service and use sudo. /bin/bumo directly.
```

### Operating Status 

```bash
    service bumo status
    #The MacOS does not have a service.
```

### Configuration

bumo.json 

#### Data Storage

```json
    "db":{
		"account_path": "data/account.db", //store account data
		"ledger_path": "data/ledger.db", //store block data
		"keyvalue_path": "data/keyvalue.db" //store consensus data
    }
```
#### Network Communication between Nodes

```json
    "p2p":
    {
        "network_id":30000,//Network ID, to distinguish test network from main network
        //consensu network
        "consensus_network":
        {
            "heartbeat_interval":60, //listened port
            "listen_port":36001,
            "target_peer_connection":50,  //Maximum number of active connection nodes
            "known_peers":
            [
                "127.0.0.1:36001"//link known nodes
            ]
        }
    }
```


#### WEB API Configuration

```json
    "webserver":{
        "listen_addresses":"0.0.0.0:36002"
    }
```

#### WebSocket API Configuration 

```json
    "wsserver":{
        "listen_address":"0.0.0.0:36003"
    }
```

#### Block Configuration

```json
    "ledger":{
        "validation_address":"buQBwe7LZYCYHfxiEGb1RE9XC9kN2qrGXWCY",//validation node's address( NO NEED to configurate for synchronized nodes or wallets)
        "validation_private_key": "66932f19d5be465ea9e7cfcb3ea7326d81953b9f99bc39ddb437b5367937f234b866695e1aae9be4bae27317c9987f80be882ae3d2535d4586deb3645ecd7e54", //validation node's private key( NO NEED to configurate for synchronized nodes or wallets)
        "max_trans_per_ledger":1000,  //the maximum number of transactions per block.
        "tx_pool":{
            "queue_limit":10240,
            "queue_per_account_txs_limit":64
        }
    }
```

`validation_address` and `validation_private_key` can be gained through bumo program command line tools, please keep the account information, after the loss will not be able to find.


```
    [root@bumo ~]# cd /usr/local/buchain/bin
    [root@bumo bin]#./bumo --create-account

    {
        "address" : "buQmtDED9nFcCfRkwAF4TVhg6SL1FupDNhZY", 
        "private_key" : "privbsZozNs3q9aixZWEUzL9ft8AYph5DixN1sQccYvLs2zPsPhPK1Pt", 
        "private_key_aes" : "e174929ecec818c0861aeb168ebb800f6317dae1d439ec85ac0ce4ccdb88487487c3b74a316ee777a3a7a77e5b12efd724cd789b3b57b063b5db0215fc8f3e89",
        "public_key" : "b00108d329d5ff69a70177a60bf1b68972576b35a22d99d0b9a61541ab568521db5ee817fea6", 
        "public_key_raw" : "08d329d5ff69a70177a60bf1b68972576b35a22d99d0b9a61541ab568521db5e", 
        "sign_type" : "ed25519" 
    }
```

#### Initial Block
```json
   "genesis": {
        "account": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3", //account of initial block
        "slogan" : "a new era of value",
        "fees": {
            "base_reserve": 10000000,  // based reserve of the account
            "byte_fee": 1000           //byte fee
        },
        "validators": ["buQBwe7LZYCYHfxiEGb1RE9XC9kN2qrGXWCY"] //validated nodes list
    }
```
    In the same blockchain, the configuration of `genesis` should keep consistent.

#### Log Configuration

```json
    "logger":{
        "path":"log/bumo.log", // log catalog
        "dest":"FILE|STDOUT|STDERR", //classification of output files
        "level":"TRACE|INFO|WARNING|ERROR|FATAL",//log level
        "time_capacity":1,
        "size_capacity":10,
        "expire_days":10
    }
```

#### Multi-nodes Cluster Configuration

The above sections describe the basic parameters of the configuration file.This section, taking two validation nodes and one synchronization node as examples, introduces the configuration of multiple nodes in a single chain, where the modules of p2p, validation and ledger need to be modified. The specific examples are as follows

P2P known_peers must be IP and port of other known nodes for nodes to connect to each other

``` json
validation 1：
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
            "192.168.1.102:36001",   //node 2's address
            "192.168.1.103:36001"   //node 3's address
        ]
    }
}

validation 2：
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
            "192.168.1.101:36001",   //node 1's address
            "192.168.1.103:36001"   //node 3's address
        ]
    }
}

Synchronization 3:
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
            "192.168.1.101:36001",   //node 1's address
            "192.168.1.102:36001"    //node 2's address
        ]
    }
}
```
Validation's `validation_address` and `validation_private_key` must match, and you need to fill in the `validation_address` of all validation nodes into the `generation.validators`


``` json
validation 1:
"ledger":
{
    "validation_address":"buQBwe7LZYCYHfxiEGb1RE9XC9kN2qrGXWCY",//validation 1' address
    "validation_private_key": "66932f19d5be465ea9e7cfcb3ea7326d81953b9f99bc39ddb437b5367937f234b866695e1aae9be4bae27317c9987f80be882ae3d2535d4586deb3645ecd7e54", //validation 1' private key
    "max_trans_per_ledger":1000,
    "tx_pool":
    {
        "queue_limit":10240,
        "queue_per_account_txs_limit":64
    }
}

validation 2:
"ledger":
{
    "validation_address":"buQqkp5SDcsxpwWXQ2QFQbvHKnZ199HY3dHm",//validation 2' address
    "validation_private_key": "1cb0151ec2b23cb97bf94d86ee1100582f9f5fbfdfe40a69edae2d2b8711395c40c1da859ac0bc93240a8a70c4a06779ed06d299880417d71fc51c1a0267875f", //validation 2' private key
    "max_trans_per_ledger":1000,
    "tx_pool":
    {
        "queue_limit":10240,
        "queue_per_account_txs_limit":64
    }
}

Synchronization 3:
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

Out of the same block chain ` genesis ` configuration, must be consistent

```json
validation 1:
"genesis": 
{
    "account": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3",
    "slogan" : "a new era of value",
    "fees": 
    {
        "base_reserve": 10000000,
        "gas_price": 1000
    },
    "validators": ["buQBwe7LZYCYHfxiEGb1RE9XC9kN2qrGXWCY", "buQqkp5SDcsxpwWXQ2QFQbvHKnZ199HY3dHm"]  //Configure all validation nodes's addresses, and if there are two, two addresses are configured.
}

validation 2:
"genesis": 
{
    "account": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3",
    "slogan" : "a new era of value",
    "fees": 
    {
        "base_reserve": 10000000,
        "gas_price": 1000
    },
    "validators": ["buQBwe7LZYCYHfxiEGb1RE9XC9kN2qrGXWCY", "buQqkp5SDcsxpwWXQ2QFQbvHKnZ199HY3dHm"]
    //Configure all validation nodes's addresses, and if there are two, two addresses are configured.
}

Synchronization 3:
"genesis": 
{
    "account": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3",
    "slogan" : "a new era of value",
    "fees": 
    {
        "base_reserve": 10000000,
        "gas_price": 1000
    },
    "validators": ["buQBwe7LZYCYHfxiEGb1RE9XC9kN2qrGXWCY", "buQqkp5SDcsxpwWXQ2QFQbvHKnZ199HY3dHm"] 
    //Configure all validation nodes's addresses, and if there are two, two addresses are configured.
}
```

## Operation and Maintenance
### Service Start and Stop
```
Start    :service bumo start
Stop    :service bumo stop
Running status:service bumo status

 #The MacOS does not have a service.
```
### Get Modules Status

```bash
[root@centos7x64-201 ~]# curl 127.0.0.1:36002/getModulesStatus
{
    "glue_manager":{
        "cache_topic_size":0,
        "ledger_upgrade":{
            "current_states":null,
            "local_state":null
        },
        "system":{
            "current_time":"2017-07-20 10:32:22", //current time
            "process_uptime":"2017-07-20 09:35:06", //starting time of bumo 
            "uptime":"2017-05-14 23:51:04"
        },
        "time":"0 ms",
        "transaction_size":0
    },
    "keyvalue_db":Object{...},
    "ledger_db":Object{...},
    "ledger_manager":{
        "account_count":2316,  //count of accounts
        "hash_type":"sha256",
        "ledger_sequence":12187,
        "time":"0 ms",
        "tx_count":1185   //count of transactions
    },
    "peer_manager":Object{...},
    "web server":Object{...},

```

### Drop Database
```bash
buchain/bin/bumo --dropdb
```
### Create Hard Fork
```bash
buchain/bin/bumo --create-hardfork
buchain/bin/bumo --clear-consensus-status
```
- As the node has joined an existing blockchain that would like to run a standalone blockchain, a hard fork can be created with the commands above.  After hard fork, there is only one validation node(local node)on the new blockchain.

- After executing the hard fork command, you will get a hash as following:
```bash
Create hard fork ledger successful, seq(20), consensus value hash(**7aa332f05748e6ce9ad3d059c959a50675109bcaf0a4ba2c5c6adc6418960197**)
```
- In the local node or synchronized node, add the hash with parameter `hardfork_points` into `bumo.json`.

```json
    "ledger": {
       	"genesis_account": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3",
        "max_trans_per_ledger": 1000,
        "hardfork_points" : 
        [
        	"7aa332f05748e6ce9ad3d059c959a50675109bcaf0a4ba2c5c6adc6418960197"
        ]
    },
```

- Start service
