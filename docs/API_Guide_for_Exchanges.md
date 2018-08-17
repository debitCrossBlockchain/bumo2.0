# API Guide for Exchanges

## Overview

This document is for exchanges to install the BUMO node, and use the BUMO SDK.

## BUMO Node Installation

Support for most operating systems such as Ubuntu, Centos, etc. It is recommended to use Ubuntu 14.04 or Centos 7. This example uses Ubuntu 14.04 as an example of a node installation.

### Installing Dependencies

You need to install the dependencies required by the system before compiling the source code for BUMO. To install dependencies, you need to complete the following steps:


1.Input the following command to install `automake`.

```
sudo apt-get install automake
```

2.Input the following command to install `autoconf`.

```
sudo apt-get install autoconf
```

3.Input the following command to install `libtool`.

```
sudo apt-get install libtool
```

4.Input the following command to install `g++`.

```
sudo apt-get install g++
```

5.Input the following command to install `libssl-dev`.

```
sudo apt-get install libssl-dev
```

6.Input the following command to install `cmake`.

```
sudo apt-get install cmake
```

7.Input the following command to install `libbz2-dev`.

```
sudo apt-get install libbz2-dev
```

8.Input the following command to install `python`.

```
sudo apt-get install python
```

9.Input the following command to install `unzip`.

```
sudo apt-get install unzip
```
### Compling Source Code

The source code of BUMO can be compiled after the dependencies are successfully installed. To compile the source code of BUMO, you need to complete the following steps:


1.Download the BUMO source code file by inputting the following command in the root directory. If `git` is not installed, you can install git with the `sudo apt-get install git` command.


```
git clone https://github.com/bumoproject/bumo.git
```

![](/images/download bumo_back2.png)


**Note**: The bumo/ directory will be created automatically during the BUMO source code being downloaded, and the source code files will be stored in this directory.

2.Input the following command to enter the directory where the source code files are located.

```
cd /bumo/build/
```

3.Input the following command to download the dependencies and initialize the development environment.

```
./install-build-deps-linux.sh
```

4.Input the following command to return to the bumo/ directory.

```
cd ../
```

5.Input the following command to complete the compilation of the BUMO source code. The message shown below shows that the compilation is successful.


```
make
```

![](/image/compile_finished.png)


**Note**: The executable files **bumo** and **bumod** generated after compilation are stored in the /bumo/bin directory.


### Installing the BUMO Node

The BUMO node can be installed after the compilation is completed. To install the BUMO node, you need to complete the following steps:

1.Input the following command to enter the installation directory.


```
cd /bumo/
```

2.Input the following command to complete the installation. The message below shows that the installation is successful.


```
make install
```

![](/image/compile_installed.png)


**Note**:

* By default the service is installed in the /usr/local/buchain/ directory.
* After the installation is complete, you can start the bumo service with the `service bumo start` command without additional configuration.
* After installing the BUMO node, the directory structure in the buchain/ directory is as follows:

| Directory|Description|
|----------|---------|
|bin|The directory stores the executable file (compiled bumo executable)|
|jslib|	The directory stores the third-party `js` library|
|config|The configuration file directory contains: bumo.json|
|data|The database directory stores account ledger data|
|scripts|The directory stores scripts to start and stop the node|
|log|The directory stores logs|

### Changing the Operating Environment

You need to stop the BUMO service before changing the operating environment of BUMO. You can modify it by following the steps below:


1.Input the following command to enter the configuration file directory.

```
cd /usr/local/buchain/config/
```

**Note**: The following runtime environment configuration files are provided in this directory.

* Bumo-mainnet.json (This file is the configuration file of the main network environment, which is applied in the production environment)
* bumo-testnet.json (This file is the configuration file applied in the test network environment)

* bumo-single.json (This file is a configuration file applied in a single-node debugging environment)

2.Change the configuration file (bumo.json) of the current running environment to another name, for example:

```
mv bumo.json bumoprevious.json
```

3.Change the environment configuration file to run to bumo.json, for example:

```
mv bumo-mainnet.json bumo.json
```

**Note**:

* In this example, the main network environment is set to the running environment.

* After changing the operating environment, you need to [Clear Database] (#Clear Database) to restart the bumo service.

## DevOps Services

How to start, stop, query the service and the system, as well as how to clear the database are described in this section.

**Starting BUMO Service**

Input the following command to start the bumo service.

```
service bumo start
```
**Stopping BUMO Service**

Input the following command stop the bumo service.

```
service bumo stop
```
**Querying BUMO Service Status**

Input the following command to query the bumo service.

```
service bumo status
```
**Querying System Status**

Input the following command to query the detailed status of the system:

```
curl 127.0.0.1:19333/getModulesStatus
```

The following response is received:

```
{
"glue_manager":{
"cache_topic_size":0,
"ledger_upgrade":{
"current_states":null,
"local_state":null
},
"system":{
"current_time":"2017-07-20 10:32:22", //Current system time
"process_uptime":"2017-07-20 09:35:06", //When bumo was started
"uptime":"2017-05-14 23:51:04"
},
"time":"0 ms",
"transaction_size":0
},
"keyvalue_db":Object{...},
"ledger_db":Object{...},
"ledger_manager":{
"account_count":2316, //Total accounts
"hash_type":"sha256",
"ledger_sequence":12187,
"time":"0 ms",
"tx_count":1185 //Total transactions
},
"peer_manager":Object{...},
"web server":Object{...},
```
### Clearing Database

You need to stop the BUMO service before clearing the data. To clear the database, you need to complete the following steps:


1.Input the following command to enter the bumo service directory.

```
/usr/local/buchain/bin
```
2.Input the following command to clear the database.

```
./bumo --dropdb
```
**Note**: After the database is successfully cleared, you can see the information shown below.

![](/image/clear_database.png)

## JAVA SDK Usage

The use of the JAVA SDK includes [Generating User Recharge Address](#generating-user-recharge-addresses), [Checking the Legality of Account Addresses](#checking-the-legality-of-account-addresses), and [Asset Transactions](#asset-transactions).


### Generating User Recharge Addresses

The exchange needs to generate a recharge address for each user. The exchange can create the user's recharge address through `Keypair.generator()` provided in Bumo-sdk-java. The specific example is as follows:


![](/image/BU-Ex-API-JAVA-v1.0.jpg)

Teturned value is shown below:

![](/assets/2.jpg)

### Checking the Legality of Account Addresses

Check the validity of the account address by the code shown below.

![](/image/3.jpg)

**Note**:

* If the return value is true, the account address is legal.
* If the return value is false, the account address is illegal.


### Asset Transactions

In the BUMO network, a block is generated every 10 seconds, and each transaction only needs one confirmation to get the final state of the transaction. In this section, we will introduce [Detecting User Recharge](#detecting-user-recharge), [User Cash or Transfer] (#User withdrawal or transfer) and [Query transaction] (#Query transaction).


#### Detecting User Recharge

The exchange needs to monitor block generation, and then parse the transaction records in the block to confirm the user's recharge behavior. The specific steps are as follows:


1.Make sure that the node block status is normal.

2.Analyze the transactions contained in the block (for parsing methods, see parsing block transactions).

3.Record the results after parsing.


**Viewing the Block Status**

View the block status by the code shown below.

![](/image/4.jpg)

**Note**:

* If the return value is true, the block is normal.
* If the return value is false, the block is abnormal.

**Parsing Transactions in the Block**

The exchange can query the transactions in the block according to the block height, and then analyze each transaction.

Example of request:

![](/image/5.jpg)

The response message is shown below:
![](/image/1.png)
![](/image/2.png)
![](/image/3.png)

**Note**:

* For how to use Bumo-sdk-java, visit the following link:

https://github.com/bumoproject/bumo-sdk-java/tree/release2.0.0

* For an example of API guide for the exchange, visit the following link:

https://github.com/bumoproject/bumo-sdk-java/blob/release2.0.0/examples/src/main/java/io/bumo/sdk/example/ExchangeDemo.java

#### Withdrawing or Transferring BU by Users

For BU withdrawal operations, refer to the transfer example provided by bumo-sdk-java as follows:


![](/image/6.jpg)

**Note**:

* Record the hash value of the BU withdrawal operation to view the final result of the BU withdrawal operation
* The current (2018-04-23) lowest value of gasPrice is 1000MO
* It is recommended to fill in 1000000 MO for feeLimit, which equals to 0.01BU

#### Querying Transactions

The final result of the BU withdrawal operation can be queried by the hash value returned when the BU withdrawal operation is initiated.

The call example is as follows:

![](/image/7.jpg)

![](/image/4.png)
![](/image/5.png)

## BU-Explorer

BUMO provides a blockchain data browsing tool for users to query block data.


You can visit the following link to query blockchain data:

* Testnet: http://explorer.bumotest.io
* Mainnet: http://explorer.bumo.io

## BUMO Wallet

BUMO provides a full-node wallet for Windows and Mac, allowing users to manage their private keys, view BU transfers, and sign transactions offline.

You can download the BUMO wallet by the following link:

https://github.com/bumoproject/bumo-wallet/releases

## FAQ

**Node start of BUChain command**

Q: Do I need to start the node when using the BUChain command line?

A: No.

**Are the value of gas_price and fee_limit fixed**

Q: Are Gas_price fixed at 1000MO and fee_limit fixed at 1000000MO?

A: They are not fixed. But at present (2018-04-23) gas_price is 1000MO, the larger the gas_price is, the higher the priority for transactions to be packaged. The fee_limit is the maximum transaction fees for the blockchain when the transaction is initiated. If the transaction is legal, the actual fees charged are less than the fee_limit filled by the caller. (gas_price can be obtained from the result.fees.gas_price field in the query result via the following link:

 [http://seed1.bumo.io:16002/getLedger?with_fee=true](http://seed1.bumo.io:16002/getLedger?with_fee=true)


**Transfer account balance**

Q: Can I transfer all the balance from my account?

A: No. In order to prevent DDOS attacks, and prevent creating a large number of spam accounts, the activated accounts of BUMO must reserve a certain amount of BU, currently at 0.1 BU (it can be obtained from the result.fees.base_reserve field in the query result via the following link:

 [http://seed1.bumo.io:16002/getLedger?with_fee=true](http://seed1.bumo.io:16002/getLedger?with_fee=true) 






