API Guide for Exchanges
=======================

Overview
--------

This document is for exchanges to install the BUMO node, and use the
BUMO SDK.

BUMO Node Installation
----------------------

Support for most operating systems such as Ubuntu, Centos, etc. It is
recommended to use Ubuntu 14.04 or Centos 7. This example uses Ubuntu
14.04 as an example of a node installation.

Installing Dependencies
~~~~~~~~~~~~~~~~~~~~~~~

You need to install the dependencies required by the system before
compiling the source code for BUMO. To install dependencies, you need to
complete the following steps:

1.Input the following command to install ``automake``.

::

   sudo apt-get install automake

2.Input the following command to install ``autoconf``.

::

   sudo apt-get install autoconf

3.Input the following command to install ``libtool``.

::

   sudo apt-get install libtool

4.Input the following command to install ``g++``.

::

   sudo apt-get install g++

5.Input the following command to install ``libssl-dev``.

::

   sudo apt-get install libssl-dev

6.Input the following command to install ``cmake``.

::

   sudo apt-get install cmake

7.Input the following command to install ``libbz2-dev``.

::

   sudo apt-get install libbz2-dev

8.Input the following command to install ``python``.

::

   sudo apt-get install python

9.Input the following command to install ``unzip``.

::

   sudo apt-get install unzip

Compiling Source Code
~~~~~~~~~~~~~~~~~~~~~

The source code of BUMO can be compiled after the dependencies are
successfully installed. If ``git`` is not installed, you can install
``git`` with the ``sudo apt-get install git`` command. To compile the
source code of BUMO, you need to complete the following steps:

1.Download the BUMO source code file by inputting the following command
in the root directory.

::

   git clone https://github.com/bumoproject/bumo.git

|image0|

**Note**: The bumo/ directory will be created automatically during the
BUMO source code being downloaded, and the source code files will be
stored in this directory.

2.Input the following command to enter the directory where the source
code files are located.

::

   cd /bumo/build/

3.Input the following command to download the dependencies and
initialize the development environment.

::

   ./install-build-deps-linux.sh

4.Input the following command to return to the bumo/ directory.

::

   cd ../

5.Input the following command to complete the compilation of the BUMO
source code. The message below shows that the compilation is successful.

::

   make

|image1|

**Note**: The executable files **bumo** and **bumod** generated after
compilation are stored in the /bumo/bin directory.

Installing the BUMO Node
~~~~~~~~~~~~~~~~~~~~~~~~

The BUMO node can be installed after the compilation is completed. To
install the BUMO node, you need to complete the following steps:

1.Input the following command to enter the installation directory.

::

   cd /bumo/

2.Input the following command to complete the installation. The message
below shows that the installation is successful.

::

   make install

|image2|

**Note**:

-  By default the service is installed in the /usr/local/buchain/
   directory.
-  After the installation is complete, you can start the bumo service
   with the ``service bumo start`` command without additional
   configuration.
-  After installing the BUMO node, the directory structure in the
   buchain/ directory is as follows:

+-----------------------------------+-----------------------------------+
| Directory                         | Description                       |
+===================================+===================================+
| bin                               | The directory stores the          |
|                                   | executable file (compiled bumo    |
|                                   | executable)                       |
+-----------------------------------+-----------------------------------+
| jslib                             | The directory stores the          |
|                                   | third-party ``js`` library        |
+-----------------------------------+-----------------------------------+
| config                            | The configuration file directory  |
|                                   | contains: bumo.json               |
+-----------------------------------+-----------------------------------+
| data                              | The database directory stores     |
|                                   | account ledger data               |
+-----------------------------------+-----------------------------------+
| scripts                           | The directory stores scripts to   |
|                                   | start and stop the node           |
+-----------------------------------+-----------------------------------+
| log                               | The directory stores logs         |
+-----------------------------------+-----------------------------------+

Changing the Operating Environment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You need to stop the BUMO service before changing the operating
environment of BUMO. You can modify it by following the steps below:

1.Input the following command to enter the configuration file directory.

::

   cd /usr/local/buchain/config/

**Note**: The following runtime environment configuration files are
provided in this directory.

-  Bumo-mainnet.json (This file is the configuration file of the main
   network environment, which is applied in the production environment)
-  bumo-testnet.json (This file is the configuration file applied in the
   test network environment)

-  bumo-single.json (This file is the configuration file applied in a
   single-node debugging environment)

2.Change the configuration file (bumo.json) of the current running
environment to another name, for example:

::

   mv bumo.json bumoprevious.json

3.Change the environment configuration file to be run to bumo.json, for
example:

::

   mv bumo-mainnet.json bumo.json

**Note**:

-  In this example, the main network environment is set to the running
   environment.

-  After changing the operating environment, you need to clear the
   database to restart the bumo service.

DevOps Services
---------------

How to start, stop, query the service and the system, as well as how to
clear the database are described in this section.

**Starting BUMO Service**

Input the following command to start the bumo service.

::

   service bumo start

**Stopping BUMO Service**

Input the following command stop the bumo service.

::

   service bumo stop

**Querying BUMO Service Status**

Input the following command to query the bumo service.

::

   service bumo status

**Querying System Status**

Input the following command to query the detailed status of the system:

::

   curl 127.0.0.1:19333/getModulesStatus

The following response message is received:

::

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

Clearing Database
~~~~~~~~~~~~~~~~~

You need to stop the BUMO service before clearing the data. To clear the
database, you need to complete the following steps:

1.Input the following command to enter the bumo service directory.

::

   /usr/local/buchain/bin

2.Input the following command to clear the database.

::

   ./bumo --dropdb

**Note**: After the database is successfully cleared, you can see the
information shown below.

|image3|

JAVA SDK Usage
--------------

The use of the JAVA SDK includes `Generating User Recharge
Address <#generating-user-recharge-addresses>`__, `Checking the Legality
of Account Addresses <#checking-the-legality-of-account-addresses>`__,
and `Asset Transactions <#asset-transactions>`__.

Generating Addresses for Users to Recharge
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The exchange needs to generate an address for each user to recharge. The
exchange can create the user’s address to recharge through
``Keypair.generator()`` provided in Bumo-sdk-java. The specific example
is as follows:

::

   /**

        * Generate an account private key, public key and address
        */
       @Test
       public void createAccount() {
           Keypair keypair = Keypair.generator();
           System.out.println(JSON.toJSONString(keypair, true));
       }

The return value is shown below:

|image4|

Checking the Legality of Account Addresses
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Check the validity of the account address by the code shown below.

::

   /**

        * Check whether the account address is valid
        */
       @Test
       public void checkAccountAddress() {
           String address = "buQemmMwmRQY1JkcU7w3nhruoX5N3j6C29uo";
           AccountCheckValidRequest accountCheckValidRequest = new AccountCheckValidRequest();
           accountCheckValidRequest.setAddress(address);
           AccountCheckValidResponse accountCheckValidResponse = sdk.getAccountService().checkValid(accountCheckValidRequest);
           if (0 == accountCheckValidResponse.getErrorCode()) {
               System.out.println(accountCheckValidResponse.getResult().isValid());
           } else {
               System.out.println(JSON.toJSONString(accountCheckValidResponse, true));
           }
       }

**Note**:

-  If the return value is true, the account address is legal.
-  If the return value is false, the account address is illegal.

Asset Transactions
~~~~~~~~~~~~~~~~~~

In the BUMO network, a block is generated every 10 seconds, and each
transaction only needs one confirmation to get its final state. In this
section, we will introduce `Detecting User
Recharging <#detecting-user-recharging>`__, `Withdrawing or Transferring
BU by Users <#withdrawing-or-transferring-bu-by-users>`__ and `Querying
Transactions <#Querying-transactions>`__.

Detecting User Recharging
^^^^^^^^^^^^^^^^^^^^^^^^^

The exchange needs to monitor block generation, and then parse the
transaction records in the block to confirm the user’s recharge
behavior. The specific steps are as follows:

1.Make sure that the node block status is normal.

2.Analyze the transactions contained in the block (for parsing methods,
see parsing transactions in the block).

3.Record the results after parsing.

**Viewing the Block Status**

View the block status by the code shown below.

::

   /**

        * Check whether the connected node is synchronous in the blockchain
        */
       @Test
       public void checkBlockStatus() {
           BlockCheckStatusResponse response = sdk.getBlockService().checkStatus();
           System.out.println(response.getResult().getSynchronous());
       }

**Note**:

-  If the return value is true, the block is normal.
-  If the return value is false, the block is abnormal.

**Parsing Transactions in the Block**

The exchange can query the transactions in the block according to the
block height, and then analyze each transaction.

Example of request:

::

   /**

        * Detect user recharge operations
        * <p>
        *Detect user recharge actions by parsing transactions in the block
        */
       @Test
       public void getTransactionOfBolck() {
           Long blockNumber = 617247L;// Block 617247
           BlockGetTransactionsRequest request = new BlockGetTransactionsRequest();
           request.setBlockNumber(blockNumber);
           BlockGetTransactionsResponse response = sdk.getBlockService().getTransactions(request);
           if (0 == response.getErrorCode()) {
               System.out.println(JSON.toJSONString(response, true));
           } else {
               System.out.println("Failure\n" + JSON.toJSONString(response, true));
           }
           //Detect whether an account has recharged BU
           // Analyze transactions[n].transaction.operations[n].pay_coin.dest_address 

           // Note:
           // Operations are arrays, there may be multiple transfer operations
       }

The response message is shown below:

::

   {
       "total_count": 1,
       "transactions": [{
           "close_time": 1524467568753121,
           "error_code": 0,
           "error_desc": "",
           "hash": "89402813097402d1983c178c5ec271c6890db40c3beb9f06db71c8d52dab6c86",
           "ledger_seq": 33063,
           "signatures": [{
               "public_key": "b001dbf0942450f5601e39ac1f7223e332fe0324f1f91ec16c286258caba46dd29f6ef9bf93b",
               "sign_data": "668984fc7ded2dd30d87a1577f78eeb34d2198de3485be14ea66d9ca18f21aa21b2e0461ad8fedefc1abcb4221d346b404e8f9f9bd9c93a7df99baffeb616e0a"
           }],
           "transaction": {
               "fee_limit": 1000000,
               "gas_price": 1000,
               "metadata": "333133323333",
               "nonce": 25,
               "operations": [{
                   "pay_coin": {
                       "amount": 3000,
                       "dest_address": "buQctxUa367fjw9jegzMVvdux5eCdEhX18ME"
                   },
                   "type": 7
               }],
               "source_address": "buQhP7pzmjoRsNG7AkhfNxiWd7HuYsYnLa4x"
           }
       }]
   }

   Details on the response message:

   total_count   The total number of transactions (generally 1)
   transactions  Query the transaction object in the block; the array size is the total number of transactions in the block
   |__ actual_fee    Transaction fees in MO
   |__close_time     Transaction time
   |__error_code     Transaction status, 0 indicates success, otherwise, failure
   |__error_desc     Transaction status information
   |__hash           Transaction hash
   |__ledger_seq     Block height
   |__signatures     Signature information
   |__public_key     Public key for the signer 
   |__sign_data      Signature data for the signer 
   |__transaction    Signature object
   |__fee_limit      Minimum fee, in MO
   |__gas_price      Gas price in MO
   |__metadata       Metadata for the transaction
   |__nonce          Transactions in the original account
   |__operations     Operation objects (multiple objects supported)
   |__pay_coin     Operation type: built-in token
   |__amount       Amount of BU transferred, in MO
   |__dest_address       Recipient address
   |__type        Operation type: 7 stands for built-in token transfer
   |__source_address  Source account address

**Note**:

-  For how to use Bumo-sdk-java, visit the following link:

https://github.com/bumoproject/bumo-sdk-java/tree/release2.0.0

-  For an example of API guide for the exchange, visit the following
   link:

https://github.com/bumoproject/bumo-sdk-java/blob/release2.0.0/examples/src/main/java/io/bumo/sdk/example/ExchangeDemo.java

Withdrawing or Transferring BU by Users
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For BU withdrawal operations, refer to the transfer example provided by
bumo-sdk-java as follows:

::

   /**
        * Send a transaction of sending bu
        *
        * @throws Exception
        */
       @Test
       public void sendBu() throws Exception {
           // Init variable
           // The account private key to send bu
           String senderPrivateKey = "privbyQCRp7DLqKtRFCqKQJr81TurTqG6UKXMMtGAmPG3abcM9XHjWvq";
           // The account address to receive bu
           String destAddress = "buQswSaKDACkrFsnP1wcVsLAUzXQsemauE";
           // The amount to be sent
           Long amount = ToBaseUnit.BU2MO("0.01");
           // The fixed write 1000L, the unit is MO
           Long gasPrice = 1000L;
           // Set up the maximum cost 0.01BU
           Long feeLimit = ToBaseUnit.BU2MO("0.01");
           // Transaction initiation account's nonce + 1
           Long nonce = 1L;

           // Record txhash for subsequent confirmation of the real result of the transaction.
           // After recommending five blocks, call again through txhash `Get the transaction information
           // from the transaction Hash'(see example: getTxByHash ()) to confirm the final result of the transaction
           String txhash = sendBu(senderPrivateKey, destAddress, amount, nonce, gasPrice, feeLimit);

       }

**Note**:

-  Record the hash value of the BU withdrawal operation to view the
   final result of the BU withdrawal operation
-  The current (2018-04-23) lowest value of gasPrice is 1000MO
-  It is recommended to fill in 1000000 MO for feeLimit, which equals to
   0.01BU

Querying Transactions
^^^^^^^^^^^^^^^^^^^^^

The final result of the BU withdrawal operation can be queried by the
hash value returned when the BU withdrawal operation is initiated.

The call example is as follows:

::

   /**
        * Get transaction information based on the transaction Hash
        */
       @Test
       public void getTxByHash() {
           String txHash = "fba9c3f73705ca3eb865c7ec2959c30bd27534509796fd5b208b0576ab155d95";
           TransactionGetInfoRequest request = new TransactionGetInfoRequest();
           request.setHash(txHash);
           TransactionGetInfoResponse response = sdk.getTransactionService().getInfo(request);
           if (0 == response.getErrorCode()) {
               System.out.println(JSON.toJSONString(response, true));
           } else {
               System.out.println("Failure\n" + JSON.toJSONString(response, true));
           }
       }

::

   public static void queryTransactionByHash(BcQueryService queryService) {
      String txHash = "";
      TransactionHistory tx = queryService.getTransactionHistoryByHash(txHash);
      System.out.println(tx);
   }

   Note:
   * When the number of tx.totalCount is greater than or equal to 1, the transaction history exists
   * When tx.transactions.errorCode equals 0, it indicates that the transaction is successful, otherwise the transaction is not successful. 
   * For the withdrawal operation, the exchange should pay attention to the pay_coin operation
   * Example of a complete BU withdrawal response:
   {
       "total_count": 1,
       "transactions": [{
           "close_time": 1524467568753121,
           "error_code": 0,
           "error_desc": "",
           "hash": "89402813097402d1983c178c5ec271c6890db40c3beb9f06db71c8d52dab6c86",
           "ledger_seq": 33063,
           "signatures": [{
               "public_key": "b001dbf0942450f5601e39ac1f7223e332fe0324f1f91ec16c286258caba46dd29f6ef9bf93b",
               "sign_data": "668984fc7ded2dd30d87a1577f78eeb34d2198de3485be14ea66d9ca18f21aa21b2e0461ad8fedefc1abcb4221d346b404e8f9f9bd9c93a7df99baffeb616e0a"
           }],
           "transaction": {
               "fee_limit": 1000000,
               "gas_price": 1000,
               "metadata": "333133323333",
               "nonce": 25,
               "operations": [{
                   "pay_coin": {
                       "amount": 3000,
                       "dest_address": "buQctxUa367fjw9jegzMVvdux5eCdEhX18ME"
                   },
                   "type": 7
               }],
               "source_address": "buQhP7pzmjoRsNG7AkhfNxiWd7HuYsYnLa4x"
           }
       }]
   }

   total_count   The total number of transactions (generally 1)
   transactions  Query the transaction object in the block, the array size is the total number of transactions in the block
   |__ actual_fee    Transaction fees in MO
   |__close_time     Transaction time
   |__error_code     Transaction status, 0 indicates success, otherwise, failure
   |__error_desc     Transaction status information
   |__hash           Transaction hash
   |__ledger_seq     Block height
   |__signatures     Signature information
   |__public_key     Public key for the signer 
   |__sign_data      Signature data for the signer 
   |__transaction    Signature object
   |__fee_limit     Minimum fee, in MO
   |__gas_price     Gas price, in MO
   |__metadata      Metadata for the transaction
   |__nonce         Transactions in the original account
   |__operations    Operation objects (multiple objects supported)
   |__pay_coin      Operation type: built-in token
   |__amount        Amount of BU transferred, in MO
   |__dest_address       Recipient address
   |__type         Operation type: 7 stands for built-in token transfer
   |__source_address  Source account address

BU-Explorer
-----------

BUMO provides a blockchain data browsing tool for users to query block
data.

You can visit the following links to query blockchain data:

-  Testnet: http://explorer.bumotest.io
-  Mainnet: http://explorer.bumo.io

BUMO Wallet
-----------

BUMO provides a full-node wallet for Windows and Mac, allowing users to
manage their private keys, view BU transfers, and sign transactions
offline.

You can download the BUMO wallet by the following link:

https://github.com/bumoproject/bumo-wallet/releases

FAQ
---

**Start node in BUChain command line**

Q: Do I need to start the node when using the BUChain command line?

A: No.

**Are the values of gas_price and fee_limit fixed**

Q: Are Gas_price fixed at 1000MO and fee_limit fixed at 1000000MO?

A: They are not fixed. But at present (2018-04-23) gas_price is 1000MO,
the larger the gas_price is, the higher the priority for transactions to
be packaged. The fee_limit is the maximum transaction fees for the
blockchain when the transaction is initiated. If the transaction is
legal, the actual fees charged are less than the fee_limit filled by the
caller. (gas_price can be obtained from the result.fees.gas_price field
in the query result via the following link:

http://seed1.bumo.io:16002/getLedger?with_fee=true

**Transfer account balance**

Q: Can I transfer all the balance from my account?

A: No. In order to prevent DDOS attacks, and prevent creating a large
number of spam accounts, the activated accounts of BUMO must reserve a
certain amount of BU, currently at 0.1 BU (it can be obtained from the
result.fees.base_reserve field in the query result via the following
link:

http://seed1.bumo.io:16002/getLedger?with_fee=true

.. |image0| image:: /docs/image/download_bumo_back2.png
.. |image1| image:: /docs/image/compile_finished.png
.. |image2| image:: /docs/image/compile_installed.png
.. |image3| image:: /docs/image/clear_database.png
.. |image4| image:: /docs/image/2.jpg

