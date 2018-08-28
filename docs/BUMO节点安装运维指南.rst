BUMO节点安装运维指南
==================

概要
----

本文档将指导您如何在Linux环境和Mac环境下安装并配置BUMO节点。

您可以参考以下安装流程图进行安装。

|image0|

系统要求
-------

在安装BUMO节点之前需要确保您的系统满足以下条件。

**硬件要求**

硬件要求至少满足以下配置：

- **推荐配置**：CPU 8 核，内存 32G，带宽 20M， SSD 磁盘500G
- **最低配置**：CPU 4 核，内存 16G，带宽 10M， SSD 磁盘500G

**软件要求**

系统软件可选择Ubuntu、Centos或者MacOS。

- Ubuntu 14.04
- Centos7
- Mac OS X 10.11.4

在linux下安装BUMO节点
--------------------

在Linux系统中支持两种安装方式：`编译安装`_ 和 `安装包安装`_ 。

.. note:: |
   - 本安装文档中使用root账号下的根目录作为安装目录。用户可选择自己的安装目录。
   - 在安装BUMO节点之前需要确保设备的网络连接正常。


编译安装
~~~~~~~

编译安装是指先将BUMO节点的源代码编译成计算机能识别的机器码然后再进行安装。编译安装由三部分构成：`安装依赖`_ ，`编译BUMO源代码`_ ，`安装BUMO节点`_。


安装依赖
^^^^^^^

在编译BUMO节点的源代码之前需要安装系统所需的依赖。安装依赖需要完成以下步骤：

1. 输入以下命令安装 ``automake``。

::

  sudo apt-get install automake


2. 输入以下命令安装 ``autoconf``。

::

  sudo apt-get install autoconf


3. 输入以下命令安装 ``libtool``。

::

  sudo apt-get install libtool


4. 输入以下命令安装 ``g++``。

::

  sudo apt-get install g++


5. 输入以下命令安装 ``libssl-dev``。

::
 
  sudo apt-get install libssl-dev


6. 输入以下命令安装 ``cmake``。

:: 

  sudo apt-get install cmake


7. 输入以下命令安装 ``libbz2-dev``。

::

  sudo apt-get install libbz2-dev


8. 输入以下命令安装 ``python``。

::

  sudo apt-get install python


9. 输入以下命令安装 ``unzip``。

:: 

  sudo apt-get install unzip


编译BUMO源代码
^^^^^^^^^^^

在成功安装依赖后才能编译BUMO的源代码。编译BUMO节点的源代码需要完成以下步骤：

1. 在根目录下输入以下命令下载BUMO的源代码文件。如果没有安装 ``git``，可以通过 ``sudo apt-get install git`` 命令来安装 ``git``。

::

  git clone https://github.com/bumoproject/bumo.git


|image1|


.. note:: 在BUMO的源代码下载过程中将自动创建bumo/目录，源代码文件将存放到该目录下。

2. 输入以下命令进入到源代码的文件目录。

::

  cd /bumo/build/


3. 输入以下命令下载依赖并初始化开发环境。

::
  
  ./install-build-deps-linux.sh


4. 输入以下命令回到bumo/目录下。

::

  cd ../


5. 输入以下命令完成BUMO源代码的编译。出现下图所示信息则表示编译成功。

::
 
  make


|image2|


.. note:: 编译完成后生成的可执行文件 **bumo** 和 **bumod** 存放在/bumo/bin目录下。


安装BUMO节点
^^^^^^^^^^^

在编译完成后才能安装BUMO节点。安装BUMO节点需要完成以下步骤：

1. 输入以下命令进入到安装目录。

::

  cd /bumo/


2. 输入以下命令完成安装。出现下图所示信息则表示安装成功。

::
  
  make install


|image3|


.. note:: | 
   - 默认情况下服务安装在/usr/local/buchain/目录下。
   - 安装完成后无需其他配置即可通过 ``service bumo start`` 命令来启动bumo服务。
   - 安装完BUMO节点后在buchain/目录下有如下目录结构：

=============   ===================================
目录             说明
-------------   -----------------------------------
bin              存放可执行文件（编译后的bumo可执行程序）
jslib            存放第三方js库
config           配置文件目录包含：bumo.json
data             数据库目录，存放账本数据
scripts          启停脚本目录
log              运行日志存储目录
=============   ===================================


安装包安装
~~~~~~~~~

安装包安装是指以安装包的方式来安装BUMO节点。通过安装包安装BUMO节点由五部分构成：`获取安装包并解压`_ 、`注册服务`_ 、`修改服务启动路径`_ 、`设置开机启动`_ 、`选择运行环境的配置文件`_。

获取安装包并解压
^^^^^^^^^^^^^^^

获取BUMO的安装包并解压安装文件需要完成以下步骤。

1. 输入以下命令下载BUMO的安装包。

::

  wget https://github.com/bumoproject/bumo/releases/download/1.0.0.6/buchain-1.0.0.6-linux-x64.tar.gz

.. note:: |

   - 如果您没有安装wget，可以用 ``apt-get install wget`` 命令来装 ``wget``。
   - 您可以在 https://github.com/bumoproject/bumo/releases 链接上找到需要的版本，然后右键单击该版本复制下载链接。
   - 在本示例中文件下载到根目录下。

2. 输入以下命令把安装包拷贝到/usr/local/目录下。

::

  cp buchain-1.0.0.6-linux-x64.tar.gz /usr/local/


.. note:: 以上拷贝操作是在文件下载目录下完成的。您需根据具体的下载目录来拷贝文件。

3. 输入以下命令进入到 /usr/local/目录下。

::

  cd /usr/local/


4. 输入以下命令解压文件。

::

  tar -zxvf buchain-1.0.0.6-linux-x64.tar.gz


.. note:: 解压完成后得到buchain/目录。


注册服务
^^^^^^^

文件解压后需要注册bumo和bumod的服务。注册服务需要完成以下步骤：

1. 输入以下命令注册bumo的服务。

::

  ln -s /usr/local/buchain/scripts/bumo /etc/init.d/bumo


2. 输入以下命令注册bumod的服务。

::
 
  ln -s /usr/local/buchain/scripts/bumod /etc/init.d/bumod


修改服务启动路径
^^^^^^^^^^^^^^^

修改bumo和bumod的启动路径需要完成以下步骤：

1. 在local/目录下输入以下命令打开bumo文件。

::

  vim buchain/scripts/bumo


2. 找到install_dir并更改bumo的安装目录。

::

  install_dir=/usr/local/buchain


|image4|

.. note:: 默认情况下install_dir的目录在/usr/local/buchain下；您可以根据bumo的具体安装目录来修改。

3. 单击 ``Esc`` 键退出编辑。

4. 输入 ``:wq`` 保存文件。

5. 在local/目录下输入以下命令打开bumod文件。

::

  vim /buchain/scripts/bumod


6. 找到install_dir并更改bumod的安装目录。

::

  install_dir=/usr/local/buchain


.. note:: 默认情况下install\_dir的目录在/usr/local/buchain下；您可以根据bumod的具体安装目录来修改。

7. 单击 ``Esc`` 键退出编辑。

8. 输入 ``:wq`` 保存文件。


设置开机启动
^^^^^^^^^^^

设置开机启动包括设置启动级别，添加启动命令和修改文件权限。设置开机启动需要完成以下步骤：

1. 输入以下命令设置1级。

::
  
  ln -s -f /etc/init.d/bumod /etc/rc1.d/S99bumod

2. 输入以下命令设置2级。

::
 
  ln -s -f /etc/init.d/bumod /etc/rc2.d/S99bumod
  
3. 输入以下命令设置3级。

::

  ln -s -f /etc/init.d/bumod /etc/rc3.d/S99bumod

4. 输入以下命令设置4级。

::
 
  ln -s -f /etc/init.d/bumod /etc/rc4.d/S99bumod

5. 输入以下命令设置5级。

::
  
  ln -s -f /etc/init.d/bumod /etc/rc5.d/S99bumod

6. 输入以下命令打开rc.local文件。

::

  vim /etc/rc.local


7. 在rc.local文件末尾追加以下命令。

::

  /etc/init.d/bumod start

|image5|

8. 单击 ``Esc`` 键退出编辑。

9. 输入 ``:wq`` 命令保存文件。

10. 执行以下命令设置rc.local文件的权限。

::
  
  chmod +x /etc/rc.local


.. note:: 至此就完成了BUMO节点的安装。在启动bumo服务之前还需要 `选择运行环境的配置文件`_ 。


选择运行环境的配置文件
^^^^^^^^^^^^^^^^^^^^^^

在安装完BUMO节点后需要选择运行环境的配置文件才能启动bumo服务。选择运行环境的配置文件需要完成以下步骤：

1. 输入以下命令进入到配置文件目录。

::
  
  cd /usr/local/buchain/config/


.. note:: | 在该目录下提供了以下运行环境的配置文件。

  - bumo-mainnet.json：该文件是主网环境的配置文件应用在生产环境中
  - bumo-testnet.json：该文件是测试网环境的配置文件
  - bumo-single.json：该文件是单节点调试环境的配置文件

2. 输入以下命令重命名运行环境的配置文件。

::

  mv bumo-testnet.json bumo.json

.. note:: |
   - 本示例中选取了测试网环境作为运行环境。您也可以根据自己的需要选取其他文件作为运行环境。
   - 重命名文件完成后可以通过 ``service start bumo`` 来启动bumo服务。
   - 安装完BUMO节点后可以在buchain/目录下查看安装文件的目录结构。

在MacOS下安装BUMO节点
--------------------

.. _编译安装-2:

编译安装
~~~~~~~

编译安装是指先将BUMO节点的源代码编译成计算机能识别的机器码然后再进行安装。编译安装由三部分构成：安装依赖 ，编译BUMO源代码 ，安装BUMO节点。

安装Xcode
^^^^^^^^^

安装Xcode需要完成以下步骤：

1. 单击 `登录苹果软件下载官网 <https://idmsa.apple.com/IDMSWebAuth/login?appIdKey=891bd3417a7776362562d2197f89480a8547b108fd934911bcbea0110d07f757&path=%2Fdownload%2Fmore%2F&rv=1>`_
2. 输入 ``Apple ID`` 和 ``Password``。
3. 单击 ``Sign in``，进入下载页面。 
4. 单击 ``Xcode 9.4.1``，开始下载 ``Xcode``。
5. 解压 ``Xcode_9.4.1.xip``。
6. 双击解压出来的文件 ``Xcode``完成安装。

安装Command Line Tools
^^^^^^^^^^^^^^^^^^^^^^

安装 ``Command Line Tools`` 需要完成以下步骤：

1. 单击 `登录苹果软件下载官网 <https://idmsa.apple.com/IDMSWebAuth/login?appIdKey=891bd3417a7776362562d2197f89480a8547b108fd934911bcbea0110d07f757&path=%2Fdownload%2Fmore%2F&rv=1>`_
2. 输入 ``Apple ID`` 和 ``Password``。
3. 单击 ``Sign in``，进入下载页面。 
4. 单击 ``Command Line Tools(macOS 10.14)for Xcode 10 Beta 6``，开始下载 ``Command Line Tools``。
5. 双击 ``Command_Line_Tools_macOS_10.14_for_Xcode_10Beta_6.dmg``。
6. 单击 ``Command Line Tools`` 图标。
7. 单击 **继续**
8. 选择语言，然后单击 **继续**。
9. 单击 **同意**。
10. 单击 **安装**。
11. 输入密码并单击 **安装软件**。

安装Homebrew
^^^^^^^^^^^^

安装Homebrew需完成以下步骤：

1. 打开mac的终端。
2. 在终端中输入以下代码：

::
 
 /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

3. 按下 ``Enter`` 键，进行安装。

安装依赖
^^^^^^^^

1. 输入以下命令设置 ``Homebrew`` 无自动更新。

::

  export HOMEBREW_NO_AUTO_UPDATE=true

2. 输入以下命令安装 ``autoconf``。

 ::

   brew install autoconf

3. 输入以下命令安装 ``automake``。

 ::

   brew install automake

4. 输入以下命令安装 ``libtool``。

::

  brew install libtool

5. 输入以下命令安装 ``cmake``。

::
  
  brew install cmake

6. 输入以下命令安装 ``python``。

::
  
  brew install python

7. 输入以下命令安装 ``m4``。

::

  brew install m4

8. 输入以下命令安装 ``wget``。

::
  
  brew install wget

编译BUMO源代码
^^^^^^^^^^^^^
1. 在根目录下输入以下命令下载BUMO的源代码文件。如果没有安装 ``git``，可以通过 ``sudo apt-get install git`` 命令来安装 ``git``。

::
  
  sudo git clone https://github.com/bumoproject/bumo.git

|image1|


.. note:: 在BUMO的源代码下载过程中将自动创建bumo/目录，源代码文件将存放到该目录下。

2. 输入以下命令进入到源代码的文件目录。

::
  
  cd /bumo/build/

3. 输入以下命令下载依赖并初始化开发环境。

::
  
  sudo ./install-build-deps-mac.sh


4. 输入以下命令回到bumo/目录下。

::

  cd ../


5. 输入以下命令完成BUMO源代码的编译。

::
 
  sudo make


.. note:: 编译完成后生成的可执行文件 **bumo** 和 **bumod** 存放在/bumo/bin目录下。

安装BUMO节点
^^^^^^^^^^^
在编译完成后才能安装BUMO节点。安装BUMO节点需要完成以下步骤：

1. 输入以下命令进入到安装目录。

::

  cd /bumo/


2. 输入以下命令完成安装。

::
  
  make install


.. note:: | 
   - 默认情况下服务安装在/usr/local/buchain/目录下。
   - 安装完BUMO节点后在buchain/目录下有如下目录结构：

=============   ======================================================
目录             说明
-------------   ------------------------------------------------------
bin              存放可执行文件（编译后的bumo可执行程序）
config           配置文件目录包含：bumo.json
data             数据库目录，存放账本数据
jslib            存放第三方js库
scripts          启停脚本目录
log              运行日志存储目录（该目录在运行BUMO节点后才会出现）
=============   ======================================================

.. _安装包安装-2:

安装包安装
~~~~~~~~~

安装包安装是指以安装包的方式来安装BUMO节点。以安装包的方式来安装BUMO节点包括两个步骤：`获取安装包并解压`_ 、`选择运行环境的配置文件`_。

.. _获取安装包-2:

获取安装包并解压
^^^^^^^^^^^^^^

1. 从以下地址下载需要的安装包。

::

  sudo wget https://github.com/bumoproject/bumo/releases/download/1.0.0.6/buchain-1.0.0.6-linux-x64.tar.gz

.. note:: |

   - 如果您没有安装wget，可以用 ``apt-get install wget`` 命令来装 ``wget``。
   - 您可以在 https://github.com/bumoproject/bumo/releases 链接上找到需要的版本，然后右键单击该版本复制下载链接。
   - 在本示例中文件下载到根目录下。

2. 输入以下命令把安装包拷贝到/usr/local/目录下。

::

  sudo cp buchain-1.0.0.6-linux-x64.tar.gz /usr/local/


.. note:: 以上拷贝操作是在文件下载目录下完成的。您需根据具体的下载目录来拷贝文件。

3. 输入以下命令进入到 /usr/local/目录下。

::

  cd /usr/local/


4. 输入以下命令解压文件。

::

  sudo tar -zxvf buchain-1.0.0.6-linux-x64.tar.gz


.. note:: 解压完成后得到buchain/目录。

=============   ======================================================
目录             说明
-------------   ------------------------------------------------------
bin              存放可执行文件（编译后的bumo可执行程序）
config           配置文件目录包含：bumo.json
data             数据库目录，存放账本数据
jslib            存放第三方js库
log              运行日志存储目录（该目录在运行BUMO节点后才会出现）
=============   ======================================================

.. _选择运行环境的配置文件-2:

选择运行环境的配置文件
^^^^^^^^^^^^^^^^^^^^^^

在安装完BUMO节点后需要选择运行环境的配置文件才能启动bumo服务。选择运行环境的配置文件需要完成以下步骤：

1. 输入以下命令进入到配置文件目录。

::
  
  cd /usr/local/buchain/config/


.. note:: | 在该目录下提供了以下运行环境的配置文件。

  - bumo-mainnet.json：该文件是主网环境的配置文件应用在生产环境中
  - bumo-testnet.json：该文件是测试网环境的配置文件
  - bumo-single.json：该文件是单节点调试环境的配置文件

2. 输入以下命令重命名运行环境的配置文件。

::

  mv bumo-testnet.json bumo.json

.. note:: |
   - 本示例中选取了测试网环境作为运行环境。您也可以根据自己的需要选取其他文件作为运行环境。
   - 重命名文件完成后进入到 /usr/local/buchain/bin 目录下，通过 ``./bumo`` 命令来启动bumo服务。
   - 安装完BUMO节点后可以在buchain/目录下查看安装文件的目录结构。


配置
----

配置分为 `通用配置`_ 和 `多节点配置示例`_ 。


通用配置
~~~~~~~

普通配置包括了存储数据、节点间通信、WEB API、WebSocket API、区块、创世区块（genesis）以及日志的配置。通用配置在/usr/local/buchain/config目录下的bumo.json文件中进行配置。

**存储数据**

::
 
   "db":{
   "account_path": "data/account.db", //存储账号数据
   "ledger_path": "data/ledger.db", //存储区块数据
   "keyvalue_path": "data/keyvalue.db" //存储共识数据
   }


**节点间网络通信**

::

   "p2p":
   {
   "network_id":30000,//网络 ID
   //共识网络
   "consensus_network":
   {
   "heartbeat_interval":60, //心跳周期，秒
   "listen_port":36001,//已监听的端口
   "target_peer_connection":50, //最大主动连接节点数
   "known_peers":
   [
   "127.0.0.1:36001"//连接其他节点
   ]
   }
   }


**WEB API 配置**

::

   "webserver":{
   "listen_addresses":"0.0.0.0:16002"
   }


**WebSocket API 配置**

::

   "wsserver":
   {
   "listen_address":"0.0.0.0:36003"
   }


**区块配置**

::

   "ledger":
   {
   "validation_address":"buQmtDED9nFcCfRkwAF4TVhg6SL1FupDNhZY",//验证节点地址，同步节点或者钱包不需要配置
   "validation_private_key": "e174929ecec818c0861aeb168ebb800f6317dae1d439ec85ac0ce4ccdb88487487c3b74a316ee777a3a7a77e5b12efd724cd789b3b57b063b5db0215fc8f3e89", //验证节点私钥，同步节点或者钱包不需要配置
   "max_trans_per_ledger":1000, //单个区块最大交易个数
   "tx_pool": //交易池配置
   {
   "queue_limit":10240, //交易池总量限制
   "queue_per_account_txs_limit":64 //单个账号的交易缓冲最大值
   }
   }


.. note:: validation\_address 和 validation\_private\_key 可以通过 bumo 程序命令行工具获得，请妥善保存该账号信息，一旦丢失将无法找回。

::

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


**创世区块**

::

   "genesis":
   {
   "account": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3", //创世区块地址
   "slogan" : "a new era of value", //存储在创世区块中的标语
   "fees":
   {
   "base_reserve": 10000000, //账号最低预留费
   "gas_price": 1000 //字节费
   },
   "validators": ["buQBwe7LZYCYHfxiEGb1RE9XC9kN2qrGXWCY"] //验证节点区块列表
   }


.. note:: 同一个区块链上的 ``genesis`` 配置，必须保持一致。``account`` 可以通过 bumo 程序命令行工具 ``./bumo --create-account`` 获取，请妥善保存该账号信息，一旦丢失将无法找回。

**日志配置**

::

   "logger":
   {
   "path":"log/buchain.log", // 日志目录
   "dest":"FILE|STDOUT|STDERR", //输出文件分类
   "level":"TRACE|INFO|WARNING|ERROR|FATAL",//日志级别
   "time_capacity":1, //时间容量，天
   "size_capacity":10, //大小容量，兆
   "expire_days":10 //清理日志周期，天
   }


多节点配置示例
~~~~~~~~~~~~~

本章节以两个验证节点和一个同步节点为例，介绍多节点在同一条区块链的配置，其中需要修改 p2p、ledger和genesis 这三个模块。

**p2p模块配置**

p2p 的 known_peers 必须为其他已知节点的 IP 和端口，用于节点之间相互连接。


::

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
   "192.168.1.102:36001", //节点二的 IP 和端口
   "192.168.1.103:36001" //节点三的 IP 和端口
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
   "192.168.1.101:36001", //节点一的 IP 和端口
   "192.168.1.103:36001" //节点三的 IP 和端口
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
   "192.168.1.101:36001", //节点一的 IP 和端口
   "192.168.1.102:36001" //节点二的 IP 和端口
   ]
   }
   }

**leger模块配置**

验证节点的 ledger 的 validation_address 和 validation_private_key 必须要匹配。并且需要把所有验证节点的 validation_address 填写到 genesis.validators 里。

::

   验证节点一：
   "ledger":
   {
   "validation_address":"buQBwe7LZYCYHfxiEGb1RE9XC9kN2qrGXWCY",//验证节点一的地址，同步节点或者钱包不需要配置
   "validation_private_key": "66932f19d5be465ea9e7cfcb3ea7326d81953b9f99bc39ddb437b5367937f234b866695e1aae9be4bae27317c9987f80be882ae3d2535d4586deb3645ecd7e54", //验证节点二的私钥，同步节点或者钱包不需要配置
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
   "validation_address":"buQqkp5SDcsxpwWXQ2QFQbvHKnZ199HY3dHm",//验证节点二的地址，同步节点或者钱包不需要配置
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

**genesis模块配置**

同一个区块链上的 genesis 配置，必须保持一致。

::

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
   "validators": ["buQBwe7LZYCYHfxiEGb1RE9XC9kN2qrGXWCY", "buQqkp5SDcsxpwWXQ2QFQbvHKnZ199HY3dHm"] //需要配置所有的验证节点地址，如果有两个验证节点，则配置两个地址。
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
   "validators": ["buQBwe7LZYCYHfxiEGb1RE9XC9kN2qrGXWCY", "buQqkp5SDcsxpwWXQ2QFQbvHKnZ199HY3dHm"] //需要配置所有的验证节点地址，如果有两个验证节点，则配置两个地址。
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
   "validators": ["buQBwe7LZYCYHfxiEGb1RE9XC9kN2qrGXWCY", "buQqkp5SDcsxpwWXQ2QFQbvHKnZ199HY3dHm"] //需要配置所有的验证节点地址，如果有两个验证节点，则配置两个地址。
   }

.. note:: |
   - 运行前请确保每个节点的初始数据一致，否则无法达成共识产生区块。
   - account，validation_address 可以通过 bumo 程序命令行工具 ``./bumo --create-account`` 获取，请妥善保存该账号信息，一旦丢失将无法找回。

运维服务
-------

在运维服务中对BUMO服务的启动、关闭、状态查询、系统详情查询、清空数据库、创建硬分叉、更改运行环境进行了详细说明。

**启动BUMO服务**

输入以下命令启动bumo服务。

::

   service bumo start

.. note:: 在mac中启动bumo服务需要进入到/usr/local/buchain/bin目录下，然后通过 ``./bumo`` 命令在启动bumo服务。

**关闭BUMO服务**

输入以下命令关闭bumo服务。

::

   service bumo stop

.. note:: 在mac中关闭bumo服务可以通过 ``control+c`` 键来完成。

**查询BUMO服务状态**

输入以下命令查询bumo服务。

::

   service bumo status

.. note:: 在mac中没有service服务。

**查询系统详细状态**

输入以下命令查询系统详细状态：

::

   curl 127.0.0.1:19333/getModulesStatus

得到如下结果：

::

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

.. note:: 在mac中没有service服务。

**清空数据库**

在清空数据之前需要停止BUMO服务。清空数据库需要完成以下步骤：

1. 输入以下命令进入bumo的服务目录。

::

   /usr/local/buchain/bin

2. 输入以下命令清空数据库。

::

   ./bumo --dropdb

.. note:: 数据库成功清空后能看到如下所示的信息。

|image6|


**创建硬分叉**

创建硬分叉需要完成以下步骤：

1. 在/usr/local目录下输入以下命令创建硬分叉。

::

  buchain/bin/bumo --create-hardfork

2. 在提示界面上输入 ``y`` 然后单击 ``Enter`` 键。创建成功后将出现以下界面。

|image7|

.. note:: |
   - 执行完上面的命令后，新的区块链网络只有一个验证节点即本节点。
   - 执行完创建硬分叉命令后将获取如下Hash值：

::

  4b9ad78065c65aaf1280edf6129ab2da93c99c42f2bcd380b5966750ccd5d80d


3. 输入以下命令清除共识状态数据。清除共识状态数据时需要确保bumo服务没有运行，否则无法清除。

::
  
  buchain/bin/bumo --clear-consensus-status


4. 把Hash值配置到本节点或同步节点/usr/local/buchain/config目录下的bumo.json文件中。

::

  "ledger": {
  "genesis_account": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3",
  "max_trans_per_ledger": 1000,
  "hardfork_points" :
  [
  "4b9ad78065c65aaf1280edf6129ab2da93c99c42f2bcd380b5966750ccd5d80d
  "
  ]
  },

5. 启动节点服务，让配置生效。


**更改运行环境**

在更改运行环境前，需要确保BUMO服务已经关闭。如果您想更改BUMO节点的运行环境，可按照以下步骤进行修改。

1. 输入以下命令进入到配置文件目录。

::

  cd /usr/local/buchain/config/


.. note:: | 在该目录下提供了以下运行环境的配置文件。
   - bumo-mainnet.json：该文件是主网环境的配置文件，应用在生成环境中
   - bumo-testnet.json：该文件是测试网环境的配置文件
   - bumo-single.json：该文件是单节点调试环境的配置文件

2. 把当前运行环境的配置文件（bumo.json）更改为其他名称，例如：

::
  
  mv bumo.json bumoprevious.json


3. 把要运行的环境配置文件更改为bumo.json，例如：

::
  
  mv bumo-mainnet.json bumo.json

.. note:: | 
   - 本示例中把主网环境设置成了运行环境。
   - 更改运行环境后需要清空数据库才能重启bumo服务。


卸载BUMO节点
------------

卸载BUMO节点分为两类，一类是针对编译安装的卸载，另一类是针对安装包安装的卸载。


针对编译安装的卸载
~~~~~~~~~~~~~~~~

在安装完BUMO节点之后可以对安装文件进行卸载。如果是利用编译安装的BUMO节点，则可以按照以下步骤完成卸载：

1. 输入以下命令进入BUMO的安装目录。

::
  
  cd /bumo

2. 输入以下命令删除BUMO节点。

:: 
  
  make uninstall

.. note:: 至此就完成了BUMO节点的卸载。

.. |image0| image:: /docs/image/flow_diagram.png
.. |image1| image:: /docs/image/download_bumo_back2.png
.. |image2| image:: /docs/image/compile_finished.png
.. |image3| image:: /docs/image/compile_installed.png
.. |image4| image:: /docs/image/start_path.png
.. |image5| image:: /docs/image/add_start_command.png
.. |image6| image:: /docs/image/clear_database.png
.. |image7| image:: /docs/image/hard_fork_created.png








































