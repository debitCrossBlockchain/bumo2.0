BUMO JAVA SDK
=============

概述
----


==================   =======================   ==========================================
参数                    类型                      描述 
===================  =======================   ==========================================
address               String                     账户地址 
balance               Long                       账户余额，单位MO，1BU = 10^8 MO,必须大于0 
nonce                 Long                       账户交易序列号，必须大于0
priv                  Priv                       账户权限   
===================  ======================    ==========================================

本文档对Bumo Java SDK常用的接口进行了详细说明,
使开发者能够更方便地操作和查询BU区块链。

术语
----

本章节对该文档中使用到的术语进行了详细说明。

**操作BU区块链** 

操作BU区块链是指向BU区块链写入或修改数据。

**提交交易** 

提交交易是指向BU区块链发送写入或修改数据的请求。

**查询BU区块链** 

查询BU区块链是指查询BU区块链中的数据。

**账户服务** 

账户服务提供了账户相关的有效性校验与查询接口。

**资产服务** 

资产服务提供了资产相关的查询接口，该资产遵循ATP1.0协议。

**Ctp10Token服务**

Ctp10Token服务提供了合约资产相关的有效性校验与查询接口，该资产遵循CTP1.0协议。

**合约服务** 

合约服务提供了合约相关的有效性校验与查询接口。

**交易服务**

交易服务提供了构建交易Blob接口、签名接口、查询与提交交易接口。

**区块服务** 

区块服务提供了区块的查询接口。

**账户nonce值** 

账户nonce值用于标识用户提交交易时交易执行的顺序。

请求参数与响应数据格式
----------------------

本章节将详细介绍请求参数与响应数据的格式。

请求参数
~~~~~~~~

接口的请求参数的类名，由 **服务名+方法名+Request** 构成，例如:
账户服务下的 ``getInfo`` 接口的请求参数格式是 ``AccountGetInfoRequest``。

请求参数的成员，是各个接口的入参成员。例如：账户服务下的 ``getInfo`` 接口的入参成员是 ``address``，那么该接口的请求参数的完整结构如下：

::

   Class AccountGetInfoRequest {
   String address;
   }
   
   
+-----------------------+-----------------------+-------------------=-----+
| 参数                  | 类型                   | 描述                  |
+=======================+=======================+=========================+
| address               | String                | 账户地址              |
+-----------------------+-----------------------+-------------------------+
| balance               | Long                  | 账户余额，单位MO，1   |
|                       |                       | BU = 10^8 MO,        |
|                       |                       | 必须大于0             |
+-----------------------+-----------------------+-------------------------+
| nonce                 | Long                  | 账户交易序列号，必须大于0 | 
+-----------------------+-----------------------+-------------------------+
| priv                  | Priv                  | 账户权限              |
+-----------------------+-----------------------+-------------------------+


响应数据
~~~~~~~~

接口的响应数据的类名，由 **服务名+方法名+Response**
构成，例如：账户服务下的 ``getNonce`` 接口的响应数据格式是 ``AccountGetNonceResponse``。

响应数据的成员包括错误码、错误描述和返回结果。例如，资产服务下的 ``getInfo`` 接口的响应数据的成员如下：

::

   Class AccountGetNonceResponse {
   Integer errorCode;
   String errorDesc;
   AccountGetNonceResult result;
   }

.. note:: |
       - errorCode: 错误码。0表示无错误，大于0表示有错误 
       - errorDesc: 错误描述 
       - result: 返回结果。一个结构体，其类名由 **服务名+方法名+Result** 构成，其成员是各个接口返回值的成员，例如：账户服务下的 ``getNonce`` 接口的结果类名是 ``AccountGetNonceResult`` ，成员有nonce，完整结构如下：

::

   Class AccountGetNonceResult {
   Long nonce;
   }

使用方法
--------

本章节介绍SDK的使用流程。首先需要生成SDK实现，然后调用相应服务的接口。服务包括账户服务、资产服务、Ctp1.0Token服务、合约服务、交易服务、区块服务。接口按用途分为生成公私钥地址接口、有效性校验接口、查询接口、广播交易相关接口。

生成SDK实例
~~~~~~~~~~~

生成SDK实例需调用SDK的接口 ``getInstance`` 来实现。具体调用如下所示：

::

   String url = "http://seed1.bumotest.io";
   SDK sdk = SDK.getInstance(url);

生成公私钥地址
~~~~~~~~~~~~~~

生成公私钥地址接口用于生成BU区块链账户的公钥、私钥和地址。直接调用 ``Keypair.generator`` 接口即可实现。具体调用如下所示：

::

   Keypair keypair = Keypair.generator();
   System.out.println(keypair.getPrivateKey());
   System.out.println(keypair.getPublicKey());
   System.out.println(keypair.getAddress());

有效性校验
~~~~~~~~~~

有效性校验接口用于校验信息的有效性，直接调用相应的接口即可实现。比如校验账户地址的有效性，具体调用如下所示：

::

   // 初始化请求参数
   String address = "buQemmMwmRQY1JkcU7w3nhruoX5N3j6C29uo";
   AccountCheckValidRequest request = new AccountCheckValidRequest();
   request.setAddress(address);

   // 调用checkValid接口
   AccountCheckValidResponse response =
   sdk.getAccountService().checkValid(request);
   if(0 == response.getErrorCode()) {
   System.out.println(response.getResult().isValid());
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

查询
~~~~

查询接口用于查询BU区块链上的数据，直接调用相应的接口即可实现。比如查询账户信息，具体调用如下所示：

::

   // 初始化请求参数
   String accountAddress = "buQemmMwmRQY1JkcU7w3nhruo%X5N3j6C29uo";
   AccountGetInfoRequest request = new AccountGetInfoRequest();
   request.setAddress(accountAddress);

   // 调用getInfo接口
   AccountGetInfoResponse response = sdk.getAccountService().getInfo(request);
   if (response.getErrorCode() == 0) {
   AccountGetInfoResult result = response.getResult();
   System.out.println(JSON.toJSONString(result,true));
   }
   else {
   System.out.println("error: " + response.getErrorDesc());
   }

广播交易
~~~~~~~~

广播交易是指通过广播的方式发起交易。广播交易包括以下步骤：

1. `获取交易发起的账户nonce值`_
2. `构建操作`_
3. `序列化交易`_
4. `签名交易`_
5. `提交交易`_

获取交易发起的账户nonce值
^^^^^^^^^^^^^^^^^^^^^^^^^

开发者可自己维护各个账户的nonce值，在提交完一个交易后，自动为nonce值递增1，这样可以在短时间内发送多笔交易；否则，必须等上一个交易执行完成后，账户的nonce值才会加1。具体接口调用如下所示：

::

   // 初始化请求参数
   String senderAddress = "buQnnUEBREw2hB6pWHGPzwanX7d28xk6KVcp";
   AccountGetNonceRequest getNonceRequest = new AccountGetNonceRequest();
   getNonceRequest.setAddress(senderAddress);

   // 调用getNonce接口
   AccountGetNonceResponse getNonceResponse = sdk.getAccountService().getNonce(getNonceRequest);

   // 赋值nonce
   if (getNonceResponse.getErrorCode() == 0) {
   AccountGetNonceResult result = getNonceResponse.getResult();
   System.out.println("nonce: " + result.getNonce());
   }
   else {
   System.out.println("error" + getNonceResponse.getErrorDesc());
   }

构建操作
^^^^^^^^

这里的操作是指在交易中做的一些动作，便于序列化交易和评估费用。例如，构建发送BU操作（``BUSendOperation``），具体接口调用如下所示：

::

   String senderAddress = "buQnnUEBREw2hB6pWHGPzwanX7d28xk6KVcp";
   String destAddress = "buQsurH1M4rjLkfjzkxR9KXJ6jSu2r9xBNEw";
   Long buAmount = ToBaseUnit.BU2MO("10.9");

   BUSendOperation operation = new BUSendOperation();
   operation.setSourceAddress(senderAddress);
   operation.setDestAddress(destAddress);
   operation.setAmount(buAmount);

序列化交易
^^^^^^^^^^

序列化交易接口用于序列化交易，并生成交易Blob串，便于网络传输。其中nonce和operation是上面接口得到的，具体接口调用如下所示：

::

   // 初始化变量
   String senderAddress = "buQnnUEBREw2hB6pWHGPzwanX7d28xk6KVcp";
   Long gasPrice = 1000L;
   Long feeLimit = ToBaseUnit.BU2MO("0.01");

   // 初始化请求参数
   TransactionBuildBlobRequest buildBlobRequest = new TransactionBuildBlobRequest();
   buildBlobRequest.setSourceAddress(senderAddress);
   buildBlobRequest.setNonce(nonce + 1);
   buildBlobRequest.setFeeLimit(feeLimit);
   buildBlobRequest.setGasPrice(gasPrice);
   buildBlobRequest.addOperation(operation);

   // 调用buildBlob接口
   TransactionBuildBlobResponse buildBlobResponse = sdk.getTransactionService().buildBlob(buildBlobRequest);
   if (buildBlobResponse.getErrorCode() == 0) {
   TransactionBuildBlobResult result = buildBlobResponse.getResult();
   System.out.println("txHash: " + result.getHash() + ", blob: " + result.getTransactionBlob());
   } else {
   System.out.println("error: " + buildBlobResponse.getErrorDesc());
   }

签名交易
''''''''

签名交易接口用于交易发起者使用其账户私钥对交易进行签名。其中 ``transactionBlob`` 是上面接口得到的，具体接口调用如下所示：

::

   // 初始化请求参数
   String senderPrivateKey = "privbyQCRp7DLqKtRFCqKQJr81TurTqG6UKXMMtGAmPG3abcM9XHjWvq";
   String []signerPrivateKeyArr = {senderPrivateKey};
   TransactionSignRequest signRequest = new TransactionSignRequest();
   signRequest.setBlob(transactionBlob);
   for (int i = 0; i < signerPrivateKeyArr.length; i++) {
   signRequest.addPrivateKey(signerPrivateKeyArr[i]);
   }

   // 调用sign接口
   TransactionSignResponse signResponse = sdk.getTransactionService().sign(signRequest);
   if (signResponse.getErrorCode() == 0) {
   TransactionSignResult result = signResponse.getResult();
   System.out.println(JSON.toJSONString(result, true));
   } else {
   System.out.println("error: " + signResponse.getErrorDesc());
   }

提交交易
^^^^^^^^

提交交易接口用于向BU区块链发送交易请求，触发交易的执行。其中 ``transactionBlob`` 和 ``signResult`` 是上面接口得到的，具体接口调用如下所示：

::

   // 初始化请求参数
   TransactionSubmitRequest submitRequest = new TransactionSubmitRequest();
   submitRequest.setTransactionBlob(transactionBlob);
   submitRequest.setSignatures(signResult.getSignatures());

   // 调用submit接口
   TransactionSubmitResponse response = sdk.getTransactionService().submit(submitRequest);
   if (0 == response.getErrorCode()) {
   System.out.println("交易广播成功，hash=" + response.getResult().getHash());
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

账户服务
--------

账户服务提供账户相关的接口，包括6个接口：``checkValid``、``getInfo``、``getNonce``、``getBalance``、``getAssets``、``getMetadata``。

checkValid
~~~~~~~~~~

``checkValid`` 接口用于检查区块链账户地址的有效性。

调用方法如下所示：

AccounCheckValidResponse checkValid(AccountCheckValidRequest);

请求参数如下表所示：

+---------+--------+------------------------------+
| 参数    | 类型   | 描述                         |
+=========+========+==============================+
| address | String | 必填，待检查的区块链账户地址 |
+---------+--------+------------------------------+

响应数据如下表所示：

+---------+--------+----------+
| 参数    | 类型   | 描述     |
+=========+========+==========+
| isValid | String | 是否有效 |
+---------+--------+----------+

错误码如下表所示：

+--------------------+--------+----------------------------------+
| 异常               | 错误码 | 描述                             |
+====================+========+==================================+
| REQUEST_NULL_ERROR | 12001  | Request parameter cannot be null |
+--------------------+--------+----------------------------------+
| SYSTEM_ERROR       | 20000  | System error                     |
+--------------------+--------+----------------------------------+

具体示例如下所示：

::

   // 初始化请求参数
   String address = "buQemmMwmRQY1JkcU7w3nhruoX5N3j6C29uo";
   AccountCheckValidRequest request = new AccountCheckValidRequest();
   request.setAddress(address);

   // 调用checkValid
   AccountCheckValidResponse response = sdk.getAccountService().checkValid(request);
   if(0 == response.getErrorCode()) {
   System.out.println(response.getResult().isValid());
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

getInfo
~~~~~~~

``getInfo`` 接口用于获取指定账户的信息。

调用方法如下所示:

AccountGetInfoResponse GetInfo(AccountGetInfoRequest);

请求参数如下表所示:

+---------+--------+------------------------------+
| 参数    | 类型   | 描述                         |
+=========+========+==============================+
| address | String | 必填，待查询的区块链账户地址 |
+---------+--------+------------------------------+

响应数据如下表所示:

+-----------------------+-----------------------+-----------------------+
| 参数                  | 类型                   | 描述                  |
+=======================+=======================+=======================+
| address               | String                | 账户地址              |
+-----------------------+-----------------------+-----------------------+
| balance               | Long                  | 账户余额，单位MO，1   |
|                       |                       | BU = 10^8 MO,         |
|                       |                       | 必须大于0             |
+-----------------------+-----------------------+-----------------------+
| nonce                 | Long                  | 账户交易序列号，必须大于0 |
+-----------------------+-----------------------+-----------------------+
| priv                  | Priv                  | 账户权限              |
+-----------------------+-----------------------+-----------------------+

错误码如下表所示：

+-----------------------+--------+----------------------------------+
| 异常                  | 错误码 | 描述                             |
+=======================+========+==================================+
| INVALID_ADDRESS_ERROR | 11006  | Invalid address                  |
+-----------------------+--------+----------------------------------+
| REQUEST_NULL_ERROR    | 12001  | Request parameter cannot be null |
+-----------------------+--------+----------------------------------+
| CONNECTNETWORK_ERROR  | 11007  | Fail to connect network          |
+-----------------------+--------+----------------------------------+
| SYSTEM_ERROR          | 20000  | System error                     |
+-----------------------+--------+----------------------------------+

具体示例如下所示：

::

   // 初始化请求参数
   String accountAddress = "buQemmMwmRQY1JkcU7w3nhruoX5N3j6C29uo";
   AccountGetInfoRequest request = new AccountGetInfoRequest();
   request.setAddress(accountAddress);

   // 调用getInfo接口
   AccountGetInfoResponse response = sdk.getAccountService().getInfo(request);
   if (response.getErrorCode() == 0) {
   AccountGetInfoResult result = response.getResult();
   System.out.println("账户信息: \n" + JSON.toJSONString(result, true));
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

Priv
^^^^

Priv的具体信息如下表所示：

+-----------------------+-----------------------+-----------------------+
| 成员                  | 类型                  | 描述                  |
+=======================+=======================+=======================+
| masterWeight          | Long                  | 账户自身权重，大小限制[0, |
|                       |                       |                       |
|                       |                       | (Integer.MAX_VALUE \* |
|                       |                       | 2L + 1)]              |
+-----------------------+-----------------------+-----------------------+
| signers               | `Signer </zhang-hu-fu | 签名者权重列表        |
|                       | -wu/getinfo/signer.md |                       |
|                       | >`__\ []              |                       |
+-----------------------+-----------------------+-----------------------+
| threshold             | `Threshold </zhang-hu | 门限                  |
|                       | -fu-wu/getinfo/threas |                       |
|                       | hold.md>`__           |                       |
+-----------------------+-----------------------+-----------------------+

Signer
^^^^^^

Signer的具体信息如下表所示：

+---------+--------+--------------------------------------------------------+
| 成员    | 类型   | 描述                                                   |
+=========+========+========================================================+
| address | String | 签名者区块链账户地址                                   |
+---------+--------+--------------------------------------------------------+
| weight  | Long   | 签名者权重，大小限制[0, (Integer.MAX_VALUE \* 2L + 1)] |
+---------+--------+--------------------------------------------------------+

Threshold
^^^^^^^^^

Threshold的具体信息如下表所示：

+-----------------------+-----------------------+-----------------------+
| 成员                  | 类型                  | 描述                  |
+=======================+=======================+=======================+
| txThreshold           | Long                  | 交易默认门限，大小限制[0, |
|                       |                       |                       |
|                       |                       | Long.MAX_VALUE]       |
+-----------------------+-----------------------+-----------------------+
| typeThresholds        | `TypeThreshold </zhan | 不同类型交易的门限    |
|                       | g-hu-fu-wu/getinfo/ty |                       |
|                       | pethreashold.md>`__\  |                       |
|                       | []                    |                       |
+-----------------------+-----------------------+-----------------------+

TypeThreshold
^^^^^^^^^^^^^

TypeThreshold的具体信息如下表所示：

+-----------+------+-------------------------------------+
| 成员      | 类型 | 描述                                |
+===========+======+=====================================+
| type      | Long | 操作类型，必须大于0                 |
+-----------+------+-------------------------------------+
| threshold | Long | 门限值，大小限制[0, Long.MAX_VALUE] |
+-----------+------+-------------------------------------+

getNonce
~~~~~~~~

getNonce接口用于获取指定账户的nonce值。

   调用方法如下所示:

AccountGetNonceResponse getNonce(AccountGetNonceRequest);

   请求参数如下表所示:

+---------+--------+------------------------------+
| 参数    | 类型   | 描述                         |
+=========+========+==============================+
| address | String | 必填，待查询的区块链账户地址 |
+---------+--------+------------------------------+

..

   响应数据如下表所示:

+-------+------+----------------+
| 参数  | 类型 | 描述           |
+=======+======+================+
| nonce | Long | 账户交易序列号 |
+-------+------+----------------+

..

   错误码如下表所示：

+-----------------------+--------+----------------------------------+
| 异常                  | 错误码 | 描述                             |
+=======================+========+==================================+
| INVALID_ADDRESS_ERROR | 11006  | Invalid address                  |
+-----------------------+--------+----------------------------------+
| REQUEST_NULL_ERROR    | 12001  | Request parameter cannot be null |
+-----------------------+--------+----------------------------------+
| CONNECTNETWORK_ERROR  | 11007  | Failed to connect to the network |
+-----------------------+--------+----------------------------------+
| SYSTEM_ERROR          | 20000  | System error                     |
+-----------------------+--------+----------------------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   String accountAddress = "buQswSaKDACkrFsnP1wcVsLAUzXQsemauEjf";
   AccountGetNonceRequest request = new AccountGetNonceRequest();
   request.setAddress(accountAddress);

   // 调用getNonce接口
   AccountGetNonceResponse response = sdk.getAccountService().getNonce(request);
   if(0 == response.getErrorCode()){
   System.out.println("账户nonce:" + response.getResult().getNonce());
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

getBalance
~~~~~~~~~~

getBalance接口用于获取指定账户的BU余额。

   调用方法如下所示：

AccountGetBalanceResponse getBalance(AccountGetBalanceRequest);

   请求参数如下表所示：

+---------+--------+------------------------------+
| 参数    | 类型   | 描述                         |
+=========+========+==============================+
| address | String | 必填，待查询的区块链账户地址 |
+---------+--------+------------------------------+

..

   响应数据如下表所示：

+---------+------+----------------------------------+
| 参数    | 类型 | 描述                             |
+=========+======+==================================+
| balance | Long | BU的余额, 单位MO，1 BU = 10^8 MO |
+---------+------+----------------------------------+

..

   错误码如下表所示：

+-----------------------+--------+----------------------------------+
| 异常                  | 错误码 | 描述                             |
+=======================+========+==================================+
| INVALID_ADDRESS_ERROR | 11006  | Invalid address                  |
+-----------------------+--------+----------------------------------+
| REQUEST_NULL_ERROR    | 12001  | Request parameter cannot be null |
+-----------------------+--------+----------------------------------+
| CONNECTNETWORK_ERROR  | 11007  | Failed to connect to the network |
+-----------------------+--------+----------------------------------+
| SYSTEM_ERROR          | 20000  | System error                     |
+-----------------------+--------+----------------------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   String accountAddress = "buQswSaKDACkrFsnP1wcVsLAUzXQsemauEjf";
   AccountGetBalanceRequest request = new AccountGetBalanceRequest();
   request.setAddress(accountAddress);

   // 调用getBalance接口
   AccountGetBalanceResponse response = sdk.getAccountService().getBalance(request);
   if(0 == response.getErrorCode()){
   AccountGetBalanceResult result = response.getResult();
   System.out.println("BU余额：" + ToBaseUnit.MO2BU(result.getBalance().toString()) + " BU");
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

getAssets
~~~~~~~~~

getAssets接口用于获取指定账户的所有资产信息。

   调用方法如下所示：

AccountGetAssets getAssets(AccountGetAssetsRequest);

   请求参数如下表所示：

+---------+--------+------------------------+
| 参数    | 类型   | 描述                   |
+=========+========+========================+
| address | String | 必填，待查询的账户地址 |
+---------+--------+------------------------+

..

   响应数据如下表所示：

+-------+------------------------------------------------------------+----------+
| 参数  | 类型                                                       | 描述     |
+=======+============================================================+==========+
| asset | `AssetInfo </zhang-hu-fu-wu/getassets/assetinfo.md>`__\ [] | 账户资产 |
+-------+------------------------------------------------------------+----------+

..

   错误码如下表所示：

+-----------------------+--------+-------------------------------------+
| 异常                  | 错误码 | 描述                                |
+=======================+========+=====================================+
| INVALID_ADDRESS_ERROR | 11006  | Invalid address                     |
+-----------------------+--------+-------------------------------------+
| REQUEST_NULL_ERROR    | 12001  | Request parameter cannot be null    |
+-----------------------+--------+-------------------------------------+
| CONNECTNETWORK_ERROR  | 11007  | Failed to connect to the network    |
+-----------------------+--------+-------------------------------------+
| NO_ASSET_ERROR        | 11009  | The account does not have the asset |
+-----------------------+--------+-------------------------------------+
| SYSTEM_ERROR          | 20000  | System error                        |
+-----------------------+--------+-------------------------------------+

..

   具体示例如下所示:

::

   // 初始化请求参数
   AccountGetAssetsRequest request = new AccountGetAssetsRequest();
   request.setAddress("buQsurH1M4rjLkfjzkxR9KXJ6jSu2r9xBNEw");

   // 调用getAssets接口
   AccountGetAssetsResponse response = sdk.getAccountService().getAssets(request);
   if (response.getErrorCode() == 0) {
   AccountGetAssetsResult result = response.getResult();
   System.out.println(JSON.toJSONString(result, true));
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

AssetInfo
^^^^^^^^^

AssetInfo的具体信息如下表所示：

+-------------+--------------------------------------------+--------------+
| 成员        | 类型                                       | 描述         |
+=============+============================================+==============+
| key         | `Key </zhang-hu-fu-wu/getassets/key.md>`__ | 资产惟一标识 |
+-------------+--------------------------------------------+--------------+
| assetAmount | Long                                       | 资产数量     |
+-------------+--------------------------------------------+--------------+

Key
^^^

Key的具体信息如下表所示：

+--------+--------+------------------+
| 成员   | 类型   | 描述             |
+========+========+==================+
| code   | String | 资产编码         |
+--------+--------+------------------+
| issuer | String | 资产发行账户地址 |
+--------+--------+------------------+

getMetadata
~~~~~~~~~~~

getMetadata接口用于获取指定账户的metadata信息。

   调用方法如下所示：

AccountGetMetadataResponse getMetadata(AccountGetMetadataRequest);

   请求参数如下表所示：

+---------+--------+-----------------------------------------+
| 参数    | 类型   | 描述                                    |
+=========+========+=========================================+
| address | String | 必填，待查询的账户地址                  |
+---------+--------+-----------------------------------------+
| key     | String | 选填，metadata关键字，长度限制[1, 1024] |
+---------+--------+-----------------------------------------+

..

   响应数据如下表所示：

+-----------------------+-----------------------+-----------------------+
| 参数                  | 类型                  | 描述                  |
+=======================+=======================+=======================+
| metadata              | `MetadataInfo </zhang | 账户                  |
|                       | -hu-fu-wu/getmetadata |                       |
|                       | /metadatainfo.md>`__  |                       |
+-----------------------+-----------------------+-----------------------+

..

   错误码如下表所示:

+-----------------------+--------+----------------------------------------------+
| 异常                  | 错误码 | 描述                                         |
+=======================+========+==============================================+
| INVALID_ADDRESS_ERROR | 11006  | Invalid address                              |
+-----------------------+--------+----------------------------------------------+
| REQUEST_NULL_ERROR    | 12001  | Request parameter cannot be null             |
+-----------------------+--------+----------------------------------------------+
| CONNECTNETWORK_ERROR  | 11007  | Failed to connect to the network             |
+-----------------------+--------+----------------------------------------------+
| NO_METADATA_ERROR     | 11010  | The account does not have the metadata       |
+-----------------------+--------+----------------------------------------------+
| INVALID_DATAKEY_ERROR | 11011  | The length of key must be between 1 and 1024 |
+-----------------------+--------+----------------------------------------------+
| SYSTEM_ERROR          | 20000  | System error                                 |
+-----------------------+--------+----------------------------------------------+

..

   具体示例如下所示:

::

   // 初始化请求参数
   String accountAddress = "buQsurH1M4rjLkfjzkxR9KXJ6jSu2r9xBNEw";
   AccountGetMetadataRequest request = new AccountGetMetadataRequest();
   request.setAddress(accountAddress);
   request.setKey("20180704");

   // 调用getMetadata接口
   AccountGetMetadataResponse response = sdk.getAccountService().getMetadata(request);
   if (response.getErrorCode() == 0) {
   AccountGetMetadataResult result = response.getResult();
   System.out.println(JSON.toJSONString(result, true));
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

MetadataInfo
^^^^^^^^^^^^

MetadataInfo的具体信息如下表所示：

+---------+--------+------------------+
| 成员    | 类型   | 描述             |
+=========+========+==================+
| key     | String | metadata的关键词 |
+---------+--------+------------------+
| value   | String | metadata的内容   |
+---------+--------+------------------+
| version | Long   | metadata的版本   |
+---------+--------+------------------+

资产服务
--------

资产服务遵循ATP1.0协议，账户服务提供资产相关的接口，目前有1个接口：getInfo。

.. _getinfo-1:

getInfo
~~~~~~~

getInfo接口用于获取指定账户的指定资产信息。

   调用方法如下所示：

AssetGetInfoResponse getInfo(AssetGetInfoRequest);

   请求参数如下表所示：

+---------+--------+---------------------------------+
| 参数    | 类型   | 描述                            |
+=========+========+=================================+
| address | String | 必填，待查询的账户地址          |
+---------+--------+---------------------------------+
| code    | String | 必填，资产编码，长度限制[1, 64] |
+---------+--------+---------------------------------+
| issuer  | String | 必填，资产发行账户地址          |
+---------+--------+---------------------------------+

..

   响应数据如下表所示：

+-------+------------------------------------------------------------+----------+
| 参数  | 类型                                                       | 描述     |
+=======+============================================================+==========+
| asset | `AssetInfo </zhang-hu-fu-wu/getassets/assetinfo.md>`__\ [] | 账户资产 |
+-------+------------------------------------------------------------+----------+

..

   错误码如下表所示：

+-------------------------+-------------------------+------------------+
| 异常                    | 错误码                  | 描述             |
+=========================+=========================+==================+
| INVALID_ADDRESS_ERROR   | 11006                   | Invalid address  |
+-------------------------+-------------------------+------------------+
| REQUEST_NULL_ERROR      | 12001                   | Request          |
|                         |                         | parameter cannot |
|                         |                         | be null          |
+-------------------------+-------------------------+------------------+
| CONNECTNETWORK_ERROR    | 11007                   | Failed to        |
|                         |                         | connect to the   |
|                         |                         | network          |
+-------------------------+-------------------------+------------------+
| INVALID_ASSET_CODE_ERRO | 11023                   | The length of    |
| R                       |                         | asset code must  |
|                         |                         | be between 1 and |
|                         |                         | 64               |
+-------------------------+-------------------------+------------------+
| INVALID_ISSUER_ADDRESS_ | 11027                   | Invalid issuer   |
| ERROR                   |                         | address          |
+-------------------------+-------------------------+------------------+
| SYSTEM_ERROR            | 20000                   | System error     |
+-------------------------+-------------------------+------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   AssetGetInfoRequest request = new AssetGetInfoRequest();
   request.setAddress("buQsurH1M4rjLkfjzkxR9KXJ6jSu2r9xBNEw");
   request.setIssuer("buQBjJD1BSJ7nzAbzdTenAhpFjmxRVEEtmxH");
   request.setCode("HNC");

   // 调用getInfo消息
   AssetGetInfoResponse response = sdk.getAssetService().getInfo(request);
   if (response.getErrorCode() == 0) {
   AssetGetInfoResult result = response.getResult();
   System.out.println(JSON.toJSONString(result, true));
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

Ctp10Token服务
~~~~~~~~~~~~~~

Ctp10Token服务遵循CTP1.0协议，主要提供合约Token相关的接口，目前有8个接口：checkValid、allowance、getInfo、getName、
getSymbol、getDecimals、getTotalSupply、getBalance。

.. _checkvalid-1:

checkValid
~~~~~~~~~~

checkValid接口用于验证合约Token的有效性。

   调用方法如下所示：

Ctp10TokenCheckValidResponse checkValid(Ctp10TokenCheckValidRequest);

   请求参数如下表所示：

+-----------------+--------+-----------------------------+
| 参数            | 类型   | 描述                        |
+=================+========+=============================+
| contractAddress | String | 必填，待验证的Token合约地址 |
+-----------------+--------+-----------------------------+

..

   响应数据如下表所示：

+---------+--------+----------+
| 参数    | 类型   | 描述     |
+=========+========+==========+
| isValid | String | 是否有效 |
+---------+--------+----------+

..

   错误码如下表所示：

+-------------------------------+--------+----------------------------------+
| 异常                          | 错误码 | 描述                             |
+===============================+========+==================================+
| INVALID_CONTRACTADDRESS_ERROR | 11037  | Invalid contract address         |
+-------------------------------+--------+----------------------------------+
| REQUEST_NULL_ERROR            | 12001  | Request parameter cannot be null |
+-------------------------------+--------+----------------------------------+
| SYSTEM_ERROR                  | 20000  | System error                     |
+-------------------------------+--------+----------------------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   Ctp10TokenCheckValidRequest request = new Ctp10TokenCheckValidRequest();
   request.setContractAddress("buQfnVYgXuMo3rvCEpKA6SfRrDpaz8D8A9Ea");

   // 调用checkValid接口
   Ctp10TokenCheckValidResponse response = sdk.getTokenService().checkValid(request);
   if (response.getErrorCode() == 0) {
   Ctp10TokenCheckValidResult result = response.getResult();
   System.out.println(result.getValid());
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

allowance
~~~~~~~~~

allowance接口用于获取spender允许从owner提取的金额。

   调用方法如下所示：

Ctp10TokenAllowanceResponse allowance(Ctp10TokenAllowanceRequest);

   请求参数如下表所示：

+-----------------+--------+---------------------------------+
| 参数            | 类型   | 描述                            |
+=================+========+=================================+
| contractAddress | String | 必填，合约账户地址              |
+-----------------+--------+---------------------------------+
| tokenOwner      | String | 必填，合约Token的持有者账户地址 |
+-----------------+--------+---------------------------------+
| spender         | String | 必填，被授权账户地址            |
+-----------------+--------+---------------------------------+

..

   响应数据如下表所示：

+-----------+--------+----------------+
| 参数      | 类型   | 描述           |
+===========+========+================+
| allowance | String | 允许提取的金额 |
+-----------+--------+----------------+

..

   错误码如下表所示：

+-------------------------------+--------+----------------------------------+
| 异常                          | 错误码 | 描述                             |
+===============================+========+==================================+
| INVALID_CONTRACTADDRESS_ERROR | 11037  | Invalid contract address         |
+-------------------------------+--------+----------------------------------+
| NO_SUCH_TOKEN_ERROR           | 11030  | No such token                    |
+-------------------------------+--------+----------------------------------+
| INVALID_TOKENOWNER_ERRPR      | 11035  | Invalid token owner              |
+-------------------------------+--------+----------------------------------+
| INVALID_SPENDER_ERROR         | 11043  | Invalid spender                  |
+-------------------------------+--------+----------------------------------+
| GET_ALLOWNANCE_ERROR          | 11036  | Fail to get allowance            |
+-------------------------------+--------+----------------------------------+
| REQUEST_NULL_ERROR            | 12001  | Request parameter cannot be null |
+-------------------------------+--------+----------------------------------+
| SYSTEM_ERROR                  | 20000  | System error                     |
+-------------------------------+--------+----------------------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   Ctp10TokenAllowanceRequest request = new Ctp10TokenAllowanceRequest();
   request.setContractAddress("buQhdBSkJqERBSsYiUShUZFMZQhXvkdNgnYq");
   request.setTokenOwner("buQnnUEBREw2hB6pWHGPzwanX7d28xk6KVcp");
   request.setSpender("buQnnUEBREw2hB6pWHGPzwanX7d28xk6KVcp");

   // 调用allowance接口
   Ctp10TokenAllowanceResponse response = sdk.getTokenService().allowance(request);
   if (response.getErrorCode() == 0) {
   Ctp10TokenAllowanceResult result = response.getResult();
   System.out.println(JSON.toJSONString(result, true));
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

getInfo-Ctp10Token
~~~~~~~~~~~~~~~~~~

getInfo-Ctp10Token接口用于获取合约Token的信息。

   调用方法如下所示：

Ctp10TokenGetInfoResponse getInfo(Ctp10TokenGetInfoRequest);

   请求参数如下表所示：

+-----------------+--------+-----------------------+
| 参数            | 类型   | 描述                  |
+=================+========+=======================+
| contractAddress | String | 待查询的合约Token地址 |
+-----------------+--------+-----------------------+

..

   响应数据如下表所示：

+---------------+---------+-------------------+
| 参数          | 类型    | 描述              |
+===============+=========+===================+
| ctp           | String  | 合约Token版本号   |
+---------------+---------+-------------------+
| symbol        | String  | 合约Token符号     |
+---------------+---------+-------------------+
| decimals      | Integer | 合约数量的精度    |
+---------------+---------+-------------------+
| totalSupply   | String  | 合约的总供应量    |
+---------------+---------+-------------------+
| name          | String  | 合约Token的名称   |
+---------------+---------+-------------------+
| contractOwner | String  | 合约Token的拥有者 |
+---------------+---------+-------------------+

..

   具体错误码如下表所示：

+-------------------------------+--------+----------------------------------+
| 异常                          | 错误码 | 描述                             |
+===============================+========+==================================+
| INVALID_CONTRACTADDRESS_ERROR | 11037  | Invalid contract address         |
+-------------------------------+--------+----------------------------------+
| NO_SUCH_TOKEN_ERROR           | 11030  | No such token                    |
+-------------------------------+--------+----------------------------------+
| GET_TOKEN_INFO_ERROR          | 11066  | Failed to get token info         |
+-------------------------------+--------+----------------------------------+
| REQUEST_NULL_ERROR            | 12001  | Request parameter cannot be null |
+-------------------------------+--------+----------------------------------+
| SYSTEM_ERROR                  | 20000  | System error                     |
+-------------------------------+--------+----------------------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   Ctp10TokenGetInfoRequest request = new Ctp10TokenGetInfoRequest();
   request.setContractAddress("buQhdBSkJqERBSsYiUShUZFMZQhXvkdNgnYq");

   // 调用getInfo接口
   Ctp10TokenGetInfoResponse response = sdk.getTokenService().getInfo(request);
   if (response.getErrorCode() == 0) {
   Ctp10TokenGetInfoResult result = response.getResult();
   System.out.println(JSON.toJSONString(result, true));
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

getName
~~~~~~~

getName接口用于获取合约Token的名称。

   调用方法如下所示：

Ctp10TokenGetNameResponse getName(Ctp10TokenGetNameRequest);

   请求参数如下表所示：

+-----------------+--------+----------------------+
| 参数            | 类型   | 描述                 |
+=================+========+======================+
| contractAddress | String | 待查询的合约账户地址 |
+-----------------+--------+----------------------+

..

   响应数据如下表所示：

+------+--------+-----------------+
| 参数 | 类型   | 描述            |
+======+========+=================+
| name | String | 合约Token的名称 |
+------+--------+-----------------+

..

   错误码如下表所示：

+-------------------------------+--------+----------------------------------+
| 异常                          | 错误码 | 描述                             |
+===============================+========+==================================+
| INVALID_CONTRACTADDRESS_ERROR | 11037  | Invalid contract address         |
+-------------------------------+--------+----------------------------------+
| NO_SUCH_TOKEN_ERROR           | 11030  | No such token                    |
+-------------------------------+--------+----------------------------------+
| GET_TOKEN_INFO_ERROR          | 11066  | Failed to get token info         |
+-------------------------------+--------+----------------------------------+
| REQUEST_NULL_ERROR            | 12001  | Request parameter cannot be null |
+-------------------------------+--------+----------------------------------+
| SYSTEM_ERROR                  | 20000  | System error                     |
+-------------------------------+--------+----------------------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   Ctp10TokenGetNameRequest request = new Ctp10TokenGetNameRequest();
   request.setContractAddress("buQhdBSkJqERBSsYiUShUZFMZQhXvkdNgnYq");

   // 调用getName接口
   Ctp10TokenGetNameResponse response = sdk.getTokenService().getName(request);
   if (response.getErrorCode() == 0) {
   Ctp10TokenGetNameResult result = response.getResult();
   System.out.println(result.getName());
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

getSymbol
~~~~~~~~~

getSymbol接口用于获取合约Token的符号。

   调用方法如下所示：

Ctp10TokenGetSymbolResponse getSymbol (Ctp10TokenGetSymbolRequest);

   请求参数如下表所示：

+-----------------+--------+----------------------+
| 参数            | 类型   | 描述                 |
+=================+========+======================+
| contractAddress | String | 待查询的合约账户地址 |
+-----------------+--------+----------------------+

..

   响应数据如下表所示：

+--------+--------+-----------------+
| 参数   | 类型   | 描述            |
+========+========+=================+
| symbol | String | 合约Token的符号 |
+--------+--------+-----------------+

..

   错误码如下表所示：

+-------------------------------+--------+----------------------------------+
| 异常                          | 错误码 | 描述                             |
+===============================+========+==================================+
| INVALID_CONTRACTADDRESS_ERROR | 11037  | Invalid contract address         |
+-------------------------------+--------+----------------------------------+
| NO_SUCH_TOKEN_ERROR           | 11030  | No such token                    |
+-------------------------------+--------+----------------------------------+
| GET_TOKEN_INFO_ERROR          | 11066  | Failed to get token info         |
+-------------------------------+--------+----------------------------------+
| REQUEST_NULL_ERROR            | 12001  | Request parameter cannot be null |
+-------------------------------+--------+----------------------------------+
| SYSTEM_ERROR                  | 20000  | System error                     |
+-------------------------------+--------+----------------------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   Ctp10TokenGetSymbolRequest request = new Ctp10TokenGetSymbolRequest();
   request.setContractAddress("buQhdBSkJqERBSsYiUShUZFMZQhXvkdNgnYq");

   // 调用getSymbol接口
   Ctp10TokenGetSymbolResponse response = sdk.getTokenService().getSymbol(request);
   if (response.getErrorCode() == 0) {
   Ctp10TokenGetSymbolResult result = response.getResult();
   System.out.println(result.getSymbol());
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

getDecimals
~~~~~~~~~~~

getDecimals接口用于获取合约Token的精度。

   调用方法如下所示：

Ctp10TokenGetDecimalsResponse getDecimals
(Ctp10TokenGetDecimalsRequest);

   请求参数如下表所示：

+-----------------+--------+----------------------+
| 参数            | 类型   | 描述                 |
+=================+========+======================+
| contractAddress | String | 待查询的合约账户地址 |
+-----------------+--------+----------------------+

..

   响应数据如下表所示：

+----------+---------+---------------+
| 参数     | 类型    | 描述          |
+==========+=========+===============+
| decimals | Integer | 合约token精度 |
+----------+---------+---------------+

..

   错误码如下表所示：

+-------------------------------+--------+----------------------------------+
| 异常                          | 错误码 | 描述                             |
+===============================+========+==================================+
| INVALID_CONTRACTADDRESS_ERROR | 11037  | Invalid contract address         |
+-------------------------------+--------+----------------------------------+
| NO_SUCH_TOKEN_ERROR           | 11030  | No such token                    |
+-------------------------------+--------+----------------------------------+
| GET_TOKEN_INFO_ERROR          | 11066  | Failed to get token info         |
+-------------------------------+--------+----------------------------------+
| REQUEST_NULL_ERROR            | 12001  | Request parameter cannot be null |
+-------------------------------+--------+----------------------------------+
| SYSTEM_ERROR                  | 20000  | System error                     |
+-------------------------------+--------+----------------------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   Ctp10TokenGetDecimalsRequest request = new Ctp10TokenGetDecimalsRequest();
   request.setContractAddress("buQhdBSkJqERBSsYiUShUZFMZQhXvkdNgnYq");

   // 调用getDecimals接口
   Ctp10TokenGetDecimalsResponse response = sdk.getTokenService().getDecimals(request);
   if (response.getErrorCode() == 0) {
   Ctp10TokenGetDecimalsResult result = response.getResult();
   System.out.println(result.getDecimals());
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

getTotalSupply
~~~~~~~~~~~~~~

getTotalSupply接口用于获取合约Token的总供应量。

   调用方法如下所示：

Ctp10TokenGetTotalSupplyResponse
getTotalSupply(Ctp10TokenGetTotalSupplyRequest);

   请求参数如下表所示：

+-----------------+--------+----------------------+
| 参数            | 类型   | 描述                 |
+=================+========+======================+
| contractAddress | String | 待查询的合约账户地址 |
+-----------------+--------+----------------------+

..

   响应数据如下表所示：

+-------------+--------+---------------------+
| 参数        | 类型   | 描述                |
+=============+========+=====================+
| totalSupply | String | 合约Token的总供应量 |
+-------------+--------+---------------------+

..

   错误码如下表所示：

+-------------------------------+--------+----------------------------------+
| 异常                          | 错误码 | 描述                             |
+===============================+========+==================================+
| INVALID_CONTRACTADDRESS_ERROR | 11037  | Invalid contract address         |
+-------------------------------+--------+----------------------------------+
| NO_SUCH_TOKEN_ERROR           | 11030  | No such token                    |
+-------------------------------+--------+----------------------------------+
| GET_TOKEN_INFO_ERROR          | 11066  | Failed to get token info         |
+-------------------------------+--------+----------------------------------+
| REQUEST_NULL_ERROR            | 12001  | Request parameter cannot be null |
+-------------------------------+--------+----------------------------------+
| SYSTEM_ERROR                  | 20000  | System error                     |
+-------------------------------+--------+----------------------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   Ctp10TokenGetTotalSupplyRequest request = new Ctp10TokenGetTotalSupplyRequest();
   request.setContractAddress("buQhdBSkJqERBSsYiUShUZFMZQhXvkdNgnYq");

   // 调用getTotalSupply接口
   Ctp10TokenGetTotalSupplyResponse response = sdk.getTokenService().getTotalSupply(request);
   if (response.getErrorCode() == 0) {
   Ctp10TokenGetTotalSupplyResult result = response.getResult();
   System.out.println(result.getTotalSupply());
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

getBalance-Ctp10Token
~~~~~~~~~~~~~~~~~~~~~

getBalance-Ctp10Token接口获取合约Token拥有者的账户余额。

   调用方法如下所示：

Ctp10TokenGetBalanceResponse getBalance(Ctp10TokenGetBalanceRequest)

   请求参数如下表所示：

+-----------------+--------+---------------------------------+
| 参数            | 类型   | 描述                            |
+=================+========+=================================+
| contractAddress | String | 待查询的合约账户地址            |
+-----------------+--------+---------------------------------+
| tokenOwner      | String | 必填，合约Token持有者的账户地址 |
+-----------------+--------+---------------------------------+

..

   响应数据如下表所示：

+---------+------+-------------+
| 参数    | 类型 | 描述        |
+=========+======+=============+
| balance | Long | token的余额 |
+---------+------+-------------+

..

   错误码如下表所示：

+-------------------------------+--------+----------------------------------+
| 异常                          | 错误码 | 描述                             |
+===============================+========+==================================+
| INVALID_TOKENOWNER_ERRPR      | 11035  | Invalid token owner              |
+-------------------------------+--------+----------------------------------+
| INVALID_CONTRACTADDRESS_ERROR | 11037  | Invalid contract address         |
+-------------------------------+--------+----------------------------------+
| NO_SUCH_TOKEN_ERROR           | 11030  | No such token                    |
+-------------------------------+--------+----------------------------------+
| GET_TOKEN_INFO_ERROR          | 11066  | Failed to get token info         |
+-------------------------------+--------+----------------------------------+
| REQUEST_NULL_ERROR            | 12001  | Request parameter cannot be null |
+-------------------------------+--------+----------------------------------+
| SYSTEM_ERROR                  | 20000  | System error                     |
+-------------------------------+--------+----------------------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   Ctp10TokenGetBalanceRequest request = new Ctp10TokenGetBalanceRequest();
   request.setContractAddress("buQhdBSkJqERBSsYiUShUZFMZQhXvkdNgnYq");
   request.setTokenOwner("buQnnUEBREw2hB6pWHGPzwanX7d28xk6KVcp");

   // 调用getBalance接口
   Ctp10TokenGetBalanceResponse response = sdk.getTokenService().getBalance(request);
   if (response.getErrorCode() == 0) {
   Ctp10TokenGetBalanceResult result = response.getResult();
   System.out.println(result.getBalance());
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

合约服务
--------

合约服务提供合约相关的接口，目前有4个接口：checkValid、getInfo、getAddress、call。

.. _checkvalid-2:

checkValid
~~~~~~~~~~

checkValid接口用于检测合约账户的有效性。

   调用方法如下所示：

ContractCheckValidResponse checkValid(ContractCheckValidRequest);

   请求参数如下表所示：

+-----------------+--------+----------------------+
| 参数            | 类型   | 描述                 |
+=================+========+======================+
| contractAddress | String | 待检测的合约账户地址 |
+-----------------+--------+----------------------+

..

   响应数据如下表所示：

+---------+---------+----------+
| 参数    | 类型    | 描述     |
+=========+=========+==========+
| isValid | Boolean | 是否有效 |
+---------+---------+----------+

..

   错误码如下表所示：

+-------------------------------+--------+----------------------------------+
| 异常                          | 错误码 | 描述                             |
+===============================+========+==================================+
| INVALID_CONTRACTADDRESS_ERROR | 11037  | Invalid contract address         |
+-------------------------------+--------+----------------------------------+
| REQUEST_NULL_ERROR            | 12001  | Request parameter cannot be null |
+-------------------------------+--------+----------------------------------+
| SYSTEM_ERROR                  | 20000  | System error                     |
+-------------------------------+--------+----------------------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   ContractCheckValidRequest request = new ContractCheckValidRequest();
   request.setContractAddress("buQfnVYgXuMo3rvCEpKA6SfRrDpaz8D8A9Ea");

   // 调用checkValid接口
   ContractCheckValidResponse response = sdk.getContractService().checkValid(request);
   if (response.getErrorCode() == 0) {
   ContractCheckValidResult result = response.getResult();
   System.out.println(result.getValid());
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

.. _getinfo-2:

getInfo
~~~~~~~

getInfo接口用于查询合约代码。

   调用方法如下所示:

ContractGetInfoResponse getInfo (ContractGetInfoRequest);

   请求参数如下表所示：

+-----------------+--------+----------------------+
| 参数            | 类型   | 描述                 |
+=================+========+======================+
| contractAddress | String | 待查询的合约账户地址 |
+-----------------+--------+----------------------+

..

   响应数据如下表所示：

+----------+----------------------------------------------------------+----------+
| 参数     | 类型                                                     | 描述     |
+==========+==========================================================+==========+
| contract | `ContractInfo </he-yue-fu-wu/getinfo/contractinfo.md>`__ | 合约信息 |
+----------+----------------------------------------------------------+----------+

..

   错误码如下表所示：

+-------------------------+-------------------------+------------------+
| 异常                    | 错误码                  | 描述             |
+=========================+=========================+==================+
| INVALID_CONTRACTADDRESS | 11037                   | Invalid contract |
| _ERROR                  |                         | address          |
+-------------------------+-------------------------+------------------+
| CONTRACTADDRESS_NOT_CON | 11038                   | contractAddress  |
| TRACTACCOUNT_ERROR      |                         | is not a         |
|                         |                         | contract account |
+-------------------------+-------------------------+------------------+
| NO_SUCH_TOKEN_ERROR     | 11030                   | No such token    |
+-------------------------+-------------------------+------------------+
| GET_TOKEN_INFO_ERROR    | 11066                   | Failed to get    |
|                         |                         | token info       |
+-------------------------+-------------------------+------------------+
| REQUEST_NULL_ERROR      | 12001                   | Request          |
|                         |                         | parameter cannot |
|                         |                         | be null          |
+-------------------------+-------------------------+------------------+
| SYSTEM_ERROR            | 20000                   | System error     |
+-------------------------+-------------------------+------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   ContractGetInfoRequest request = new ContractGetInfoRequest();
   request.setContractAddress("buQfnVYgXuMo3rvCEpKA6SfRrDpaz8D8A9Ea");

   // 调用getInfo接口
   ContractGetInfoResponse response = sdk.getContractService().getInfo(request);
   if (response.getErrorCode() == 0) {
   System.out.println(JSON.toJSONString(response.getResult(), true));
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

ContractInfo
^^^^^^^^^^^^

ContractInfo的具体信息如下表所示：

+---------+---------+-------------------+
| 成员    | 类型    | 描述              |
+=========+=========+===================+
| type    | Integer | 合约类型，默认为0 |
+---------+---------+-------------------+
| payload | String  | 合约代码          |
+---------+---------+-------------------+

getAddress
~~~~~~~~~~

getAddress接口用于查询合约地址。

   调用方法如下所示：

ContractGetAddressResponse getInfo (ContractGetAddressRequest);

   请求参数如下表所示：

+------+--------+--------------------+
| 参数 | 类型   | 描述               |
+======+========+====================+
| hash | String | 创建合约交易的hash |
+------+--------+--------------------+

..

   响应数据如下表所示：

+-----------------------+-----------------------+-----------------------+
| 参数                  | 类型                  | 描述                  |
+=======================+=======================+=======================+
| contractAddressList   | List                  | 合约地址列表          |
|                       | (`ContractAddressInfo |                       |
|                       |  </he-yue-fu-wu/getad |                       |
|                       | dress/contractaddress |                       |
|                       | info.md>`__)          |                       |
+-----------------------+-----------------------+-----------------------+

..

   错误码如下表所示：

+----------------------+--------+----------------------------------+
| 异常                 | 错误码 | 描述                             |
+======================+========+==================================+
| INVALID_HASH_ERROR   | 11055  | Invalid transaction hash         |
+----------------------+--------+----------------------------------+
| CONNECTNETWORK_ERROR | 11007  | Failed to connect to the network |
+----------------------+--------+----------------------------------+
| REQUEST_NULL_ERROR   | 12001  | Request parameter cannot be null |
+----------------------+--------+----------------------------------+
| SYSTEM_ERROR         | 20000  | System error                     |
+----------------------+--------+----------------------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   ContractGetAddressRequest request = new ContractGetAddressRequest();
   request.setHash("44246c5ba1b8b835a5cbc29bdc9454cdb9a9d049870e41227f2dcfbcf7a07689");

   // 调用getAddress接口
   ContractGetAddressResponse response = sdk.getContractService().getAddress(request);
   if (response.getErrorCode() == 0) {
   System.out.println(JSON.toJSONString(response.getResult(), true));
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

ContractAddressInfo
~~~~~~~~~~~~~~~~~~~

ContractAddressInfo的具体信息如下表所示：

+-----------------+---------+----------------+
| 成员            | 类型    | 描述           |
+=================+=========+================+
| contractAddress | String  | 合约地址       |
+-----------------+---------+----------------+
| operationIndex  | Integer | 所在操作的下标 |
+-----------------+---------+----------------+

call
~~~~

call接口用于调试合约代码。

   调用方法如下所示：

ContractCallesponse call(ContractCallRequest);

   请求参数如下表所示：

+-------------------+---------------------+----------------------------+
| 参数              | 类型                | 描述                       |
+===================+=====================+============================+
| sourceAddress     | String              | 选填，合约触发账户地址     |
+-------------------+---------------------+----------------------------+
| contractAddress   | String              | 选填，合约账户地址，与code不能同时为空 |
+-------------------+---------------------+----------------------------+
| code              | String              | 选填，合约代码，与contractAddress不能 |
|                   |                     | 同时为空，长度限制[1,      |
|                   |                     | 64]                        |
+-------------------+---------------------+----------------------------+
| input             | String              | 选填，合约入参             |
+-------------------+---------------------+----------------------------+
| contractBalance   | String              | 选填，赋予合约的初始 BU    |
|                   |                     | 余额, 单位MO，1 BU = 10^8  |
|                   |                     | MO, 大小限制[1,            |
|                   |                     | Long.MAX_VALUE]            |
+-------------------+---------------------+----------------------------+
| optType           | Integer             | 必填，0:                   |
|                   |                     | 调用合约的读写接口 init,   |
|                   |                     | 1: 调用合约的读写接口      |
|                   |                     | main, 2 :调用只读接口      |
|                   |                     | query                      |
+-------------------+---------------------+----------------------------+
| feeLimit          | Long                | 交易要求的最低手续费，     |
|                   |                     | 大小限制[1,                |
|                   |                     | Long.MAX_VALUE]            |
+-------------------+---------------------+----------------------------+
| gasPrice          | Long                | 交易燃料单价，大小限制[1000, |
|                   |                     |                            |
|                   |                     | Long.MAX_VALUE]            |
+-------------------+---------------------+----------------------------+

..

   响应数据如下表所示：

+-----------------------+-----------------------+-----------------------+
| 参数                  | 类型                  | 描述                  |
+=======================+=======================+=======================+
| logs                  | JSONObject            | 日志信息              |
+-----------------------+-----------------------+-----------------------+
| queryRets             | JSONArray             | 查询结果集            |
+-----------------------+-----------------------+-----------------------+
| stat                  | `ContractStat </he-yu | 合约资源占用信息      |
|                       | e-fu-wu/call/contract |                       |
|                       | stat.md>`__           |                       |
+-----------------------+-----------------------+-----------------------+
| txs                   | `TransactionEnvs </he | 交易集                |
|                       | -yue-fu-wu/call/trans |                       |
|                       | actionenvs.md>`__\ [] |                       |
+-----------------------+-----------------------+-----------------------+

..

   错误码如下表所示：

+-------------------------+-------------------------+------------------+
| 异常                    | 错误码                  | 描述             |
+=========================+=========================+==================+
| INVALID_SOURCEADDRESS_E | 11002                   | Invalid          |
| RROR                    |                         | sourceAddress    |
+-------------------------+-------------------------+------------------+
| INVALID_CONTRACTADDRESS | 11037                   | Invalid contract |
| _ERROR                  |                         | address          |
+-------------------------+-------------------------+------------------+
| CONTRACTADDRESS_CODE_BO | 11063                   | ContractAddress  |
| TH_NULL_ERROR           |                         | and code cannot  |
|                         |                         | be empty at the  |
|                         |                         | same time        |
+-------------------------+-------------------------+------------------+
| INVALID_OPTTYPE_ERROR   | 11064                   | OptType must be  |
|                         |                         | between 0 and 2  |
+-------------------------+-------------------------+------------------+
| REQUEST_NULL_ERROR      | 12001                   | Request          |
|                         |                         | parameter cannot |
|                         |                         | be null          |
+-------------------------+-------------------------+------------------+
| CONNECTNETWORK_ERROR    | 11007                   | Failed to        |
|                         |                         | connect to the   |
|                         |                         | network          |
+-------------------------+-------------------------+------------------+
| SYSTEM_ERROR            | 20000                   | System error     |
+-------------------------+-------------------------+------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   ContractCallRequest request = new ContractCallRequest();
   request.setCode("\"use strict\";log(undefined);function query() { getBalance(thisAddress); }");
   request.setFeeLimit(1000000000L);
   request.setOptType(2);

   // 调用call接口
   ContractCallResponse response = sdk.getContractService().call(request);
   if (response.getErrorCode() == 0) {
   ContractCallResult result = response.getResult();
   System.out.println(JSON.toJSONString(result, true));
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

ContractStat
^^^^^^^^^^^^

ContractStat的具体信息如下表所示：

+-------------+------+----------------+
| 成员        | 类型 | 描述           |
+=============+======+================+
| applyTime   | Long | 接收时间       |
+-------------+------+----------------+
| memoryUsage | Long | 内存占用量     |
+-------------+------+----------------+
| stackUsage  | Long | 堆栈占用量     |
+-------------+------+----------------+
| step        | Long | 完成需要的步数 |
+-------------+------+----------------+

TransactionEnvs
^^^^^^^^^^^^^^^

TransactionEnvs的具体信息如下表所示：

+-----------------------+-----------------------+-----------------------+
| 成员                  | 类型                  | 描述                  |
+=======================+=======================+=======================+
| transactionEnv        | `TransactionEnv </he- | 交易                  |
|                       | yue-fu-wu/call/transa |                       |
|                       | ctionenv.md>`__       |                       |
+-----------------------+-----------------------+-----------------------+

TransactionEnv
^^^^^^^^^^^^^^

TransactionEnv的具体信息如下表所示：

+-----------------------+-----------------------+-----------------------+
| 成员                  | 类型                  | 描述                  |
+=======================+=======================+=======================+
| transaction           | `TransactionInfo </he | 交易内容              |
|                       | -yue-fu-wu/call/trans |                       |
|                       | actioninfo.md>`__     |                       |
+-----------------------+-----------------------+-----------------------+
| trigger               | `ContractTrigger </he | 合约触发者            |
|                       | -yue-fu-wu/call/contr |                       |
|                       | acttrigger.md>`__     |                       |
+-----------------------+-----------------------+-----------------------+

TransactionInfo
^^^^^^^^^^^^^^^

TransactionInfo的具体信息如下表所示：

+-----------------------+-----------------------+-----------------------+
| 成员                  | 类型                  | 描述                  |
+=======================+=======================+=======================+
| sourceAddress         | String                | 交易发起的源账户地址  |
+-----------------------+-----------------------+-----------------------+
| feeLimit              | Long                  | 交易要求的最低费用    |
+-----------------------+-----------------------+-----------------------+
| gasPrice              | Long                  | 交易燃料单价          |
+-----------------------+-----------------------+-----------------------+
| nonce                 | Long                  | 交易序列号            |
+-----------------------+-----------------------+-----------------------+
| operations            | `Operation </he-yue-f | 操作列表              |
|                       | u-wu/call/operation.m |                       |
|                       | d>`__\ []             |                       |
+-----------------------+-----------------------+-----------------------+

ContractTrigger
^^^^^^^^^^^^^^^

ContractTrigger的具体信息如下表所示：

+-----------------------+-----------------------+-----------------------+
| 成员                  | 类型                  | 描述                  |
+=======================+=======================+=======================+
| transaction           | `TriggerTransaction < | 触发交易              |
|                       | /he-yue-fu-wu/call/tr |                       |
|                       | iggertransaction.md>` |                       |
|                       | __                    |                       |
+-----------------------+-----------------------+-----------------------+

Operation
^^^^^^^^^

Operation的具体信息如下表所示：

+-----------------------+-----------------------+-----------------------+
| 成员                  | 类型                  | 描述                  |
+=======================+=======================+=======================+
| type                  | Integer               | 操作类型              |
+-----------------------+-----------------------+-----------------------+
| sourceAddress         | String                | 操作发起源账户地址    |
+-----------------------+-----------------------+-----------------------+
| metadata              | String                | 备注                  |
+-----------------------+-----------------------+-----------------------+
| createAccount         | `OperationCreateAccou | 创建账户操作          |
|                       | nt </he-yue-fu-wu/cal |                       |
|                       | l/operationcreateacco |                       |
|                       | unt.md>`__            |                       |
+-----------------------+-----------------------+-----------------------+
| issueAsset            | `OperationIssueAsset  | 发行资产操作          |
|                       | </he-yue-fu-wu/call/o |                       |
|                       | perationissueasset.md |                       |
|                       | >`__                  |                       |
+-----------------------+-----------------------+-----------------------+
| payAsset              | `OperationPayAsset </ | 转移资产操作          |
|                       | he-yue-fu-wu/call/ope |                       |
|                       | rationpayasset.md>`__ |                       |
+-----------------------+-----------------------+-----------------------+
| payCoin               | `OperationPayCoin </h | 发送BU操作            |
|                       | e-yue-fu-wu/call/oper |                       |
|                       | ationpaycoin.md>`__   |                       |
+-----------------------+-----------------------+-----------------------+
| setMetadata           | `OperationSetMetadata | 设置metadata操作      |
|                       |  </he-yue-fu-wu/call/ |                       |
|                       | operationsetmetadata. |                       |
|                       | md>`__                |                       |
+-----------------------+-----------------------+-----------------------+
| setPrivilege          | `OperationSetPrivileg | 设置账户权限操作      |
|                       | e </he-yue-fu-wu/call |                       |
|                       | /operationsetprivileg |                       |
|                       | e.md>`__              |                       |
+-----------------------+-----------------------+-----------------------+
| log                   | `OperationLog </he-yu | 记录日志              |
|                       | e-fu-wu/call/operatio |                       |
|                       | nlog.md>`__           |                       |
+-----------------------+-----------------------+-----------------------+

TriggerTransaction
^^^^^^^^^^^^^^^^^^

TriggerTransaction的具体信息如下表所示：

+------+--------+----------+
| 成员 | 类型   | 描述     |
+======+========+==========+
| hash | String | 交易hash |
+------+--------+----------+

OperationCreateAccount
^^^^^^^^^^^^^^^^^^^^^^

OperationCreateAccount的具体信息如下表所示：

+-----------------------+-----------------------+-----------------------+
| 成员                  | 类型                  | 描述                  |
+=======================+=======================+=======================+
| destAddress           | String                | 目标账户地址          |
+-----------------------+-----------------------+-----------------------+
| contract              | `Contract </he-yue-fu | 合约信息              |
|                       | -wu/call/contract.md> |                       |
|                       | `__                   |                       |
+-----------------------+-----------------------+-----------------------+
| priv                  | `Priv </zhang-hu-fu-w | 账户权限              |
|                       | u/getinfo/priv.md>`__ |                       |
+-----------------------+-----------------------+-----------------------+
| metadata              | `MetadataInfo </zhang | 账户                  |
|                       | -hu-fu-wu/getmetadata |                       |
|                       | /metadatainfo.md>`__\ |                       |
|                       |  []                   |                       |
+-----------------------+-----------------------+-----------------------+
| initBalance           | Long                  | 账户资产, 单位MO，1   |
|                       |                       | BU = 10^8 MO,         |
+-----------------------+-----------------------+-----------------------+
| initInput             | String                | 合约init函数的入参    |
+-----------------------+-----------------------+-----------------------+

Contract
^^^^^^^^

Contract的具体信息如下表所示：

+---------+---------+------------------------+
| 成员    | 类型    | 描述                   |
+=========+=========+========================+
| type    | Integer | 合约的语种，默认不赋值 |
+---------+---------+------------------------+
| payload | String  | 对应语种的合约代码     |
+---------+---------+------------------------+

.. _metadatainfo-1:

MetadataInfo
^^^^^^^^^^^^

MetadataInfo的具体信息如下表所示:

+---------+--------+------------------+
| 成员    | 类型   | 描述             |
+=========+========+==================+
| key     | String | metadata的关键词 |
+---------+--------+------------------+
| value   | String | metadata的内容   |
+---------+--------+------------------+
| version | Long   | metadata的版本   |
+---------+--------+------------------+

OperationIssueAsset
^^^^^^^^^^^^^^^^^^^

OperationIssueAsset的具体信息如下表所示:

+-------------+--------+----------+
| 成员        | 类型   | 描述     |
+=============+========+==========+
| code        | String | 资产编码 |
+-------------+--------+----------+
| assetAmount | Long   | 资产数量 |
+-------------+--------+----------+

OperationPayAsset
^^^^^^^^^^^^^^^^^

OperationPayAsset的具体信息如下表所示:

+-----------------------+-----------------------+-----------------------+
| 成员                  | 类型                  | 描述                  |
+=======================+=======================+=======================+
| destAddress           | String                | 待转移资产的目标账户地址 |
+-----------------------+-----------------------+-----------------------+
| asset                 | `AssetInfo </zhang-hu | 账户资产              |
|                       | -fu-wu/getassets/asse |                       |
|                       | tinfo.md>`__          |                       |
+-----------------------+-----------------------+-----------------------+
| input                 | String                | 合约main函数入参      |
+-----------------------+-----------------------+-----------------------+

OperationPayCoin
^^^^^^^^^^^^^^^^

OperationPayCoin的具体信息如下表所示:

+-------------+--------+----------------------+
| 成员        | 类型   | 描述                 |
+=============+========+======================+
| destAddress | String | 待转移的目标账户地址 |
+-------------+--------+----------------------+
| buAmount    | Long   | 待转移的BU数量       |
+-------------+--------+----------------------+
| input       | String | 合约main函数入参     |
+-------------+--------+----------------------+

OperationSetMetadata
^^^^^^^^^^^^^^^^^^^^

OperationSetMetadata的具体信息如下表所示:

+------------+---------+------------------+
| 成员       | 类型    | 描述             |
+============+=========+==================+
| key        | String  | metadata的关键词 |
+------------+---------+------------------+
| value      | String  | metadata的内容   |
+------------+---------+------------------+
| version    | Long    | metadata的版本   |
+------------+---------+------------------+
| deleteFlag | boolean | 是否删除metadata |
+------------+---------+------------------+

OperationSetPrivilege
^^^^^^^^^^^^^^^^^^^^^

OperationSetPrivilege的具体信息如下表所示:

+-----------------------+-----------------------+-----------------------+
| 成员                  | 类型                  | 描述                  |
+=======================+=======================+=======================+
| masterWeight          | String                | 账户自身权重，大小限制[0, |
|                       |                       |                       |
|                       |                       | (Integer.MAX_VALUE \* |
|                       |                       | 2L + 1)]              |
+-----------------------+-----------------------+-----------------------+
| signers               | `Signer </zhang-hu-fu | 签名者权重列表        |
|                       | -wu/getinfo/signer.md |                       |
|                       | >`__\ []              |                       |
+-----------------------+-----------------------+-----------------------+
| txThreshold           | String                | 交易门限，大小限制[0, |
|                       |                       | Long.MAX_VALUE]       |
+-----------------------+-----------------------+-----------------------+
| typeThreshold         | `TypeThreshold </zhan | 指定类型交易门限      |
|                       | g-hu-fu-wu/getinfo/ty |                       |
|                       | pethreashold.md>`__   |                       |
+-----------------------+-----------------------+-----------------------+

OperationLog
^^^^^^^^^^^^

OperationLog的具体信息如下表所示:

+-------+----------+----------+
| 成员  | 类型     | 描述     |
+=======+==========+==========+
| topic | String   | 日志主题 |
+-------+----------+----------+
| data  | String[] | 日志内容 |
+-------+----------+----------+

交易服务
--------

交易服务提供交易相关的接口，目前有5个接口：buildBlob, evaluateFee, sign,
submit, getInfo。

buildBlob
~~~~~~~~~

buildBlob接口用于序列化交易，生成交易Blob串，便于网络传输。

**说明**\ ：在调用buildBlob之前需要构建一些操作，目前操作有16种，分别是：AccountActivateOperation、AccountSetMetadataOperation、
AccountSetPrivilegeOperation、AssetIssueOperation、AssetSendOperation、BUSendOperation、TokenIssueOperation、TokenTransferOperation、TokenTransferFromOperation、TokenApproveOperation、TokenAssignOperation、TokenChangeOwnerOperation、ContractCreateOperation、ContractInvokeByAssetOperation、ContractInvokeByBUOperation、LogCreateOperation。

   调用方法如下所示：

TransactionBuildBlobResponse buildBlob(TransactionBuildBlobRequest);

   请求参数如下表所示：

+-------------------+---------------------+----------------------------+
| 参数              | 类型                | 描述                       |
+===================+=====================+============================+
| sourceAddress     | String              | 必填，发起该操作的源账户地址 |
+-------------------+---------------------+----------------------------+
| nonce             | Long                | 必填，待发起的交易序列号，函数里+1，大小限制[1, |
|                   |                     |                            |
|                   |                     | Long.MAX_VALUE]            |
+-------------------+---------------------+----------------------------+
| gasPrice          | Long                | 必填，交易燃料单价，单位MO，1 |
|                   |                     |                            |
|                   |                     | BU = 10^8                  |
|                   |                     | MO，大小限制[1000,         |
|                   |                     | Long.MAX_VALUE]            |
+-------------------+---------------------+----------------------------+
| feeLimit          | Long                | 必填，交易要求的最低的手续费，单位MO，1 |
|                   |                     |                            |
|                   |                     | BU = 10^8 MO，大小限制[1,  |
|                   |                     | Long.MAX_VALUE]            |
+-------------------+---------------------+----------------------------+
| operation         | BaseOperation[]     | 必填，待提交的操作列表，不能为空 |
+-------------------+---------------------+----------------------------+
| ceilLedgerSeq     | long                | 选填，距离当前区块高度指定差值的区块内执行的限制，当 |
|                   |                     | 区块超出当时区块高度与所设差值的和后，交易执行失败。 |
|                   |                     | 必须大于等于0，是0时不限制 |
+-------------------+---------------------+----------------------------+
| metadata          | String              | 选填，备注                 |
+-------------------+---------------------+----------------------------+

..

   响应数据如下表所示：

+-----------------+--------+-----------------------------------+
| 参数            | 类型   | 描述                              |
+=================+========+===================================+
| transactionBlob | String | Transaction序列化后的16进制字符串 |
+-----------------+--------+-----------------------------------+
| hash            | String | 交易hash                          |
+-----------------+--------+-----------------------------------+

..

   错误码如下表所示：

+-------------------------+-------------------------+------------------+
| 异常                    | 错误码                  | 描述             |
+=========================+=========================+==================+
| INVALID_SOURCEADDRESS_E | 11002                   | Invalid          |
| RROR                    |                         | sourceAddress    |
+-------------------------+-------------------------+------------------+
| INVALID_NONCE_ERROR     | 11048                   | Nonce must be    |
|                         |                         | between 1 and    |
|                         |                         | Long.MAX_VALUE   |
+-------------------------+-------------------------+------------------+
| INVALID_DESTADDRESS_ERR | 11003                   | Invalid          |
| OR                      |                         | destAddress      |
+-------------------------+-------------------------+------------------+
| INVALID_INITBALANCE_ERR | 11004                   | InitBalance must |
| OR                      |                         | be between 1 and |
|                         |                         | Long.MAX_VALUE   |
+-------------------------+-------------------------+------------------+
| SOURCEADDRESS_EQUAL_DES | 11005                   | SourceAddress    |
| TADDRESS_ERROR          |                         | cannot be equal  |
|                         |                         | to destAddress   |
+-------------------------+-------------------------+------------------+
| INVALID_ISSUE_AMMOUNT_E | 11008                   | AssetAmount this |
| RROR                    |                         | will be issued   |
|                         |                         | must be between  |
|                         |                         | 1 and            |
|                         |                         | Long.MAX_VALUE   |
+-------------------------+-------------------------+------------------+
| INVALID_DATAKEY_ERROR   | 11011                   | The length of    |
|                         |                         | key must be      |
|                         |                         | between 1 and    |
|                         |                         | 1024             |
+-------------------------+-------------------------+------------------+
| INVALID_DATAVALUE_ERROR | 11012                   | The length of    |
|                         |                         | value must be    |
|                         |                         | between 0 and    |
|                         |                         | 256000           |
+-------------------------+-------------------------+------------------+
| INVALID_DATAVERSION_ERR | 11013                   | The version must |
| OR                      |                         | be equal to or   |
|                         |                         | greater than 0   |
+-------------------------+-------------------------+------------------+
| INVALID_MASTERWEIGHT    | 11015                   | MasterWeight     |
| \_ERROR                 |                         | must be between  |
|                         |                         | 0 and            |
|                         |                         | (Integer.MAX_VAL |
|                         |                         | UE               |
|                         |                         | \* 2L + 1)       |
+-------------------------+-------------------------+------------------+
| INVALID_SIGNER_ADDRESS_ | 11016                   | Invalid signer   |
| ERROR                   |                         | address          |
+-------------------------+-------------------------+------------------+
| INVALID_SIGNER_WEIGHT   | 11017                   | Signer weight    |
| \_ERROR                 |                         | must be between  |
|                         |                         | 0 and            |
|                         |                         | (Integer.MAX_VAL |
|                         |                         | UE               |
|                         |                         | \* 2L + 1)       |
+-------------------------+-------------------------+------------------+
| INVALID_TX_THRESHOLD_ER | 11018                   | TxThreshold must |
| ROR                     |                         | be between 0 and |
|                         |                         | Long.MAX_VALUE   |
+-------------------------+-------------------------+------------------+
| INVALID_OPERATION_TYPE_ | 11019                   | Operation type   |
| ERROR                   |                         | must be between  |
|                         |                         | 1 and 100        |
+-------------------------+-------------------------+------------------+
| INVALID_TYPE_THRESHOLD_ | 11020                   | TypeThreshold    |
| ERROR                   |                         | must be between  |
|                         |                         | 0 and            |
|                         |                         | Long.MAX_VALUE   |
+-------------------------+-------------------------+------------------+
| INVALID_ASSET_CODE      | 11023                   | The length of    |
| \_ERROR                 |                         | key must be      |
|                         |                         | between 1 and 64 |
+-------------------------+-------------------------+------------------+
| INVALID_ASSET_AMOUNT_ER | 11024                   | AssetAmount must |
| ROR                     |                         | be between 0 and |
|                         |                         | Long.MAX_VALUE   |
+-------------------------+-------------------------+------------------+
| INVALID_BU_AMOUNT_ERROR | 11026                   | BuAmount must be |
|                         |                         | between 0 and    |
|                         |                         | Long.MAX_VALUE   |
+-------------------------+-------------------------+------------------+
| INVALID_ISSUER_ADDRESS_ | 11027                   | Invalid issuer   |
| ERROR                   |                         | address          |
+-------------------------+-------------------------+------------------+
| NO_SUCH_TOKEN_ERROR     | 11030                   | No such token    |
+-------------------------+-------------------------+------------------+
| INVALID_TOKEN_NAME_ERRO | 11031                   | The length of    |
| R                       |                         | token name must  |
|                         |                         | be between 1 and |
|                         |                         | 1024             |
+-------------------------+-------------------------+------------------+
| INVALID_TOKEN_SYMBOL_ER | 11032                   | The length of    |
| ROR                     |                         | symbol must be   |
|                         |                         | between 1 and    |
|                         |                         | 1024             |
+-------------------------+-------------------------+------------------+
| INVALID_TOKEN_DECIMALS_ | 11033                   | Decimals must be |
| ERROR                   |                         | between 0 and 8  |
+-------------------------+-------------------------+------------------+
| INVALID_TOKEN_TOTALSUPP | 11034                   | TotalSupply must |
| LY_ERROR                |                         | be between 1 and |
|                         |                         | Long.MAX_VALUE   |
+-------------------------+-------------------------+------------------+
| INVALID_TOKENOWNER_ERRP | 11035                   | Invalid token    |
| R                       |                         | owner            |
+-------------------------+-------------------------+------------------+
| INVALID_CONTRACTADDRESS | 11037                   | Invalid contract |
| _ERROR                  |                         | address          |
+-------------------------+-------------------------+------------------+
| CONTRACTADDRESS_NOT_CON | 11038                   | ContractAddress  |
| TRACTACCOUNT_ERROR      |                         | is not a         |
|                         |                         | contract account |
+-------------------------+-------------------------+------------------+
| INVALID_TOKEN_AMOUNT_ER | 11039                   | Token amount     |
| ROR                     |                         | must be between  |
|                         |                         | 1 and            |
|                         |                         | Long.MAX_VALUE   |
+-------------------------+-------------------------+------------------+
| SOURCEADDRESS_EQUAL_CON | 11040                   | SourceAddress    |
| TRACTADDRESS_ERROR      |                         | cannot be equal  |
|                         |                         | to               |
|                         |                         | contractAddress  |
+-------------------------+-------------------------+------------------+
| INVALID_FROMADDRESS_ERR | 11041                   | Invalid          |
| OR                      |                         | fromAddress      |
+-------------------------+-------------------------+------------------+
| FROMADDRESS_EQUAL_DESTA | 11042                   | FromAddress      |
| DDRESS_ERROR            |                         | cannot be equal  |
|                         |                         | to destAddress   |
+-------------------------+-------------------------+------------------+
| INVALID_SPENDER_ERROR   | 11043                   | Invalid spender  |
+-------------------------+-------------------------+------------------+
| PAYLOAD_EMPTY_ERROR     | 11044                   | Payload cannot   |
|                         |                         | be empty         |
+-------------------------+-------------------------+------------------+
| INVALID_LOG_TOPIC       | 11045                   | The length of    |
| \_ERROR                 |                         | key must be      |
|                         |                         | between 1 and    |
|                         |                         | 128              |
+-------------------------+-------------------------+------------------+
| INVALID_LOG_DATA        | 11046                   | The length of    |
| \_ERROR                 |                         | value must be    |
|                         |                         | between 1 and    |
|                         |                         | 1024             |
+-------------------------+-------------------------+------------------+
| INVALID_CONTRACT_TYPE_E | 11047                   | Type must be     |
| RROR                    |                         | equal to or      |
|                         |                         | greater than 0   |
+-------------------------+-------------------------+------------------+
| INVALID_NONCE_ERROR     | 11048                   | Nonce must be    |
|                         |                         | between 1 and    |
|                         |                         | Long.MAX_VALUE   |
+-------------------------+-------------------------+------------------+
| INVALID\_               | 11049                   | GasPrice must be |
| GASPRICE_ERROR          |                         | between 1000 and |
|                         |                         | Long.MAX_VALUE   |
+-------------------------+-------------------------+------------------+
| INVALID_FEELIMIT_ERROR  | 11050                   | FeeLimit must be |
|                         |                         | between 1 and    |
|                         |                         | Long.MAX_VALUE   |
+-------------------------+-------------------------+------------------+
| OPERATIONS_EMPTY_ERROR  | 11051                   | Operations       |
|                         |                         | cannot be empty  |
+-------------------------+-------------------------+------------------+
| INVALID_CEILLEDGERSEQ_E | 11052                   | CeilLedgerSeq    |
| RROR                    |                         | must be equal or |
|                         |                         | bigger than 0    |
+-------------------------+-------------------------+------------------+
| OPERATIONS_ONE_ERROR    | 11053                   | One of the       |
|                         |                         | operations       |
|                         |                         | cannot be        |
|                         |                         | resolved         |
+-------------------------+-------------------------+------------------+
| REQUEST_NULL_ERROR      | 12001                   | Request          |
|                         |                         | parameter cannot |
|                         |                         | be null          |
+-------------------------+-------------------------+------------------+
| SYSTEM_ERROR            | 20000                   | System error     |
+-------------------------+-------------------------+------------------+

..

   具体示例如下所示：

::

   // 初始化变量
   String senderAddresss = "buQfnVYgXuMo3rvCEpKA6SfRrDpaz8D8A9Ea";
   String destAddress = "buQsurH1M4rjLkfjzkxR9KXJ6jSu2r9xBNEw";
   Long buAmount = ToBaseUnit.BU2MO("10.9");
   Long gasPrice = 1000L;
   Long feeLimit = ToBaseUnit.BU2MO("0.01");
   Long nonce = 1L;

   // 构建sendBU操作
   BUSendOperation operation = new BUSendOperation();
   operation.setSourceAddress(senderAddresss);
   operation.setDestAddress(destAddress);
   operation.setAmount(buAmount);

   // 初始化请求参数
   TransactionBuildBlobRequest request = new TransactionBuildBlobRequest();
   request.setSourceAddress(senderAddresss);
   request.setNonce(nonce);
   request.setFeeLimit(feeLimit);
   request.setGasPrice(gasPrice);
   request.addOperation(operation);

   // 调用buildBlob接口
   String transactionBlob = null;
   TransactionBuildBlobResponse response = sdk.getTransactionService().buildBlob(request);
   if (response.getErrorCode() == 0) {
   TransactionBuildBlobResult result = response.getResult();
   System.out.println(JSON.toJSONString(result, true));
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

BaseOperation
^^^^^^^^^^^^^

BaseOperation是buildBlob接口中所有操作的基类。下表对BaseOperation进行了说明：

+---------------+--------+----------------------+
| 成员变量      | 类型   | 描述                 |
+===============+========+======================+
| sourceAddress | String | 选填，操作源账户地址 |
+---------------+--------+----------------------+
| metadata      | String | 选填，备注           |
+---------------+--------+----------------------+

AccountActivateOperation
^^^^^^^^^^^^^^^^^^^^^^^^

AccountActivateOperation继承于BaseOperation，feeLimit目前(2018.07.26)固定是0.01
BU。

+-----------------------+-----------------------+-----------------------+
| 成员变量              | 类型                  | 描述                  |
+=======================+=======================+=======================+
| sourceAddress         | String                | 选填，操作源账户地址  |
+-----------------------+-----------------------+-----------------------+
| destAddress           | String                | 必填，目标账户地址    |
+-----------------------+-----------------------+-----------------------+
| initBalance           | Long                  | 必填，初始化资产，单位MO，1 |
|                       |                       |                       |
|                       |                       | BU = 10^8 MO, 大小(0, |
|                       |                       | Long.MAX_VALUE]       |
+-----------------------+-----------------------+-----------------------+
| metadata              | String                | 选填，备注            |
+-----------------------+-----------------------+-----------------------+

AccountSetMetadataOperation
^^^^^^^^^^^^^^^^^^^^^^^^^^^

AccountSetMetadataOperation继承于BaseOperation，feeLimit目前(2018.07.26)固定是0.01
BU。

+---------------+---------+-------------------------------------------+
| 成员变量      | 类型    | 描述                                      |
+===============+=========+===========================================+
| sourceAddress | String  | 选填，操作源账户地址                      |
+---------------+---------+-------------------------------------------+
| key           | String  | 必填，metadata的关键词，长度限制[1, 1024] |
+---------------+---------+-------------------------------------------+
| value         | String  | 必填，metadata的内容，长度限制[0, 256000] |
+---------------+---------+-------------------------------------------+
| version       | Long    | 选填，metadata的版本                      |
+---------------+---------+-------------------------------------------+
| deleteFlag    | Boolean | 选填，是否删除metadata                    |
+---------------+---------+-------------------------------------------+
| metadata      | String  | 选填，备注                                |
+---------------+---------+-------------------------------------------+

AccountSetPrivilegeOperation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

AccountSetPrivilegeOperation继承于BaseOperation，feeLimit目前(2018.07.26)固定是0.01
BU。

+-----------------------+-----------------------+-----------------------+
| 成员变量              | 类型                  | 描述                  |
+=======================+=======================+=======================+
| sourceAddress         | String                | 选填，操作源账户地址  |
+-----------------------+-----------------------+-----------------------+
| masterWeight          | String                | 选填，账户自身权重，大小限制[0, |
|                       |                       |                       |
|                       |                       | (Integer.MAX_VALUE \* |
|                       |                       | 2L + 1)]              |
+-----------------------+-----------------------+-----------------------+
| signers               | `Signer </zhang-hu-fu | 选填，签名者权重列表  |
|                       | -wu/getinfo/signer.md |                       |
|                       | >`__\ []              |                       |
+-----------------------+-----------------------+-----------------------+
| txThreshold           | String                | 选填，交易门限，大小限制[0, |
|                       |                       |                       |
|                       |                       | Long.MAX_VALUE]       |
+-----------------------+-----------------------+-----------------------+
| typeThreshold         | `TypeThreshold </zhan | 选填，指定类型交易门限 |
|                       | g-hu-fu-wu/getinfo/ty |                       |
|                       | pethreashold.md>`__\  |                       |
|                       | []                    |                       |
+-----------------------+-----------------------+-----------------------+
| metadata              | String                | 选填，备注            |
+-----------------------+-----------------------+-----------------------+

AssetIssueOperation
^^^^^^^^^^^^^^^^^^^

AssetIssueOperation继承于BaseOperation，feeLimit目前(2018.07.26)固定是50.01
BU。

+---------------+--------+-------------------------------------------------+
| 成员变量      | 类型   | 描述                                            |
+===============+========+=================================================+
| sourceAddress | String | 选填，操作源账户地址                            |
+---------------+--------+-------------------------------------------------+
| code          | String | 必填，资产编码，长度限制[1, 64]                 |
+---------------+--------+-------------------------------------------------+
| assetAmount   | Long   | 必填，资产发行数量，大小限制[0, Long.MAX_VALUE] |
+---------------+--------+-------------------------------------------------+
| metadata      | String | 选填，备注                                      |
+---------------+--------+-------------------------------------------------+

AssetSendOperation
^^^^^^^^^^^^^^^^^^

AssetSendOperation继承于BaseOperation，feeLimit目前(2018.07.26)固定是0.01
BU。

+---------------+--------+---------------------------------------------+
| 成员变量      | 类型   | 描述                                        |
+===============+========+=============================================+
| sourceAddress | String | 选填，操作源账户地址                        |
+---------------+--------+---------------------------------------------+
| destAddress   | String | 必填，目标账户地址                          |
+---------------+--------+---------------------------------------------+
| code          | String | 必填，资产编码，长度限制[1, 64]             |
+---------------+--------+---------------------------------------------+
| issuer        | String | 必填，资产发行账户地址                      |
+---------------+--------+---------------------------------------------+
| assetAmount   | Long   | 必填，资产数量，大小限制[0, Long.MAX_VALUE] |
+---------------+--------+---------------------------------------------+
| metadata      | String | 选填，备注                                  |
+---------------+--------+---------------------------------------------+

BUSendOperation
^^^^^^^^^^^^^^^

BUSendOperation继承于BaseOperation，feeLimit目前(2018.07.26)固定是0.01
BU。

+---------------+--------+-------------------------------------------------+
| 成员变量      | 类型   | 描述                                            |
+===============+========+=================================================+
| sourceAddress | String | 选填，操作源账户地址                            |
+---------------+--------+-------------------------------------------------+
| destAddress   | String | 必填，目标账户地址                              |
+---------------+--------+-------------------------------------------------+
| buAmount      | Long   | 必填，资产发行数量，大小限制[0, Long.MAX_VALUE] |
+---------------+--------+-------------------------------------------------+
| metadata      | String | 选填，备注                                      |
+---------------+--------+-------------------------------------------------+

Ctp10TokenIssueOperation
^^^^^^^^^^^^^^^^^^^^^^^^

Ctp10TokenIssueOperation继承于BaseOperation，feeLimit目前(2018.07.26)固定是10.08
BU。

+-----------------------+-----------------------+-----------------------+
| 成员变量              | 类型                  | 描述                  |
+=======================+=======================+=======================+
| sourceAddress         | String                | 选填，操作源账户地址  |
+-----------------------+-----------------------+-----------------------+
| initBalance           | Long                  | 必填，给合约账户的初始化资产，单位MO，1 |
|                       |                       |                       |
|                       |                       | BU = 10^8 MO,         |
|                       |                       | 大小限制[1, max(64)]  |
+-----------------------+-----------------------+-----------------------+
| name                  | String                | 必填，ctp10Token名称，长度限制[ |
|                       |                       | 1,                    |
|                       |                       | 1024]                 |
+-----------------------+-----------------------+-----------------------+
| symbol                | String                | 必填，ctp10Token符号，长度限制[ |
|                       |                       | 1,                    |
|                       |                       | 1024]                 |
+-----------------------+-----------------------+-----------------------+
| decimals              | Integer               | 必填，ctp10Token数量的精度，大小 |
|                       |                       | 限制[0,               |
|                       |                       | 8]                    |
+-----------------------+-----------------------+-----------------------+
| supply                | String                | 必填，ctp10Token发行的总供应量( |
|                       |                       | 不带精度)，大小限制[1, |
|                       |                       |                       |
|                       |                       | Long.MAX_VALUE]       |
+-----------------------+-----------------------+-----------------------+
| metadata              | String                | 选填，备注            |
+-----------------------+-----------------------+-----------------------+

Ctp10TokenTransferOperation
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Ctp10TokenTransferOperation继承于BaseOperation，feeLimit目前(2018.07.26)固定是0.02
BU

+-----------------------+-----------------------+-----------------------+
| 成员变量              | 类型                  | 描述                  |
+=======================+=======================+=======================+
| sourceAddress         | String                | 选填，合约token的持有者账户地址 |
+-----------------------+-----------------------+-----------------------+
| contractAddress       | String                | 必填，合约账户地址    |
+-----------------------+-----------------------+-----------------------+
| destAddress           | String                | 必填，待转移的目标账户地址 |
+-----------------------+-----------------------+-----------------------+
| tokenAmount           | String                | 必填，待转移的token数量，大小限制[1 |
|                       |                       | ,                     |
|                       |                       | Long.MAX_VALUE]       |
+-----------------------+-----------------------+-----------------------+
| metadata              | String                | 选填，备注            |
+-----------------------+-----------------------+-----------------------+

TokenTransferFromOperation
^^^^^^^^^^^^^^^^^^^^^^^^^^

TokenTransferFromOperation继承于BaseOperation，feeLimit目前(2018.07.26)固定是0.02
BU。

+-----------------------+-----------------------+-----------------------+
| 成员变量              | 类型                  | 描述                  |
+=======================+=======================+=======================+
| sourceAddress         | String                | 选填，操作源账户地址  |
+-----------------------+-----------------------+-----------------------+
| contractAddress       | String                | 必填，合约账户地址    |
+-----------------------+-----------------------+-----------------------+
| fromAddress           | String                | 必填，待转移的源账户地址 |
+-----------------------+-----------------------+-----------------------+
| destAddress           | String                | 必填，待转移的目标账户地址 |
+-----------------------+-----------------------+-----------------------+
| tokenAmount           | String                | 必填，待转移的ctp10Token数量，大 |
|                       |                       | 小限制[1,             |
|                       |                       | Long.MAX_VALUE]       |
+-----------------------+-----------------------+-----------------------+
| metadata              | String                | 选填，备注            |
+-----------------------+-----------------------+-----------------------+

Ctp10TokenApproveOperation
^^^^^^^^^^^^^^^^^^^^^^^^^^

Ctp10TokenApproveOperation继承于BaseOperation，feeLimit目前(2018.07.26)固定是0.02
BU。

+-----------------------+-----------------------+-----------------------+
| 成员变量              | 类型                  | 描述                  |
+=======================+=======================+=======================+
| sourceAddress         | String                | 选填，合约token的持有者账户地址 |
+-----------------------+-----------------------+-----------------------+
| contractAddress       | String                | 必填，合约账户地址    |
+-----------------------+-----------------------+-----------------------+
| spender               | String                | 必填，授权的账户地址  |
+-----------------------+-----------------------+-----------------------+
| tokenAmount           | String                | 必填，被授权的待转移的ctp10Token |
|                       |                       | 数量，大小限制[1,     |
|                       |                       | Long.MAX_VALUE]       |
+-----------------------+-----------------------+-----------------------+
| metadata              | String                | 选填，备注            |
+-----------------------+-----------------------+-----------------------+

Ctp10TokenAssignOperation
^^^^^^^^^^^^^^^^^^^^^^^^^

Ctp10TokenAssignOperation继承于BaseOperation，feeLimit目前(2018.07.26)固定是0.02
BU。

+-----------------------+-----------------------+-----------------------+
| 成员变量              | 类型                  | 描述                  |
+=======================+=======================+=======================+
| sourceAddress         | String                | 选填，合约token的拥有者账户地址 |
+-----------------------+-----------------------+-----------------------+
| contractAddress       | String                | 必填，合约账户地址    |
+-----------------------+-----------------------+-----------------------+
| destAddress           | String                | 必填，待分配的目标账户地址 |
+-----------------------+-----------------------+-----------------------+
| tokenAmount           | String                | 必填，待分配的ctp10Token数量，大 |
|                       |                       | 小限制[1,             |
|                       |                       | Long.MAX_VALUE]       |
+-----------------------+-----------------------+-----------------------+
| metadata              | String                | 选填，备注            |
+-----------------------+-----------------------+-----------------------+

Ctp10TokenChangeOwnerOperation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Ctp10TokenChangeOwnerOperation继承于BaseOperation，feeLimit目前(2018.07.26)固定是0.02
BU。

+-----------------+--------+---------------------------------+
| 成员变量        | 类型   | 描述                            |
+=================+========+=================================+
| sourceAddress   | String | 选填，合约token的拥有者账户地址 |
+-----------------+--------+---------------------------------+
| contractAddress | String | 必填，合约账户地址              |
+-----------------+--------+---------------------------------+
| tokenOwner      | String | 必填，待转移的目标账户地址      |
+-----------------+--------+---------------------------------+
| metadata        | String | 选填，备注                      |
+-----------------+--------+---------------------------------+

ContractCreateOperation
^^^^^^^^^^^^^^^^^^^^^^^

ContractCreateOperation继承于BaseOperation，feeLimit目前(2018.07.26)固定是10.01
BU。

+--------------------+--------------+----------------------------------+
| 成员变量           | 类型         | 描述                             |
+====================+==============+==================================+
| sourceAddress      | String       | 选填，操作源账户地址             |
+--------------------+--------------+----------------------------------+
| initBalance        | Long         | 必填，给合约账户的初始化资产，单位MO，1 |
|                    |              |                                  |
|                    |              | BU = 10^8 MO, 大小限制[1,        |
|                    |              | Long.MAX_VALUE]                  |
+--------------------+--------------+----------------------------------+
| type               | Integer      | 选填，合约的语种，默认是0        |
+--------------------+--------------+----------------------------------+
| payload            | String       | 必填，对应语种的合约代码         |
+--------------------+--------------+----------------------------------+
| initInput          | String       | 选填，合约代码中init方法的入参   |
+--------------------+--------------+----------------------------------+
| metadata           | String       | 选填，备注                       |
+--------------------+--------------+----------------------------------+

ContractInvokeByAssetOperation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

ContractInvokeByAssetOperation继承于BaseOperation，feeLimit要根据合约中执行交易来做添加手续费，首先发起交易手续费，目前(2018.07.26)是0.01BU，然后合约中的交易也需要交易发起者添加相应交易的手续费。

+-----------------------+-----------------------+-----------------------+
| 成员变量              | 类型                  | 描述                  |
+=======================+=======================+=======================+
| sourceAddress         | String                | 选填，操作源账户地址  |
+-----------------------+-----------------------+-----------------------+
| contractAddress       | String                | 必填，合约账户地址    |
+-----------------------+-----------------------+-----------------------+
| code                  | String                | 选填，资产编码，长度限制[0, |
|                       |                       |                       |
|                       |                       | 1024];当为空时，仅触发合约; |
+-----------------------+-----------------------+-----------------------+
| issuer                | String                | 选填，资产发行账户地址，当null时，仅触 |
|                       |                       | 发合约                |
+-----------------------+-----------------------+-----------------------+
| assetAmount           | Long                  | 选填，资产数量，大小限制[0, |
|                       |                       |                       |
|                       |                       | Long.MAX_VALUE]，当是0时， |
|                       |                       | 仅触发合约            |
+-----------------------+-----------------------+-----------------------+
| input                 | String                | 选填，待触发的合约的main()入参 |
+-----------------------+-----------------------+-----------------------+
| metadata              | String                | 选填，备注            |
+-----------------------+-----------------------+-----------------------+

ContractInvokeByBUOperation
^^^^^^^^^^^^^^^^^^^^^^^^^^^

ContractInvokeByBUOperation继承于BaseOperation，feeLimit要根据合约中执行交易来添加手续费，首先发起交易手续费，目前(2018.07.26)是0.01BU，然后合约中的交易也需要交易发起者添加相应交易的手续费。

+-----------------------+-----------------------+-----------------------+
| 成员变量              | 类型                  | 描述                  |
+=======================+=======================+=======================+
| sourceAddress         | String                | 选填，操作源账户地址  |
+-----------------------+-----------------------+-----------------------+
| contractAddress       | String                | 必填，合约账户地址    |
+-----------------------+-----------------------+-----------------------+
| buAmount              | Long                  | 选填，资产发行数量，大小限制[0, |
|                       |                       |                       |
|                       |                       | Long.MAX_VALUE]，当0时仅触 |
|                       |                       | 发合约                |
+-----------------------+-----------------------+-----------------------+
| input                 | String                | 选填，待触发的合约的main()入参 |
+-----------------------+-----------------------+-----------------------+
| metadata              | String                | 选填，备注            |
+-----------------------+-----------------------+-----------------------+

evaluateFee
~~~~~~~~~~~

evaluateFee接口实现交易的费用评估。

   调用方法如下所示：

TransactionEvaluateFeeResponse evaluateFee
(TransactionEvaluateFeeRequest);

   请求参数如下表所示：

+-------------------+---------------------+----------------------------+
| 参数              | 类型                | 描述                       |
+===================+=====================+============================+
| sourceAddress     | String              | 必填，发起该操作的源账户地址 |
+-------------------+---------------------+----------------------------+
| nonce             | Long                | 必填，待发起的交易序列号，大小限制[1, |
|                   |                     |                            |
|                   |                     | Long.MAX_VALUE]            |
+-------------------+---------------------+----------------------------+
| operation         | `BaseOperation </ji | 必填，待提交的操作列表，不能为空 |
|                   | ao-yi-fu-wu/buildbl |                            |
|                   | ob/baseoperation.md |                            |
|                   | >`__\ []            |                            |
+-------------------+---------------------+----------------------------+
| signtureNumber    | Integer             | 选填，待签名者的数量，默认是1，大小限制[1, |
|                   |                     |                            |
|                   |                     | Integer.MAX_VALUE]         |
+-------------------+---------------------+----------------------------+
| ceilLedgerSeq     | Long                | 选填，距离当前区块高度指定差值的区块内执行的限制，当 |
|                   |                     | 区块超出当时区块高度与所设差值的和后，交易执行失败。 |
|                   |                     | 必须大于等于0，是0时不限制 |
+-------------------+---------------------+----------------------------+
| metadata          | String              | 选填，备注                 |
+-------------------+---------------------+----------------------------+

..

   响应数据如下表所示：

+------+-------------------------------------------------------+------------+
| 参数 | 类型                                                  | 描述       |
+======+=======================================================+============+
| txs  | `TestTx </jiao-yi-fu-wu/evaluatefee/testtx.md>`__\ [] | 评估交易集 |
+------+-------------------------------------------------------+------------+

..

   错误码如下表所示：

+-------------------------+-------------------------+------------------+
| 异常                    | 错误码                  | 描述             |
+=========================+=========================+==================+
| INVALID_SOURCEADDRESS_E | 11002                   | Invalid          |
| RROR                    |                         | sourceAddress    |
+-------------------------+-------------------------+------------------+
| INVALID_NONCE_ERROR     | 11045                   | Nonce must be    |
|                         |                         | between 1 and    |
|                         |                         | Long.MAX_VALUE   |
+-------------------------+-------------------------+------------------+
| OPERATIONS_EMPTY_ERROR  | 11051                   | Operations       |
|                         |                         | cannot be empty  |
+-------------------------+-------------------------+------------------+
| OPERATIONS_ONE_ERROR    | 11053                   | One of           |
|                         |                         | operations       |
|                         |                         | cannot be        |
|                         |                         | resolved         |
+-------------------------+-------------------------+------------------+
| INVALID_SIGNATURENUMBER | 11054                   | SignagureNumber  |
| _ERROR                  |                         | must be between  |
|                         |                         | 1 and            |
|                         |                         | Integer.MAX_VALU |
|                         |                         | E                |
+-------------------------+-------------------------+------------------+
| REQUEST_NULL_ERROR      | 12001                   | Request          |
|                         |                         | parameter cannot |
|                         |                         | be null          |
+-------------------------+-------------------------+------------------+
| SYSTEM_ERROR            | 20000                   | System error     |
+-------------------------+-------------------------+------------------+

..

   具体示例如下所示：

::

   // 初始化变量
   String senderAddresss = "buQnnUEBREw2hB6pWHGPzwanX7d28xk6KVcp";
   String destAddress = "buQfnVYgXuMo3rvCEpKA6SfRrDpaz8D8A9Ea";
   Long buAmount = ToBaseUnit.BU2MO("10.9");
   Long gasPrice = 1000L;
   Long feeLimit = ToBaseUnit.BU2MO("0.01");
   Long nonce = 51L;

   // 构建sendBU操作
   BUSendOperation buSendOperation = new BUSendOperation();
   buSendOperation.setSourceAddress(senderAddresss);
   buSendOperation.setDestAddress(destAddress);
   buSendOperation.setAmount(buAmount);

   // 初始化评估交易请求参数
   TransactionEvaluateFeeRequest request = new TransactionEvaluateFeeRequest();
   request.addOperation(buSendOperation);
   request.setSourceAddress(senderAddresss);
   request.setNonce(nonce);
   request.setSignatureNumber(1);
   request.setMetadata(HexFormat.byteToHex("evaluate fees".getBytes()));

   // 调用evaluateFee接口
   TransactionEvaluateFeeResponse response = sdk.getTransactionService().evaluateFee(request);
   if (response.getErrorCode() == 0) {
   TransactionEvaluateFeeResult result = response.getResult();
   System.out.println(JSON.toJSONString(result, true));
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

TestTx
^^^^^^

TestTx的具体信息如下表所示：

+-----------------------+-----------------------+-----------------------+
| 成员变量              | 类型                  | 描述                  |
+=======================+=======================+=======================+
| transactionEnv        | `TestTransactionFees  | 评估交易费用          |
|                       | </jiao-yi-fu-wu/evalu |                       |
|                       | atefee/testtransactio |                       |
|                       | nfees.md>`__          |                       |
+-----------------------+-----------------------+-----------------------+

TestTransactionFees
^^^^^^^^^^^^^^^^^^^

TestTransactionFees的具体信息如下表所示：

+-----------------------+-----------------------+-----------------------+
| 成员变量              | 类型                  | 描述                  |
+=======================+=======================+=======================+
| transactionFees       | `TransactionFees </ji | 交易费用              |
|                       | ao-yi-fu-wu/evaluatef |                       |
|                       | ee/transactionfees.md |                       |
|                       | >`__                  |                       |
+-----------------------+-----------------------+-----------------------+

TransactionFees
^^^^^^^^^^^^^^^

TransactionFees的具体信息如下表所示：

+----------+------+--------------------+
| 成员变量 | 类型 | 描述               |
+==========+======+====================+
| feeLimit | Long | 交易要求的最低费用 |
+----------+------+--------------------+
| gasPrice | Long | 交易燃料单价       |
+----------+------+--------------------+

sign
~~~~

sign接口用于实现交易的签名。

   调用方法如下所示：

TransactionSignResponse sign(TransactionSignRequest);

   请求参数如下表所示：

+-------------+----------+------------------------+
| 参数        | 类型     | 描述                   |
+=============+==========+========================+
| blob        | String   | 必填，待签名的交易Blob |
+-------------+----------+------------------------+
| privateKeys | String[] | 必填，私钥列表         |
+-------------+----------+------------------------+

..

   响应数据如下表所示：

+------------+-----------+------------------+
| 参数       | 类型      | 描述             |
+============+===========+==================+
| signatures | Signature | 签名后的数据列表 |
+------------+-----------+------------------+

..

   错误码如下表所示：

+-----------------------+--------+----------------------------------+
| 异常                  | 错误码 | 描述                             |
+=======================+========+==================================+
| INVALID_BLOB_ERROR    | 11056  | Invalid blob                     |
+-----------------------+--------+----------------------------------+
| PRIVATEKEY_NULL_ERROR | 11057  | PrivateKeys cannot be empty      |
+-----------------------+--------+----------------------------------+
| PRIVATEKEY_ONE_ERROR  | 11058  | One of privateKeys is invalid    |
+-----------------------+--------+----------------------------------+
| REQUEST_NULL_ERROR    | 12001  | Request parameter cannot be null |
+-----------------------+--------+----------------------------------+
| SYSTEM_ERROR          | 20000  | System error                     |
+-----------------------+--------+----------------------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   String issuePrivateKey = "privbyQCRp7DLqKtRFCqKQJr81TurTqG6UKXMMtGAmPG3abcM9XHjWvq";
   String []signerPrivateKeyArr = {issuePrivateKey};
   String transactionBlob = "0A246275516E6E5545425245773268423670574847507A77616E5837643238786B364B566370102118C0843D20E8073A56080712246275516E6E5545425245773268423670574847507A77616E5837643238786B364B566370522C0A24627551426A4A443142534A376E7A41627A6454656E416870466A6D7852564545746D78481080A9E08704";
   TransactionSignRequest request = new TransactionSignRequest();
   request.setBlob(transactionBlob);
   for (int i = 0; i < signerPrivateKeyArr.length; i++) {
   request.addPrivateKey(signerPrivateKeyArr[i]);
   }
   TransactionSignResponse response = sdk.getTransactionService().sign(request);
   if(0 == response.getErrorCode()){
   System.out.println(JSON.toJSONString(response.getResult(), true));
   }else{
   System.out.println("error: " + response.getErrorDesc());
   }

Signature
^^^^^^^^^

Signature的具体信息如下表所示：

+-----------+------+------------+
| 成员变量  | 类型 | 描述       |
+===========+======+============+
| signData  | Long | 签名后数据 |
+-----------+------+------------+
| publicKey | Long | 公钥       |
+-----------+------+------------+

submit
~~~~~~

submit接口用于实现交易的提交。

   调用方法如下所示：

TransactionSubmitResponse submit(TransactionSubmitRequest);

   请求参数如下表所示：

+-----------------------+-----------------------+-----------------------+
| 参数                  | 类型                  | 描述                  |
+=======================+=======================+=======================+
| blob                  | String                | 必填，交易blob        |
+-----------------------+-----------------------+-----------------------+
| signature             | `Signature </jiao-yi- | 必填，签名列表        |
|                       | fu-wu/sign/signature. |                       |
|                       | md>`__\ []            |                       |
+-----------------------+-----------------------+-----------------------+

..

   响应数据如下表所示：

+------+--------+----------+
| 参数 | 类型   | 描述     |
+======+========+==========+
| hash | String | 交易hash |
+------+--------+----------+

..

   错误码如下表所示：

+-----------------------+--------+----------------------------------+
| 异常                  | 错误码 | 描述                             |
+=======================+========+==================================+
| INVALID_BLOB_ERROR    | 11056  | Invalid blob                     |
+-----------------------+--------+----------------------------------+
| SIGNATURE_EMPTY_ERROR | 11067  | The signatures cannot be empty   |
+-----------------------+--------+----------------------------------+
| REQUEST_NULL_ERROR    | 12001  | Request parameter cannot be null |
+-----------------------+--------+----------------------------------+
| SYSTEM_ERROR          | 20000  | System error                     |
+-----------------------+--------+----------------------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   String transactionBlob = "0A246275516E6E5545425245773268423670574847507A77616E5837643238786B364B566370102118C0843D20E8073A56080712246275516E6E5545425245773268423670574847507A77616E5837643238786B364B566370522C0A24627551426A4A443142534A376E7A41627A6454656E416870466A6D7852564545746D78481080A9E08704";
   Signature signature = new Signature();
   signature.setSignData("D2B5E3045F2C1B7D363D4F58C1858C30ABBBB0F41E4B2E18AF680553CA9C3689078E215C097086E47A4393BCA715C7A5D2C180D8750F35C6798944F79CC5000A");
   signature.setPublicKey("b0011765082a9352e04678ef38d38046dc01306edef676547456c0c23e270aaed7ffe9e31477");
   TransactionSubmitRequest request = new TransactionSubmitRequest();
   request.setTransactionBlob(transactionBlob);
   request.addSignature(signature);

   // 调用submit接口
   TransactionSubmitResponse response = sdk.getTransactionService().submit(request);
   if (0 == response.getErrorCode()) { // 交易提交成功
   System.out.println(JSON.toJSONString(response.getResult(), true));
   } else{
   System.out.println("error: " + response.getErrorDesc());
   }

.. _getinfo-3:

getInfo
~~~~~~~

getInfo接口用于实现根据交易hash查询交易。

   调用方法如下所示：

TransactionGetInfoResponse getInfo (TransactionGetInfoRequest);

   请求参数如下表所示：

+------+--------+----------+
| 参数 | 类型   | 描述     |
+======+========+==========+
| hash | String | 交易hash |
+------+--------+----------+

..

   响应数据如下表所示：

+-----------------------+-----------------------+-----------------------+
| 参数                  | 类型                  | 描述                  |
+=======================+=======================+=======================+
| totalCount            | Long                  | 返回的总交易数        |
+-----------------------+-----------------------+-----------------------+
| transactions          | `TransactionHistory < | 交易内容              |
|                       | /jiao-yi-fu-wu/getinf |                       |
|                       | o/transactionhistory. |                       |
|                       | md>`__\ []            |                       |
+-----------------------+-----------------------+-----------------------+

..

   错误码如下表所示：

+----------------------+--------+----------------------------------+
| 异常                 | 错误码 | 描述                             |
+======================+========+==================================+
| INVALID_HASH_ERROR   | 11055  | Invalid transaction hash         |
+----------------------+--------+----------------------------------+
| REQUEST_NULL_ERROR   | 12001  | Request parameter cannot be null |
+----------------------+--------+----------------------------------+
| CONNECTNETWORK_ERROR | 11007  | Failed to connect to the network |
+----------------------+--------+----------------------------------+
| SYSTEM_ERROR         | 20000  | System error                     |
+----------------------+--------+----------------------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   String txHash = "1653f54fbba1134f7e35acee49592a7c29384da10f2f629c9a214f6e54747705";
   TransactionGetInfoRequest request = new TransactionGetInfoRequest();
   request.setHash(txHash);

   // 调用getInfo接口
   TransactionGetInfoResponse response = sdk.getTransactionService().getInfo(request);
   if (response.getErrorCode() == 0) {
   System.out.println(JSON.toJSONString(response.getResult(), true));
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

TransactionHistory
^^^^^^^^^^^^^^^^^^

TransactionHistory的具体信息如下表所示：

+-----------------------+-----------------------+-----------------------+
| 成员变量              | 类型                  | 描述                  |
+=======================+=======================+=======================+
| actualFee             | String                | 交易实际费用          |
+-----------------------+-----------------------+-----------------------+
| closeTime             | Long                  | 交易关闭时间          |
+-----------------------+-----------------------+-----------------------+
| errorCode             | Long                  | 交易错误码            |
+-----------------------+-----------------------+-----------------------+
| errorDesc             | String                | 交易描述              |
+-----------------------+-----------------------+-----------------------+
| hash                  | String                | 交易hash              |
+-----------------------+-----------------------+-----------------------+
| ledgerSeq             | Long                  | 区块序列号            |
+-----------------------+-----------------------+-----------------------+
| transaction           | `TransactionInfo </he | 交易内容列表          |
|                       | -yue-fu-wu/call/trans |                       |
|                       | actioninfo.md>`__     |                       |
+-----------------------+-----------------------+-----------------------+
| signatures            | `Signature </jiao-yi- | 签名列表              |
|                       | fu-wu/sign/signature. |                       |
|                       | md>`__\ []            |                       |
+-----------------------+-----------------------+-----------------------+
| txSize                | Long                  | 交易大小              |
+-----------------------+-----------------------+-----------------------+

区块服务
--------

区块服务提供区块相关的接口，目前有11个接口：getNumber、checkStatus、getTransactions、getInfo、getLatestInfo、getValidators、getLatestValidators、getReward、getLatestReward、getFees、getLatestFees。

getNumber
~~~~~~~~~

getNumber接口用于查询最新的区块高度。

   调用方法如下所示：

BlockGetNumberResponse getNumber();

   响应数据如下表所示：

+-------------+-------------+---------------------------------+
| 参数        | 类型        | 描述                            |
+=============+=============+=================================+
| header      | BlockHeader | 区块头                          |
+-------------+-------------+---------------------------------+
| blockNumber | Long        | 最新的区块高度，对应底层字段seq |
+-------------+-------------+---------------------------------+

..

   错误码如下表所示：

+----------------------+--------+----------------------------------+
| 异常                 | 错误码 | 描述                             |
+======================+========+==================================+
| CONNECTNETWORK_ERROR | 11007  | Failed to connect to the network |
+----------------------+--------+----------------------------------+
| SYSTEM_ERROR         | 20000  | System error                     |
+----------------------+--------+----------------------------------+

..

   具体示例如下所示：

::

   // 调用getNumber接口
   BlockGetNumberResponse response = sdk.getBlockService().getNumber();
   if(0 == response.getErrorCode()){
   System.out.println(JSON.toJSONString(response.getResult(), true));
   }else{
   System.out.println("error: " + response.getErrorDesc());
   }

checkStatus
~~~~~~~~~~~

checkStatus接口用于检查本地节点区块是否同步完成。

   调用方法如下所示：

BlockCheckStatusResponse checkStatus();

   响应数据如下表所示：

+---------------+---------+--------------+
| 参数          | 类型    | 描述         |
+===============+=========+==============+
| isSynchronous | Boolean | 区块是否同步 |
+---------------+---------+--------------+

..

   错误码如下表所示：

+----------------------+--------+----------------------------------+
| 异常                 | 错误码 | 描述                             |
+======================+========+==================================+
| CONNECTNETWORK_ERROR | 11007  | Failed to connect to the network |
+----------------------+--------+----------------------------------+
| SYSTEM_ERROR         | 20000  | System error                     |
+----------------------+--------+----------------------------------+

..

   具体示例如下所示：

::

   // 调用checkStatus
   BlockCheckStatusResponse response = sdk.getBlockService().checkStatus();
   if(0 == response.getErrorCode()){
   System.out.println(JSON.toJSONString(response.getResult(), true));
   }else{
   System.out.println("error: " + response.getErrorDesc());
   }

getTransactions
~~~~~~~~~~~~~~~

getTransactions接口用于查询指定区块高度下的所有交易。

   调用方法如下所示：

BlockGetTransactionsResponse
getTransactions(BlockGetTransactionsRequest);

   请求参数如下表所示：

+-------------+------+-----------------------------------+
| 参数        | 类型 | 描述                              |
+=============+======+===================================+
| blockNumber | Long | 必填，待查询的区块高度，必须大于0 |
+-------------+------+-----------------------------------+

..

   响应数据如下表所示：

+-----------------------+-----------------------+-----------------------+
| 参数                  | 类型                  | 描述                  |
+=======================+=======================+=======================+
| totalCount            | Long                  | 返回的总交易数        |
+-----------------------+-----------------------+-----------------------+
| transactions          | `TransactionHistory < | 交易内容              |
|                       | /jiao-yi-fu-wu/getinf |                       |
|                       | o/transactionhistory. |                       |
|                       | md>`__\ []            |                       |
+-----------------------+-----------------------+-----------------------+

..

   错误码如下表所示：

+---------------------------+--------+------------------------------------+
| 异常                      | 错误码 | 描述                               |
+===========================+========+====================================+
| INVALID_BLOCKNUMBER_ERROR | 11060  | BlockNumber must be greater than 0 |
+---------------------------+--------+------------------------------------+
| REQUEST_NULL_ERROR        | 12001  | Request parameter cannot be null   |
+---------------------------+--------+------------------------------------+
| CONNECTNETWORK_ERROR      | 11007  | Failed to connect to the network   |
+---------------------------+--------+------------------------------------+
| SYSTEM_ERROR              | 20000  | System error                       |
+---------------------------+--------+------------------------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   Long blockNumber = 617247L;// 第617247区块
   BlockGetTransactionsRequest request = new BlockGetTransactionsRequest();
   request.setBlockNumber(blockNumber);

   // 调用getTransactions接口
   BlockGetTransactionsResponse response = sdk.getBlockService().getTransactions(request);
   if(0 == response.getErrorCode()){
   System.out.println(JSON.toJSONString(response.getResult(), true));
   }else{
   System.out.println("error: " + response.getErrorDesc());
   }

.. _getinfo-4:

getInfo
~~~~~~~

getInfo接口用于获取区块信息。

   调用方法如下所示：

BlockGetInfoResponse getInfo(BlockGetInfoRequest);

   请求参数如下表所示：

+-------------+------+------------------------+
| 参数        | 类型 | 描述                   |
+=============+======+========================+
| blockNumber | Long | 必填，待查询的区块高度 |
+-------------+------+------------------------+

..

   响应数据如下表所示：

+-----------+--------+--------------+
| 参数      | 类型   | 描述         |
+===========+========+==============+
| closeTime | Long   | 区块关闭时间 |
+-----------+--------+--------------+
| number    | Long   | 区块高度     |
+-----------+--------+--------------+
| txCount   | Long   | 交易总量     |
+-----------+--------+--------------+
| version   | String | 区块版本     |
+-----------+--------+--------------+

..

   错误码如下表所示：

+---------------------------+--------+------------------------------------+
| 异常                      | 错误码 | 描述                               |
+===========================+========+====================================+
| INVALID_BLOCKNUMBER_ERROR | 11060  | BlockNumber must be greater than 0 |
+---------------------------+--------+------------------------------------+
| REQUEST_NULL_ERROR        | 12001  | Request parameter cannot be null   |
+---------------------------+--------+------------------------------------+
| CONNECTNETWORK_ERROR      | 11007  | Fail to connect network            |
+---------------------------+--------+------------------------------------+
| SYSTEM_ERROR              | 20000  | System error                       |
+---------------------------+--------+------------------------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   BlockGetInfoRequest request = new BlockGetInfoRequest();
   request.setBlockNumber(629743L);

   // 调用getInfo接口
   BlockGetInfoResponse response = sdk.getBlockService().getInfo(request);
   if (response.getErrorCode() == 0) {
   BlockGetInfoResult result = response.getResult();
   System.out.println(JSON.toJSONString(result, true));
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

getLatestInfo
~~~~~~~~~~~~~

getLatestInfo接口用于获取最新区块信息。

   调用方法如下所示：

BlockGetLatestInfoResponse getLatestInfo();

   响应数据如下表所示：

+-----------+--------+---------------------------+
| 参数      | 类型   | 描述                      |
+===========+========+===========================+
| closeTime | Long   | 区块关闭时间              |
+-----------+--------+---------------------------+
| number    | Long   | 区块高度，对应底层字段seq |
+-----------+--------+---------------------------+
| txCount   | Long   | 交易总量                  |
+-----------+--------+---------------------------+
| version   | String | 区块版本                  |
+-----------+--------+---------------------------+

..

   错误码如下表所示：

+----------------------+--------+----------------------------------+
| 异常                 | 错误码 | 描述                             |
+======================+========+==================================+
| CONNECTNETWORK_ERROR | 11007  | Failed to connect to the network |
+----------------------+--------+----------------------------------+
| SYSTEM_ERROR         | 20000  | System error                     |
+----------------------+--------+----------------------------------+

..

   具体示例如下所示：

::

   // 调用getLatestInfo接口
   BlockGetLatestInfoResponse response = sdk.getBlockService().getLatestInfo();
   if (response.getErrorCode() == 0) {
   BlockGetLatestInfoResult result = response.getResult();
   System.out.println(JSON.toJSONString(result, true));
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

getValidators
^^^^^^^^^^^^^

getValidators接口用于获取指定区块中所有验证节点数。

   调用方法如下所示：

BlockGetValidatorsResponse getValidators(BlockGetValidatorsRequest);

   请求参数如下表所示：

+-------------+------+-----------------------------------+
| 参数        | 类型 | 描述                              |
+=============+======+===================================+
| blockNumber | Long | 必填，待查询的区块高度，必须大于0 |
+-------------+------+-----------------------------------+

..

   响应数据如下表所示：

+-----------------------+-----------------------+-----------------------+
| 参数                  | 类型                  | 描述                  |
+=======================+=======================+=======================+
| validators            | `ValidatorInfo </qu-k | 验证节点列表          |
|                       | uai-fu-wu/getvalidato |                       |
|                       | rs/validatorinfo.md>` |                       |
|                       | __\ []                |                       |
+-----------------------+-----------------------+-----------------------+

..

   错误码如下表所示：

+---------------------------+--------+------------------------------------+
| 异常                      | 错误码 | 描述                               |
+===========================+========+====================================+
| INVALID_BLOCKNUMBER_ERROR | 11060  | BlockNumber must be greater than 0 |
+---------------------------+--------+------------------------------------+
| REQUEST_NULL_ERROR        | 12001  | Request parameter cannot be null   |
+---------------------------+--------+------------------------------------+
| CONNECTNETWORK_ERROR      | 11007  | Failed to connect to the network   |
+---------------------------+--------+------------------------------------+
| SYSTEM_ERROR              | 20000  | System error                       |
+---------------------------+--------+------------------------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   BlockGetValidatorsRequest request = new BlockGetValidatorsRequest();
   request.setBlockNumber(629743L);

   // 调用getValidators接口
   BlockGetValidatorsResponse response = sdk.getBlockService().getValidators(request);
   if (response.getErrorCode() == 0) {
   BlockGetValidatorsResult result = response.getResult();
   System.out.println(JSON.toJSONString(result, true));
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

ValidatorInfo
^^^^^^^^^^^^^

+-----------------+--------+--------------+
| 成员变量        | 类型   | 描述         |
+=================+========+==============+
| address         | String | 共识节点地址 |
+-----------------+--------+--------------+
| plegeCoinAmount | Long   | 验证节点押金 |
+-----------------+--------+--------------+

getLatestValidators
~~~~~~~~~~~~~~~~~~~

getLatestValidators接口用于获取最新区块中所有验证节点数。

   调用方法如下所示：

BlockGetLatestValidatorsResponse getLatestValidators();

   响应数据如下表所示：

+-----------------------+-----------------------+-----------------------+
| 参数                  | 类型                  | 描述                  |
+=======================+=======================+=======================+
| validators            | `ValidatorInfo </qu-k | 验证节点列表          |
|                       | uai-fu-wu/getvalidato |                       |
|                       | rs/validatorinfo.md>` |                       |
|                       | __\ []                |                       |
+-----------------------+-----------------------+-----------------------+

..

   错误码如下表所示：

+----------------------+--------+----------------------------------+
| 异常                 | 错误码 | 描述                             |
+======================+========+==================================+
| CONNECTNETWORK_ERROR | 11007  | Failed to connect to the network |
+----------------------+--------+----------------------------------+
| SYSTEM_ERROR         | 20000  | System error                     |
+----------------------+--------+----------------------------------+

..

   具体示例如下所示：

::

   // 调用getLatestValidators接口
   BlockGetLatestValidatorsResponse response = sdk.getBlockService().getLatestValidators();
   if (response.getErrorCode() == 0) {
   BlockGetLatestValidatorsResult result = response.getResult();
   System.out.println(JSON.toJSONString(result, true));
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

getReward
~~~~~~~~~

getReward接口用于获取指定区块中的区块奖励和验证节点奖励。

   调用方法如下所示：

BlockGetRewardResponse getReward(BlockGetRewardRequest);

   请求参数如下表所示：

+-------------+------+-----------------------------------+
| 参数        | 类型 | 描述                              |
+=============+======+===================================+
| blockNumber | Long | 必填，待查询的区块高度，必须大于0 |
+-------------+------+-----------------------------------+

..

   响应数据如下表所示：

+-----------------------+-----------------------+-----------------------+
| 参数                  | 类型                  | 描述                  |
+=======================+=======================+=======================+
| blockReward           | Long                  | 区块奖励数            |
+-----------------------+-----------------------+-----------------------+
| validatorsReward      | `ValidatorReward </qu | 验证节点奖励情况      |
|                       | -kuai-fu-wu/getreward |                       |
|                       | /validatorreward.md>` |                       |
|                       | __\ []                |                       |
+-----------------------+-----------------------+-----------------------+

..

   错误码如下表所示：

+---------------------------+--------+------------------------------------+
| 异常                      | 错误码 | 描述                               |
+===========================+========+====================================+
| INVALID_BLOCKNUMBER_ERROR | 11060  | BlockNumber must be greater than 0 |
+---------------------------+--------+------------------------------------+
| REQUEST_NULL_ERROR        | 12001  | Request parameter cannot be null   |
+---------------------------+--------+------------------------------------+
| CONNECTNETWORK_ERROR      | 11007  | Failed to connect to the network   |
+---------------------------+--------+------------------------------------+
| SYSTEM_ERROR              | 20000  | System error                       |
+---------------------------+--------+------------------------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   BlockGetRewardRequest request = new BlockGetRewardRequest();
   request.setBlockNumber(629743L);

   // 调用getReward接口
   BlockGetRewardResponse response = sdk.getBlockService().getReward(request);
   if (response.getErrorCode() == 0) {
   BlockGetRewardResult result = response.getResult();
   System.out.println(JSON.toJSONString(result, true));
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

ValidatorReward
^^^^^^^^^^^^^^^

ValidatorReward的具体信息如下表所示：

+-----------+--------+--------------+
| 成员变量  | 类型   | 描述         |
+===========+========+==============+
| validator | String | 验证节点地址 |
+-----------+--------+--------------+
| reward    | Long   | 验证节点奖励 |
+-----------+--------+--------------+

getLatestReward
~~~~~~~~~~~~~~~

getLatestReward接口获取最新区块中的区块奖励和验证节点奖励。

   调用方法如下所示：

BlockGetLatestRewardResponse getLatestReward();

   响应数据如下表所示：

+-----------------------+-----------------------+-----------------------+
| 参数                  | 类型                  | 描述                  |
+=======================+=======================+=======================+
| blockReward           | Long                  | 区块奖励数            |
+-----------------------+-----------------------+-----------------------+
| validatorsReward      | `ValidatorReward </qu | 验证节点奖励情况      |
|                       | -kuai-fu-wu/getreward |                       |
|                       | /validatorreward.md>` |                       |
|                       | __\ []                |                       |
+-----------------------+-----------------------+-----------------------+

..

   错误码如下表所示：

+----------------------+--------+----------------------------------+
| 异常                 | 错误码 | 描述                             |
+======================+========+==================================+
| CONNECTNETWORK_ERROR | 11007  | Failed to connect to the network |
+----------------------+--------+----------------------------------+
| SYSTEM_ERROR         | 20000  | System error                     |
+----------------------+--------+----------------------------------+

..

   具体示例如下所示：

::

   // 调用getLatestReward接口
   BlockGetLatestRewardResponse response = sdk.getBlockService().getLatestReward();
   if (response.getErrorCode() == 0) {
   BlockGetLatestRewardResult result = response.getResult();
   System.out.println(JSON.toJSONString(result, true));
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

getFees
~~~~~~~

getFees接口获取指定区块中的账户最低资产限制和燃料单价。

   调用方法如下所示：

BlockGetFeesResponse getFees(BlockGetFeesRequest);

   请求参数如下表所示：

+-------------+------+-----------------------------------+
| 参数        | 类型 | 描述                              |
+=============+======+===================================+
| blockNumber | Long | 必填，待查询的区块高度，必须大于0 |
+-------------+------+-----------------------------------+

..

   响应数据如下表所示：

+------+-------------------------------------------+------+
| 参数 | 类型                                      | 描述 |
+======+===========================================+======+
| fees | `Fees </qu-kuai-fu-wu/getfees/fees.md>`__ | 费用 |
+------+-------------------------------------------+------+

..

   错误码如下表所示：

+---------------------------+--------+------------------------------------+
| 异常                      | 错误码 | 描述                               |
+===========================+========+====================================+
| INVALID_BLOCKNUMBER_ERROR | 11060  | BlockNumber must be greater than 0 |
+---------------------------+--------+------------------------------------+
| REQUEST_NULL_ERROR        | 12001  | Request parameter cannot be null   |
+---------------------------+--------+------------------------------------+
| CONNECTNETWORK_ERROR      | 11007  | Failed to connect to the network   |
+---------------------------+--------+------------------------------------+
| SYSTEM_ERROR              | 20000  | System error                       |
+---------------------------+--------+------------------------------------+

..

   具体示例如下所示：

::

   // 初始化请求参数
   BlockGetFeesRequest request = new BlockGetFeesRequest();
   request.setBlockNumber(629743L);

   // 调用getFees接口
   BlockGetFeesResponse response = sdk.getBlockService().getFees(request);
   if (response.getErrorCode() == 0) {
   System.out.println(JSON.toJSONString(response.getResult(), true));
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

Fees
^^^^

Fees的具体信息如下表所示：

+-------------+------+--------------------------------------+
| 成员变量    | 类型 | 描述                                 |
+=============+======+======================================+
| baseReserve | Long | 账户最低资产限制                     |
+-------------+------+--------------------------------------+
| gasPrice    | Long | 交易燃料单价，单位MO，1 BU = 10^8 MO |
+-------------+------+--------------------------------------+

getLatestFees
~~~~~~~~~~~~~

getLatestFees接口用于获取最新区块中的账户最低资产限制和燃料单价。

   调用方法如下所示：

BlockGetLatestFeesResponse getLatestFees();

   响应数据如下表所示：

+------+-------------------------------------------+------+
| 参数 | 类型                                      | 描述 |
+======+===========================================+======+
| fees | `Fees </qu-kuai-fu-wu/getfees/fees.md>`__ | 费用 |
+------+-------------------------------------------+------+

..

   错误码如下表所示：

+----------------------+--------+----------------------------------+
| 异常                 | 错误码 | 描述                             |
+======================+========+==================================+
| CONNECTNETWORK_ERROR | 11007  | Failed to connect to the network |
+----------------------+--------+----------------------------------+
| SYSTEM_ERROR         | 20000  | System error                     |
+----------------------+--------+----------------------------------+

..

   具体示例如下所示：

::

   // 调用getLatestFees接口
   BlockGetLatestFeesResponse response = sdk.getBlockService().getLatestFees();
   if (response.getErrorCode() == 0) {
   System.out.println(JSON.toJSONString(response.getResult(), true));
   } else {
   System.out.println("error: " + response.getErrorDesc());
   }

错误码
------

下表对可能出现的错误信息进行了详细描述。

+-----------------------+-----------------------+-----------------------+
| 异常                  | 错误码                | 描述                  |
+=======================+=======================+=======================+
| ACCOUNT_CREATE_ERROR  | 11001                 | Failed to create the  |
|                       |                       | account               |
+-----------------------+-----------------------+-----------------------+
| INVALID_SOURCEADDRESS | 11002                 | Invalid sourceAddress |
| _ERROR                |                       |                       |
+-----------------------+-----------------------+-----------------------+
| INVALID_DESTADDRESS_E | 11003                 | Invalid destAddress   |
| RROR                  |                       |                       |
+-----------------------+-----------------------+-----------------------+
| INVALID_INITBALANCE_E | 11004                 | InitBalance must be   |
| RROR                  |                       | between 1 and         |
|                       |                       | Long.MAX_VALUE        |
+-----------------------+-----------------------+-----------------------+
| SOURCEADDRESS_EQUAL_D | 11005                 | SourceAddress cannot  |
| ESTADDRESS_ERROR      |                       | be equal to           |
|                       |                       | destAddress           |
+-----------------------+-----------------------+-----------------------+
| INVALID_ADDRESS_ERROR | 11006                 | Invalid address       |
+-----------------------+-----------------------+-----------------------+
| CONNECTNETWORK_ERROR  | 11007                 | Failed to connect to  |
|                       |                       | the network           |
+-----------------------+-----------------------+-----------------------+
| INVALID_ISSUE_AMOUNT_ | 11008                 | Amount of the token   |
| ERROR                 |                       | to be issued must be  |
|                       |                       | between 1 and         |
|                       |                       | Long.MAX_VALUE        |
+-----------------------+-----------------------+-----------------------+
| NO_ASSET_ERROR        | 11009                 | The account does not  |
|                       |                       | have the asset        |
+-----------------------+-----------------------+-----------------------+
| NO_METADATA_ERROR     | 11010                 | The account does not  |
|                       |                       | have the metadata     |
+-----------------------+-----------------------+-----------------------+
| INVALID_DATAKEY_ERROR | 11011                 | The length of key     |
|                       |                       | must be between 1 and |
|                       |                       | 1024                  |
+-----------------------+-----------------------+-----------------------+
| INVALID_DATAVALUE_ERR | 11012                 | The length of value   |
| OR                    |                       | must be between 0 and |
|                       |                       | 256000                |
+-----------------------+-----------------------+-----------------------+
| INVALID_DATAVERSION_E | 11013                 | The version must be   |
| RROR                  |                       | equal to or greater   |
|                       |                       | than 0                |
+-----------------------+-----------------------+-----------------------+
| INVALID_MASTERWEIGHT_ | 11015                 | MasterWeight must be  |
| ERROR                 |                       | between 0 and         |
|                       |                       | (Integer.MAX_VALUE \* |
|                       |                       | 2L + 1)               |
+-----------------------+-----------------------+-----------------------+
| INVALID_SIGNER_ADDRES | 11016                 | Invalid signer        |
| S_ERROR               |                       | address               |
+-----------------------+-----------------------+-----------------------+
| INVALID_SIGNER_WEIGHT | 11017                 | Signer weight must be |
| _ERROR                |                       | between 0 and         |
|                       |                       | (Integer.MAX_VALUE \* |
|                       |                       | 2L + 1)               |
+-----------------------+-----------------------+-----------------------+
| INVALID_TX_THRESHOLD_ | 11018                 | TxThreshold must be   |
| ERROR                 |                       | between 0 and         |
|                       |                       | Long.MAX_VALUE        |
+-----------------------+-----------------------+-----------------------+
| INVALID_OPERATION_TYP | 11019                 | Operation type must   |
| E_ERROR               |                       | be between 1 and 100  |
+-----------------------+-----------------------+-----------------------+
| INVALID_TYPE_THRESHOL | 11020                 | TypeThreshold must be |
| D_ERROR               |                       | between 0 and         |
|                       |                       | Long.MAX_VALUE        |
+-----------------------+-----------------------+-----------------------+
| INVALID_ASSET_CODE_ER | 11023                 | The length of key     |
| ROR                   |                       | must be between 1 and |
|                       |                       | 64                    |
+-----------------------+-----------------------+-----------------------+
| INVALID_ASSET_AMOUNT_ | 11024                 | AssetAmount must be   |
| ERROR                 |                       | between 0 and         |
|                       |                       | Long.MAX_VALUE        |
+-----------------------+-----------------------+-----------------------+
| INVALID_BU_AMOUNT_ERR | 11026                 | BuAmount must be      |
| OR                    |                       | between 0 and         |
|                       |                       | Long.MAX_VALUE        |
+-----------------------+-----------------------+-----------------------+
| INVALID_ISSUER_ADDRES | 11027                 | Invalid issuer        |
| S_ERROR               |                       | address               |
+-----------------------+-----------------------+-----------------------+
| NO_SUCH_TOKEN_ERROR   | 11030                 | No such token         |
+-----------------------+-----------------------+-----------------------+
| INVALID_TOKEN_NAME_ER | 11031                 | The length of token   |
| ROR                   |                       | name must be between  |
|                       |                       | 1 and 1024            |
+-----------------------+-----------------------+-----------------------+
| INVALID_TOKEN_SIMBOL_ | 11032                 | The length of symbol  |
| ERROR                 |                       | must be between 1 and |
|                       |                       | 1024                  |
+-----------------------+-----------------------+-----------------------+
| INVALID_TOKEN_DECIMAL | 11033                 | Decimals must be      |
| S_ERROR               |                       | between 0 and 8       |
+-----------------------+-----------------------+-----------------------+
| INVALID_TOKEN_TOTALSU | 11034                 | TotalSupply must be   |
| PPLY_ERROR            |                       | between 1 and         |
|                       |                       | Long.MAX_VALUE        |
+-----------------------+-----------------------+-----------------------+
| INVALID_TOKENOWNER_ER | 11035                 | Invalid token owner   |
| RPR                   |                       |                       |
+-----------------------+-----------------------+-----------------------+
| INVALID_CONTRACTADDRE | 11037                 | Invalid contract      |
| SS_ERROR              |                       | address               |
+-----------------------+-----------------------+-----------------------+
| CONTRACTADDRESS_NOT_C | 11038                 | contractAddress is    |
| ONTRACTACCOUNT_ERROR  |                       | not a contract        |
|                       |                       | account               |
+-----------------------+-----------------------+-----------------------+
| INVALID_TOKEN_AMOUNT_ | 11039                 | TokenAmount must be   |
| ERROR                 |                       | between 1 and         |
|                       |                       | Long.MAX_VALUE        |
+-----------------------+-----------------------+-----------------------+
| SOURCEADDRESS_EQUAL_C | 11040                 | SourceAddress cannot  |
| ONTRACTADDRESS_ERROR  |                       | be equal to           |
|                       |                       | contractAddress       |
+-----------------------+-----------------------+-----------------------+
| INVALID_FROMADDRESS_E | 11041                 | Invalid fromAddress   |
| RROR                  |                       |                       |
+-----------------------+-----------------------+-----------------------+
| FROMADDRESS_EQUAL_DES | 11042                 | FromAddress cannot be |
| TADDRESS_ERROR        |                       | equal to destAddress  |
+-----------------------+-----------------------+-----------------------+
| INVALID_SPENDER_ERROR | 11043                 | Invalid spender       |
+-----------------------+-----------------------+-----------------------+
| PAYLOAD_EMPTY_ERROR   | 11044                 | Payload cannot be     |
|                       |                       | empty                 |
+-----------------------+-----------------------+-----------------------+
| INVALID_LOG_TOPIC_ERR | 11045                 | The length of one log |
| OR                    |                       | topic must be between |
|                       |                       | 1 and 128             |
+-----------------------+-----------------------+-----------------------+
| INVALID_LOG_DATA_ERRO | 11046                 | The length of one     |
| R                     |                       | piece of log data     |
|                       |                       | must be between 1 and |
|                       |                       | 1024                  |
+-----------------------+-----------------------+-----------------------+
| INVALID_CONTRACT_TYPE | 11047                 | Invalid contract type |
| _ERROR                |                       |                       |
+-----------------------+-----------------------+-----------------------+
| INVALID_NONCE_ERROR   | 11048                 | Nonce must be between |
|                       |                       | 1 and Long.MAX_VALUE  |
+-----------------------+-----------------------+-----------------------+
| INVALID_GASPRICE_ERRO | 11049                 | GasPrice must be      |
| R                     |                       | between 1000 and      |
|                       |                       | Long.MAX_VALUE        |
+-----------------------+-----------------------+-----------------------+
| INVALID_FEELIMIT_ERRO | 11050                 | FeeLimit must be      |
| R                     |                       | between 1 and         |
|                       |                       | Long.MAX_VALUE        |
+-----------------------+-----------------------+-----------------------+
| OPERATIONS_EMPTY_ERRO | 11051                 | Operations cannot be  |
| R                     |                       | empty                 |
+-----------------------+-----------------------+-----------------------+
| INVALID_CEILLEDGERSEQ | 11052                 | CeilLedgerSeq must be |
| _ERROR                |                       | equal to or greater   |
|                       |                       | than 0                |
+-----------------------+-----------------------+-----------------------+
| OPERATIONS_ONE_ERROR  | 11053                 | One of the operations |
|                       |                       | cannot be resolved    |
+-----------------------+-----------------------+-----------------------+
| INVALID_SIGNATURENUMB | 11054                 | SignagureNumber must  |
| ER_ERROR              |                       | be between 1 and      |
|                       |                       | Integer.MAX_VALUE     |
+-----------------------+-----------------------+-----------------------+
| INVALID_HASH_ERROR    | 11055                 | Invalid transaction   |
|                       |                       | hash                  |
+-----------------------+-----------------------+-----------------------+
| INVALID_BLOB_ERROR    | 11056                 | Invalid blob          |
+-----------------------+-----------------------+-----------------------+
| PRIVATEKEY_NULL_ERROR | 11057                 | PrivateKeys cannot be |
|                       |                       | empty                 |
+-----------------------+-----------------------+-----------------------+
| PRIVATEKEY_ONE_ERROR  | 11058                 | One of privateKeys is |
|                       |                       | invalid               |
+-----------------------+-----------------------+-----------------------+
| PUBLICKEY_NULL_ERROR  | 11061                 | PublicKey cannot be   |
|                       |                       | empty                 |
+-----------------------+-----------------------+-----------------------+
| URL_EMPTY_ERROR       | 11062                 | Url cannot be empty   |
+-----------------------+-----------------------+-----------------------+
| CONTRACTADDRESS_CODE_ | 11063                 | ContractAddress and   |
| BOTH_NULL_ERROR       |                       | code cannot be empty  |
|                       |                       | at the same time      |
+-----------------------+-----------------------+-----------------------+
| INVALID_OPTTYPE_ERROR | 11064                 | OptType must be       |
|                       |                       | between 0 and 2       |
+-----------------------+-----------------------+-----------------------+
| GET_ALLOWANCE_ERROR   | 11065                 | Failed to get         |
|                       |                       | allowance             |
+-----------------------+-----------------------+-----------------------+
| GET_TOKEN_INFO_ERROR  | 11066                 | Failed to get the     |
|                       |                       | token info            |
+-----------------------+-----------------------+-----------------------+
| SIGNATURE_EMPTY_ERROR | 11067                 | The signatures cannot |
|                       |                       | be empty              |
+-----------------------+-----------------------+-----------------------+
| REQUEST_NULL_ERROR    | 12001                 | Request parameter     |
|                       |                       | cannot be null        |
+-----------------------+-----------------------+-----------------------+
| CONNECTN_BLOCKCHAIN_E | 19999                 | Failed to connect to  |
| RROR                  |                       | the blockchain        |
+-----------------------+-----------------------+-----------------------+
| SYSTEM_ERROR          | 20000                 | System error          |
+-----------------------+-----------------------+-----------------------+
