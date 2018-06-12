English | [中文](develop_CN.md) 

# BUMO Blockchain Development Kit

<!-- TOC -->

- [BUMO Blockchain Development Kit](#BUMO区块链开发文档)
    - [Basic Knowledge](#basic-knowledge)
        - [What is Protocol Buffer3](#what-is-protocol-buffer3)
        - [Protocol Buffer3 and JSON](#protocol-buffer3-and-json)
        - [Websocket and http](#websocket-and-http)
        - [Port Configuration](#port-configuration)
        - [Perform Transaction](#perform-transaction)
        - [Give it a Try](#give-it-a-try) 
    - [HTTP api](#http-api)
        - [Creating Test Key Pair](#creating-test-key-pair)
        - [Querying Account](#querying-account) 
        - [Querying Basic Information of Account](#querying-basic-information-of-account)
        - [Querying Transaction](#querying-transaction)
        - [Querying Transaction in Buffer-queue](#querying-transactions-in-buffer-queue)
        - [Querying Block Header](#querying-block-header)
        - [Submitting Transaction](#submitting-transaction)
        - [Serializing Transaction](#serializing-transaction)
        - [Debugging Smart Contract](#debugging-smart-contract)
        - [Evaluating Transaction Fee](#evaluating-transaction-fee)
    - [Transaction Definition](#transaction-definition)
        - [Basic Structure of Transaction](#basic-structure-of-transactions)
        - [Operation](#operation)
            - [Operating Code](#operating-code)
            - [Creating Account](#creating-account)
            - [Issuing Assets](#issuing-assets) 
            - [Transferring Assets](#transferring-assets) 
            - [Setting Metadata](#setting-metadata)
            - [Setting Privilege](#setting-privilege) 
            - [Transferring BU Assets](#transferring-bu-assets)
            - [Log](#log)
    - [Advanced Features](#advanced-features)
        - [Distributing Weight](#distributing-weight) 
        - [Controlling Version](#controlling-version)
        - [Smart Contract](#smart-contract)
            - [Syntax](#syntax)
            - [Default Functions](#default-functions)
            - [Default Variables](#default-variables)
            - [Exceptions](#exceptions)
        - [Validator Nodes Election](#validator-nodes-election)
            - [Creating Election Contract Account](#creating-election-contract-account)
            - [Candidates of Validator Nodes](#candidates-of-validator-nodes)
            - [Voting on Validator Nodes Appliers](#voting-on-validator-nodes-appliers) 
            - [Forfeiting the Pledge](#forfeit-the-pledge) 
            - [Banning Malicious Nodes](#banning-malicious-nodes) 
            - [Revoking banning Malicious Nodes](#revoking-banning-malicious-nodes)
            - [Voting on Banning Malicious Nodes](#voting-on-banning-malicious-nodes) 
            - [Querying Functions](#querying-functions) 
        - [Election Contract of Transaction Fee](#election-contract-of-transaction-fee)
            - [Structure of Transaction Fee](#structure-of-transaction-fee)
            - [Types of Transaction Fee](#types-of-transaction-fee) 
            - [Querying Transaction Fee History](#querying-transaction-fee-history) 
            - [Querying Proposals of Transaction Fee](#querying-proposals-of-transaction-fee)
            - [Proposals of Transaction Fee](#proposals-of-transaction-fee)
            - [Selectinging Transaction Fee](#selecting-transaction-fee)
    - [Error Code](#error-code)
    - [Example](#example)

<!-- /TOC -->

## Basic Knowledge

### What is Protocol Buffer3

BUMO Blockchain serializes data with `protocol buffer 3`, which is a general serialization protocol launched by Google. Click this [link](https://developers.google.com/protocol-buffers/docs/proto3) to get more information. All the data format we use are under the dir: `src\proto`. Other data with reference to transaction, block ,account are in the `chain.proto` file. 

### Protocol Buffer3 and JSON

Data from http api are in json format, and are transferred automatically by protocol buffer. The text defined as type **bytes** is hexadecimal in json. It can not be utilized directly.  

### Websocket and http

BUMO Blockchain offers two types of api: **websocket** and **http**. These two objects assign the service port, and they can be found in the downloaded dir:`/config/bumo.json`. 

```json
    "webserver":
    {
        "listen_addresses": "0.0.0.0:36002"
    },
    "wsserver":
    {
        "listen_address": "0.0.0.0:36003"
    }
```

### Port Configuration

| network type        | network ID（network_id）|WebServer |P2P  | WebSocket |
| :------------- | -------|--------- |-----|-----------|
| mainnet        | 10000|16002 |16001 |16003
| testnet | 20000|26002 | 26001 | 26003
| beta version | 30000 | 36002 | 36001 | 36003 


### Perform Transaction 

1. Fill in the transaction  → `Transaction`
2. Serializing the transaction (protocol buffer 3) to bytes stream   →  `transaction_blob`
3. Signing the `transaction_blob` with private key `skey`, and get the `sign_data`. (Public key is `pkey`)
4. Submitting transaction. (Refer to [Submitting Transactions](#submit-transactions))
5. Querying or subscribe the notification (from websocket API) to confirm the transaction status.

### Give it a Try

If you have just deployed the blockchain,  at the moment there will only be an initial account on the blockchain. You can send the query `HTTP GET host:36002/getGenesisAccount`to http api, then, you will obtain a response as follow: 

```json
{
   "error_code" : 0,
   "result" : {
      "address" : "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3",
      "assets" : null,
      "balance" : 100000000000000000,
      "metadatas" : null,
      "priv" : {
         "master_weight" : 1,
         "thresholds" : {
            "tx_threshold" : 1
         }
      }
   }
}
```

The value of address from the response/return is the initial account.



You can also query other accounts' information from **Querying Account api**.

```text
HTTP GET host:36002/getAccount?address=buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3
```

## HTTP api

### Creating Test Key Pair 

```text
HTTP GET /createKeyPair
```

Function: This api is only for testing, please DO NOT use this api in the production environment (substitute SDK or Command for api). If the server is malicious, using the api will risk you private key in exposure. This api only create a new public-private key pair which does not write into bumo chain.If you want to write, you can do any transaction of [Creating Account](#creating-account), [Transferring Assets](#transferring-assets) ,[Transferring BU Assets](#transferring-bu-assets) for the new key pair.

return,

```json
{
  "error_code" : 0,
  "result" : {
          "address": "buQd4TBqSbHw3EoLMnSmH4SJFMkHUtEQbUvz",        //address of the account
          "private_key": "privbzYwbUSCwQZq7eXgu4C9cpqrQD4enXY49V7qUrifc6fCtiPmBhWA",  //private key
          "public_key": "b0016558bd75fe20d6f7953cef0a95509d11d73652b70f183c72ede25711778dfc0039ea73f3",  //public key
          "public_key_raw": "6558bd75fe20d6f7953cef0a95509d11d73652b70f183c72ede25711778dfc00"       //public key without prefixes and suffixes
  }
}

```

### Querying Account

```text
HTTP GET /getAccount?address=buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3&key=hello&code=xxx&issuer=xxx
```

Function: Returning the information, assets and metadata of specific account.  

| Parameter           | Description                                                  |
| :------------------ | ------------------------------------------------------------ |
| address*(required)* | Address of the account.                                      |
| key                 | Appointed key value of the metadata in the account. Returns will contain complete information of metadata as the key is null. |
| code, issuer,type   | Assets code and Assets issuer. The parameters should be specified in pairs. Returns will contain complete information of assets as the code-issuer pair is null. *The `type` defaults to 0 currently, you do not need to change it.* |

return,

```json
{
  "error_code" : 0,
  "result" : {
    "address" : "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3", //address of the account
    "assets" : [//all assets of the account
      {
        "amount" : 1400,
        "key" :
        {
          "code" : "CNY",
          "issuer" : "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3"
        }
      }, {
        "amount" : 1000,
        "key" :
        {
          "code" : "USD",
          "issuer" : "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3"
        }
      }
    ],
    "assets_hash" : "9696b03e4c3169380882e0217a986717adfc5877b495068152e6aa25370ecf4a",
    "contract" : null,
    "metadatas" : [//all the metadata of the account
      {
        "key" : "123",
        "value" : "123_value",
        "version" : 1
      }, {
        "key" : "456",
        "value" : "456_value",
        "version" : 1
      }, {
        "key" : "abcd",
        "value" : "abcd_value",
        "version" : 1
      }
    ],
    "nonce" : 1, //Count of the transactions from the source account. It would be concealed if the nonce is 0.
    "priv" : {
      "master_weight" : 1,
      "thresholds" : {
        "tx_threshold" : 1
      }
    },
    "metadatas_hash" : "82c8407cc7cd77897be3100c47ed9d43ec4097ee1c00e2c13447187e5b1ac66c"
  }
}

```

- If the account does not exist, then return, 

```json
{
   "error_code" : 4,
   "result" : null
}
```

### Querying Basic Information of Account

```text
HTTP GET /getAccountBase?address=buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3
```

 - return,

| Parameter         | Description            |
| :---------------- | ---------------------- |
| address(required) | Address of the account |

return,

```json
{
  "error_code" : 0,
  "result" : {
    "address" : "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3", //Address of the account
    "assets_hash" : "9696b03e4c3169380882e0217a986717adfc5877b495068152e6aa25370ecf4a",
    "balance" : 899671600,
    "contract" : null,
    "address of the accountnonce" : 1, // Count of the transactions from the source account. It would be concealed if the nonce is 0.
    "priv" : {
      "master_weight" : 1,
      "thresholds" : {
        "tx_threshold" : 1
      }
    },
    "metadatas_hash" : "82c8407cc7cd77897be3100c47ed9d43ec4097ee1c00e2c13447187e5b1ac66c"
  }
}

```

- If the account does not exist, then return, 

```json
{
   "error_code" : 4,
   "result" : null
}
```

### Querying Transaction

```text
GET /getTransactionHistory?ledger_seq=6
```

| Parameters | Description                                                  |
| :--------- | ------------------------------------------------------------ |
| hash       | Querying transactions with the unique identification by hash value. |
| ledger_seq | Querying transactions with specified block.                  |

Note: If you appoint the two parameters at the same time, api will return the specified transaction in the specified block. 

return,

```json
{
    "error_code": 0,
    "result": {
        "total_count": 2,
        "transactions": [{
            "actual_fee": 313000,
            "close_time": 1524031260097214,
            "error_code": 0,
            "error_desc": "",
            "hash": "89a9d6e5d2c0e2b5c4fe58045ab2236d12e9449ef232342a48a2e2628e12014d",
            "ledger_seq": 6,
            "signatures": [{
                "public_key": "b00180c2007082d1e2519a0f2d08fd65ba607fe3b8be646192a2f18a5fa0bee8f7a810d011ed",
                "sign_data": "27866d70a58fc527b1ff1b4a693b8034b0078fc7ac7591fb05679abe5ca660db5c372922bfa8f26e76511e2c33386306ded7593874a6aec5baeeaddbd2012f06"
            }],
            "transaction": {
                "fee_limit": 10000000000,
                "gas_price": 1000,
                "nonce": 1,
                "operations": [{
                    "create_account": {
                        "dest_address": "buQBAfoMfXZVPpg9DaabMmM2EwUnfoVsTSVV",
                        "init_balance": 10000000,
                        "priv": {
                            "master_weight": 1,
                            "thresholds": {
                                "tx_threshold": 1
                            }
                        }
                    },
                    "type": 1
                },
                {
                    "create_account": {
                        "dest_address": "buQj8UyKbN3myVSerLDVXVXH47vWXfqtxKLm",
                        "init_balance": 10000000,
                        "priv": {
                            "master_weight": 1,
                            "thresholds": {
                                "tx_threshold": 1
                            }
                        }
                    },
                    "type": 1
                }],
                "source_address": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3"
            },
            "tx_size": 313
        },
        {
            "actual_fee": 313000,
            "close_time": 1524031260097214,
            "error_code": 0,
            "error_desc": "",
            "hash": "7d7cfc860acfd83865d01bb655ac772113833f0769264d6392c6220024b8f93f",
            "ledger_seq": 6,
            "signatures": [{
                "public_key": "b00180c2007082d1e2519a0f2d08fd65ba607fe3b8be646192a2f18a5fa0bee8f7a810d011ed",
                "sign_data": "fb7d9d87f4c9140b6e19a199091c6871e2380ad8e8a8fcada9b42a2911057111dc796d731f3f887e600aa89cc8692300f980723298a93b91db711155670d3e0d"
            }],
            "transaction": {
                "fee_limit": 10000000000,
                "gas_price": 1000,
                "nonce": 2,
                "operations": [{
                    "create_account": {
                        "dest_address": "buQntAvayDWkAhPh6CSrTWbiEniAL2ys5m2p",
                        "init_balance": 10000000,
                        "priv": {
                            "master_weight": 1,
                            "thresholds": {
                                "tx_threshold": 1
                            }
                        }
                    },
                    "type": 1
                },
                {
                    "create_account": {
                        "dest_address": "buQX5X9y59zbmqyFgFPQPcyUPcPnvwsLatsq",
                        "init_balance": 10000000,
                        "priv": {
                            "master_weight": 1,
                            "thresholds": {
                                "tx_threshold": 1
                            }
                        }
                    },
                    "type": 1
                }],
                "source_address": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3"
            },
            "tx_size": 313
        }]
    }
}
```

If the account does not exist, then return, 

```json
{
  "error_code": 4,
  "result":
  {
    "total_count": 0,
    "transactions": []
  }
}
```

### Querying Transactions in Buffer-queue

```text
GET /getTransactionCache?hash=ad545bfc26c440e324076fbbe1d8affbd8a2277858dc35927d425d0fe644e698&limit=100
```

| Parameters | Description                                                  |
| :--------- | ------------------------------------------------------------ |
| hash       | Querying transactions with the unique identification by hash value. |
| limit      | Querying the first N transactions in the buffer queue.       |

Note: If you appoint the two parameters at the same time, api will return the query by hash value. 

return,
```json
{
    "error_code": 0,
    "result": {
        "total_count": 1,
        "transactions": [
            {
                "hash": "a336c8f4b49c8b2c5a6c68543368ed3b450b6138a9f878892cf982ffb6fe234e",
                "incoming_time": 1521013029435154,
                "signatures": [
                    {
                        "public_key": "b001882b9d1b5e7019f163d001c85194cface61e294483710f5e66ef40a4d387f5fcb0166f4f",
                        "sign_data": "c5885144ffccb0b434b494271258e846c30a4551036e483822ee2b57400576e9e700e8960eb424764d033a2e73af6e6a2bfa5da390f71161732e13beee206107"
                    }
                ],
                "status": "processing",
                "transaction": {
                    "fee_limit": 100000,
                    "gas_price": 1000,
                    "nonce": 2,
                    "operations": [
                        {
                            "create_account": {
                                "dest_address": "buQWufKdVicxRAqmQs6m1Z9QuFZG2W7LMsi2",
                                "init_balance": 300000,
                                "metadatas": [
                                    {
                                        "key": "key",
                                        "value": "bubinuo"
                                    }
                                ],
                                "priv": {
                                    "master_weight": 1,
                                    "thresholds": {
                                        "tx_threshold": 2
                                    }
                                }
                            },
                            "type": 1
                        }
                    ],
                    "source_address": "buQBDf23WtBBC8GySAZHsoBMVGeENWzSRYqB"
                }
            }
        ]
    }
}
```

If the account does not exist, then return, 

```json
{
  "error_code": 4,
  "result":
  {
    "total_count": 0,
    "transactions": []
  }
}
```

### Querying Block Header

```text
GET /getLedger?seq=xxxx&with_validator=true&with_consvalue=true&with_fee=true&with_block_reward=true
```

| Parameters        | Description                                                  |
| :---------------- | ------------------------------------------------------------ |
| seq               | Sequence of the ledger. Api will return the current ledger as the`seq`is null. |
| with_validator    | Defaults to `false`. Concealing the verifier nodes list.     |
| with_consvalue    | Defaults to `false`. Concealing the consensus value.         |
| with_fee          | Defaults to `false`. Concealing the fee configuration.       |
| with_block_reward | Defaults to `false`. Concealing the block reward and verifier reward. |


- If the ledger exists, then return,

```json
{
   "error_code" : 0,
   "result" : {
      "block_reward" : 800000000,
      "consensus_value" : {
         "close_time" : 1524031260097214,
         "ledger_seq" : 6,
         "previous_ledger_hash" : "ef329c7ed761e3065ab08f9e7672fd5f4e3ddd77b0be35598979aff8c21ada9b",
         "previous_proof" : "0ac1010a2c080110022a26080310052220432dde2fd32a2a66da77647231821c87958f56c303bd08003633952d384eb0b61290010a4c623030316435363833363735303137666662633332366538666232303738653532316566383435373234363236353339356536383934633835323434656566643262666130386635393862661240deeb9b782410f0f86d897006cac8ad152e56e4f914e5d718706de84044ef98baef25512a337865772641d57090b5c77e9e2149dbd41910e8d6cd85c3387ea708",
         "previous_proof_plain" : {
            "commits" : [
               {
                  "pbft" : {
                     "commit" : {
                        "sequence" : 5,
                        "value_digest" : "432dde2fd32a2a66da77647231821c87958f56c303bd08003633952d384eb0b6",
                        "view_number" : 3
                     },
                     "round_number" : 1,
                     "type" : 2
                  },
                  "signature" : {
                     "public_key" : "b001d5683675017ffbc326e8fb2078e521ef8457246265395e6894c85244eefd2bfa08f598bf",
                     "sign_data" : "deeb9b782410f0f86d897006cac8ad152e56e4f914e5d718706de84044ef98baef25512a337865772641d57090b5c77e9e2149dbd41910e8d6cd85c3387ea708"
                  }
               }
            ]
         },
         "txset" : {
            "txs" : [
               {
                  "signatures" : [
                     {
                        "public_key" : "b00180c2007082d1e2519a0f2d08fd65ba607fe3b8be646192a2f18a5fa0bee8f7a810d011ed",
                        "sign_data" : "27866d70a58fc527b1ff1b4a693b8034b0078fc7ac7591fb05679abe5ca660db5c372922bfa8f26e76511e2c33386306ded7593874a6aec5baeeaddbd2012f06"
                     }
                  ],
                  "transaction" : {
                     "fee_limit" : 10000000000,
                     "gas_price" : 1000,
                     "nonce" : 1,
                     "operations" : [
                        {
                           "create_account" : {
                              "dest_address" : "buQBAfoMfXZVPpg9DaabMmM2EwUnfoVsTSVV",
                              "init_balance" : 10000000,
                              "priv" : {
                                 "master_weight" : 1,
                                 "thresholds" : {
                                    "tx_threshold" : 1
                                 }
                              }
                           },
                           "type" : 1
                        },
                        {
                           "create_account" : {
                              "dest_address" : "buQj8UyKbN3myVSerLDVXVXH47vWXfqtxKLm",
                              "init_balance" : 10000000,
                              "priv" : {
                                 "master_weight" : 1,
                                 "thresholds" : {
                                    "tx_threshold" : 1
                                 }
                              }
                           },
                           "type" : 1
                        }
                     ],
                     "source_address" : "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3"
                  }
               },
               {
                  "signatures" : [
                     {
                        "public_key" : "b00180c2007082d1e2519a0f2d08fd65ba607fe3b8be646192a2f18a5fa0bee8f7a810d011ed",
                        "sign_data" : "fb7d9d87f4c9140b6e19a199091c6871e2380ad8e8a8fcada9b42a2911057111dc796d731f3f887e600aa89cc8692300f980723298a93b91db711155670d3e0d"
                     }
                  ],
                  "transaction" : {
                     "fee_limit" : 10000000000,
                     "gas_price" : 1000,
                     "nonce" : 2,
                     "operations" : [
                        {
                           "create_account" : {
                              "dest_address" : "buQntAvayDWkAhPh6CSrTWbiEniAL2ys5m2p",
                              "init_balance" : 10000000,
                              "priv" : {
                                 "master_weight" : 1,
                                 "thresholds" : {
                                    "tx_threshold" : 1
                                 }
                              }
                           },
                           "type" : 1
                        },
                        {
                           "create_account" : {
                              "dest_address" : "buQX5X9y59zbmqyFgFPQPcyUPcPnvwsLatsq",
                              "init_balance" : 10000000,
                              "priv" : {
                                 "master_weight" : 1,
                                 "thresholds" : {
                                    "tx_threshold" : 1
                                 }
                              }
                           },
                           "type" : 1
                        }
                     ],
                     "source_address" : "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3"
                  }
               }
            ]
         }
      },
      "fees" : {
         "base_reserve" : 10000000,
         "gas_price" : 1000
      },
      "header" : {
         "account_tree_hash" : "6aca37dfe83f213942b21d02618b989619cfd7c0e67a8a14b0f7599dd4010aad",
         "close_time" : 1524031260097214,
         "consensus_value_hash" : "14a65d69f619395135da2ff98281d5707494801f12184a4318b9a76383e651a8",
         "fees_hash" : "916daa78d264b3e2d9cff8aac84c943a834f49a62b7354d4fa228dab65515313",
         "hash" : "2cf378b326ab0026625c8d036813aef89a0b383e75055b80cb7cc25a657a9c5d",
         "previous_hash" : "ef329c7ed761e3065ab08f9e7672fd5f4e3ddd77b0be35598979aff8c21ada9b",
         "seq" : 6,
         "tx_count" : 2,
         "validators_hash" : "d857aa40ecdb123415f893159321eb223e4dbc11863daef86f35565dd1633316",
         "version" : 1000
      },
      "validators" : [
         {
            "address" : "buQhmPKU1xTyC3n7zJ8zLQXtuDJmM2zTrJey"
         }
      ],
      "validators_reward" : {
         "buQhmPKU1xTyC3n7zJ8zLQXtuDJmM2zTrJey" : 800000000
      }
   }
}
```

- If the account does not exist, then return, 

``` json
{
   "error_code" : 4,
   "result" : null
}
```

### Submitting Transaction

```text
POST /submitTransaction
```

Data Format

```json
{
  "items" : [{
      "transaction_blob" : "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",//Serialization of a transaction in the hexadecimal format
      "signatures" : [{//first signature
          "sign_data" : "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",//Signature data
          "public_key" : "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"//Public key
        }, {//second signature
          "sign_data" : "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",//Signature data
          "public_key" : "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"//Public key
        }
      ]
    }
  ]
}
```

| Parameters       | Description                                                  |
| :--------------- | ------------------------------------------------------------ |
| transaction_blob | Serialization of a transaction in hexadecimal format. Please fill in the transaction contents referring to [`the definition of transactions`](#定义交易). |
| sign_data        | Signature data in hexadecimal format. You can get this value by signing `transaction_blob`.         Note: Please **DO NOT** sign the `transaction_blob` before it is transferred into bytes stream. |
| public_key       | Public key in hexadecimal format.                            |


### Serializing Transaction

If you do not have a protocol buffer tool, you may serialize transaction with this api.   

| Parameters               | Description                                                  |
| :----------------------- | ------------------------------------------------------------ |
| source_address(required) | Address of the transaction sender                            |
| nonce(required)          | The value must be equal to the present nonce of source address + 1.  You can get this value by [Querying Account](#查询账号) function. |
| ceil_ledger_seq          | Block height limitation. If the value is larger than 0, it means the transaction is valid in less or equal block height. Otherwise,  it is meaningless if the value is 0. |
| metadata(optional)       | User-defined post in hexadecimal format.                     |

Getting more information of operations data types in json, please refer to [Operation](#操作). 

```http
POST /getTransactionBlob
```

```json
{
    "source_address":"xxxxxxxxxxx",//source address, as the sender account of the transaction
    "nonce":2, //nonce value
    "ceil_ledger_seq": 0, //(selected)
    "fee_limit":1000, //transaction fee
    "gas_price": 1000, //gas price (must greater or qual to the minimun value) (不小于配置的最低值)
    "metadata":"0123456789abcdef", //user-defined post in hexadecimal format. （selected）
    "operations":[
    {
      //(according to your need)
    },
    {
      //(according to your need)
    }
    ......
    ]
}
```

return,

```json
{
    "error_code": 0,
    "error_desc": "",
    "result": {
        "hash": "474210d69cf0a797a24be65e187eddc7f15de626d38f8b49446b21ddd12247f8",//transaction hash  交易的hash
        "transaction_blob": "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" //serialization of a transaction in hexadecimal format. 交易序列化之后的16进制表示
    }
}
```

For example, we defined a transaction in json format with only one operation. The operation is creating an account.  
`POST /getTransactionBlob`

```json
{
  "source_address": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3",
  "nonce": 1,
  "fee_limit": 1000000,
  "gas_price": 1000,
  "operations": [{
      "type": 1,
      "create_account": {
        "dest_address": "buQgmhhxLwhdUvcWijzxumUHaNqZtJpWvNsf",
        "metadatas": [{
            "key": "hello",
            "value": "setting metadata while creating an account"
          }
        ],
        "priv": {
          "master_weight": 10,
          "signers": [],
          "thresholds": {
            "tx_threshold": 7
          }
        },
        "contract": {
          "payload": "function main(inputStr){\n /*contract entrance function*/ }"
        }
      }
    }
  ]
}
```

return,

```json
{
   "error_code" : 0,
   "error_desc" : "",
   "result" : {
      "hash" : "106345c939250c26495d461957a17cd58d9f69dc5d21c772e20e1ff337c7c2ce",
      "transaction_blob" : "0a2462755173396e70614371396d4e465a473138717538385a636d5859716436627170545533100122af0108012aaa010a24627551676d6868784c77686455766357696a7a78756d5548614e715a744a7057764e7366123a123866756e6374696f6e206d61696e28696e707574537472297b0a202f2ae8bf99e698afe59088e7baa6e585a5e58fa3e587bde695b02a2f207d1a06080a1a020807223e0a0568656c6c6f1235e8bf99e698afe5889be5bbbae8b4a6e58fb7e79a84e8bf87e7a88be4b8ade8aebee7bdaee79a84e4b880e4b8aa6d65746164617461"
   }
}
```
### Debugging Smart Contract
In the smart contract module, we designed a sandbox for debugging the contract. You won't alter any real status of blockchain or smart contract in the debugging process. BUMO Blockchain offers `callContract`api to help developers debug the smart contract. The smart contract can be stored on the blockchain or uploaded from localhost. Calling `callContract`api won't send any transactions, so there is no transaction fee to pay.

```text
   POST /callContract
```
- returned post, 
```http
{
  "contract_address" : "",
  "code" : "\"use strict\";log(undefined);function query() { return 1; }",
  "input" : "{}",
  "contract_balance" : "100009000000",
  "fee_limit" : 100000000000000000,
  "gas_price": 1000,
  "opt_type" : 2,
  "source_address" : ""
}
```
  - contract_address: Address of the called smart contract.  If it does notexit in the database, it will return **false**. Otherwise, if you fill in a null value, it will return the content in code section. 

  - code：Test code. If contact_address is null, use the code section. If the code section is also null, return to false.

  - input： Pass parameters to the called smart contract 

  - fee_limit : Transaction fee

  - gas_price : Gas price

  - contract_balance : Setting an initial BU balance

  - opt_type: 0:call the writer read-only interface of the smart contract `init`, 1:call the write/read interface of the smart contract `main`,call the read only interface of the smart contract `query`.

  - source_address：Simulating the address that calls the smart contract. 

  - return,

```json
  {
   "error_code" : 0,
   "error_desc" : "",
   "result" : {
      "logs" : {
         "0-buQVkReBYUPUYHBinVDrrb9FQRpo49b9YRXq" : [ "undefined" ]
      },
      "query_rets" : [
         {
            "result" : {
               "type" : "number",
               "value" : "000000000000f03f",
               "valuePlain" : 1
            }
         }
      ],
      "actual_fee" : 0,
      "stat" : {
         "apply_time" : 6315,
         "memory_usage" : 886176,
         "stack_usage": 2564,
         "step" : 3
      },
      "txs" : null
   }
}
```

### Evaluating Transaction Fee
```text
   POST /testTransaction
```
- returned post,
```json
{
  "items": [
    {
      "transaction_json": {
        "source_address": "buQBDf23WtBBC8GySAZHsoBMVGeENWzSRYqB",
        "metadata":"0123456789abcdef", //(selected)
        "nonce": 6,
        "operations": [
          {
            "type": 7,
            "pay_coin": {
              "dest_address": "buQft4EdxHrtatWUXjTFD7xAbMXACnUyT8vw",
              "amount": 10000
            }
          }
        ]
      },
      "signature_number":1
    }
  ]
}
```
Evaluating transaction fee would not alter the account balance. Related sender address and receiver address should exist in the system, except for the address of the initial account. 
  - source_address：Source address of the simulated transaction
  - nonce : add 1 on the original basis
  - signature_number : Signature number ( defaults to 1 )
  - metadata : (optional)
  - fee_limit : (set null)
  - gas_price : (set null)
  - operations : (any operating types)

  - return

```json
{
    "error_code": 0,
    "error_desc": "",
    "result": {
        "hash": "7f0d9de23d6d8f2964a1efe4a458e02e43e47f60f3c22bb132b676c54a44ba04",
        "logs": null,
        "query_rets": null,
        "stat": null,
        "txs": [
            {
                "actual_fee": 264,
                "gas": 264,
                "transaction_env": {
                    "transaction": {
                        "fee_limit": 99999999700110000,
                        "gas_price": 1,
                        "nonce": 1,
                        "operations": [
                            {
                                "pay_coin": {
                                    "amount": 299890000,
                                    "dest_address": "buQkBDTfe4tx2Knw9NDKyntVmsYvYtHmAiE7"
                                },
                                "type": 7
                            }
                        ],
                        "source_address": "buQBDf23WtBBC8GySAZHsoBMVGeENWzSRYqB"
                    }
                }
            }
        ]
    }
}
```

  - gas_price: The minimum price in the system
  - actual_fee: Actual fee for evaluating transaction fee 
  - gas: Gas for evaluating transaction fee 

## Transaction Definition

### Basic Structure of Transactions

- json format
  ```json
  {
      "source_address":"xxxxxxxxxxx",//Source address. As the sender account of the transaction
      "nonce":2, //Nonce value
      "fee_limit" : 1000000, //Transaction fee
      "gas_price": 1000,//Gas price ( no less than the default value )
      "ceil_ledger_seq": 100, //(optional)   It means The limitation of the block height. As the value is over “0”, it should larger than the present block height.
      "metadata":"0123456789abcdef", //(optional) User-defined post in hexadecimal format
      "operations":[
      {
      //(according to your need)
      },
      {
      //(according to your need)
      }
      ......
      ]
  }
  ```

- protocol buffer structure
  ```text
  message Transaction
  {
      enum Limit{
              UNKNOWN = 0;
              OPERATIONS = 1000;
      };
      string source_address = 1;
      int64 nonce = 2;
      int64 fee_limit = 3;
      int64 gas_price =4;
      int64 ceil_ledger_seq = 5;
      bytes metadata = 6;
      repeated Operation operations = 7;
  }
  ```

  **Seven key-sections in a transaction**

  - source_address:  Source address, as the sender of the transaction. After the transaction has been completed, 1 will be added to the nonce of the source automatically  . The nonce here means the amount of transactions this source address has previously made. "

    

  - nonce: The value must be equal to the present nonce of source address + 1. It is designed to prevent **Replay Attack**. ( How to query the nonce of an account. Click [Querying Account](#Quering-account). If the nonce is NULL, it means the present nonce is “0”. 

  

  - fee_limit: The maximum acceptable transaction fee for the current transaction. As the pledge, the initial transaction fee will be locked at the beginning. If the transaction is completed successfully, the actual transaction fee will be deducted, or else the transaction will be charged according to the current number. 

  

  - gas_price: To calculate the fee of every manipulation, including the fee of transaction bytes. 

    

  - ceil_ledger_seq: Advanced feature. The block height limitation of the** current transaction.  

  

  - operations: Manipulation list. The tasks in this transaction. Click [Operation](#操作). 

    

  - metadata: (optional) 

  

  Every class extends from Operation: 

  ```text
  message Operation
  {
      enum Type {
          UNKNOWN = 0;
          CREATE_ACCOUNT = 1;
          ISSUE_ASSET = 2;
          PAY_ASSET = 3;
          SET_METADATA = 4;
          SET_SIGNER_WEIGHT = 5;
          SET_THRESHOLD = 6;
          PAY_COIN = 7;
          LOG = 8;
      };
      Type type = 1;
      string source_address = 2;
      bytes metadata = 3;
  
      OperationCreateAccount create_account = 4;
      OperationIssueAsset issue_asset = 5;
      OperationPayAsset pay_asset = 6;
      OperationSettingMetadata set_metadata = 7;
      OperationSettingSignerWeight set_signer_weight = 8;
      OperationSettingThreshold set_threshold = 9;
      OperationPayCoin pay_coin = 10;
      OperationLog log = 11;
  }
  ```

  - type: operating types.   For example, as the value ISSUEE_ASSET, the `issue_asset`section would be utilized. As the value PAY_ASSET, the `pay_asset`section would be utilized (refer to [Operating Code](#operating-code)).

    

  - source_address: The target address of this operation. It defaults to local source address.

  

  - metadata: Notes for this operation. User-defined, can set null.
  - metadata: (optional) 

### Operation

#### Operating Code 

| Code Value | Name              | Description         |
| :--------- | ----------------- | ------------------- |
| 1          | CREATE_ACCOUNT    | Creating account    |
| 2          | ISSUE_ASSET       | Issuing asset       |
| 3          | PAY_ASSET         | Transferring asset  |
| 4          | SET_METADATA      | Setting metadata    |
| 7          | PAY_COIN          | Paying with BU COIN |
| 9          | SET_PRIVILEGE     | Setting weight and threshold|

#### Creating Account

|Parameters|Description
|:--- | --- 
|dest_address |  Address of the account, create common account: address, create contract account: null. If create specific contract, refer to [Validator Nodes Election](#validator-nodes-election) and [Election Contract of Transaction Fee](#election-contract-of-transaction-fee)
|contract|  Setting null as a normal account. Otherwise, it is a contract account.  
| priv|  Weight information of the account 
|init_balance | Initializing BU value
|init_input | Passing initialized parameters to the contract 

- Function
  create a new account on the blockchain

- Conditions
  - Valid parameters
  - The new account is not an existing one

- **Note: If the target account is a contract account, the `priv`should be set like the FOLLOWING: {"master_weight" : 0 , "thresholds": {"tx_threshold":1}}, if the target account is a common account, the `priv`should be set like the FOLLOWING: {"master_weight" : 1 , "thresholds": {"tx_threshold":1}}**


- json format

Create common account
```json
    {
      "type": 1,
      "create_account": {
        "dest_address": "buQgmhhxLwhdUvcWijzxumUHaNqZtJpWvNsf",
        "contract": {
          "payload": ""
        },
        "init_balance": 100000,  //init_balance to this account
        "init_input" : "",  // if create contract , then init with this input
        "metadatas": [{
            "key": "111",
            "value": "hello 111!",
            "version": 0
          }, {
            "key": "222",
            "value": "hello 222!",
            "version": 0
          }
        ],
        "priv": {
          "master_weight": 1,
          "thresholds": {
            "tx_threshold": 1
          }
        }
      }
    }
```

create contract account
```json
    {
      "type": 1,
      "create_account": 
      {
        "dest_address": "",
        "contract": 
        {
          "payload": "
            'use strict';
            function init(bar)
            {
              return;
            }

            function main(input)
            {
              return;
            }

            function query()
            {
              return;
            }
          "
        },
        "init_balance": 100000,  //init balance to this account
        "init_input" : "{\"method\":\"toWen\",\"params\":{\"feeType\":0}}",
        "priv":  {
          "master_weight": 0,
          "thresholds": {
              "tx_threshold": 1
          }
        }
      }
    }
```
Contract account will be created automatically, and store in transaction which can be search by hash：

```
GET /getTransactionHistory?hash=150dbbf1beaaae23bb3b7148cf65279d7de46a76d7ec8e480ef745f5708beb16
```
return result
``` json
{
    "error_code": 0,
    "result": {
        "total_count": 1,
        "transactions": 
		[
			{
            "actual_fee": 1000402000,
            "close_time": 1528725055019893,
            "error_code": 0,
            "error_desc": "[{\"contract_address\":\"buQfFcsf1NUGY1o25sp8mQuaP6W8jahwZPmX\",\"operation_index\":0}]", //create contract result with contract address and opration index
            "hash": "4cbf50e03645f1075d7e5c450ced93e26e3153cf7b88ea8003b2fda39e618e64",
            "ledger_seq": 14671,
            "signatures": [{
                "public_key": "b00180c2007082d1e2519a0f2d08fd65ba607fe3b8be646192a2f18a5fa0bee8f7a810d011ed",
                "sign_data": "87fdcad0d706479e1a3f75fac2238763cd15fd93f81f1b8889fb798cefbe1752c192bbd3b5da6ebdb31ae47d8b62bb1166dcceca8d96020708f3ac5434838604"
            }],
            "transaction": {
                "fee_limit": 20004420000,
                "gas_price": 1000,
                "nonce": 30,
                "operations": [{
                    "create_account": {
                        "contract": {
                            "payload": "\n\t\t          \n\t\t        \t'use strict';\n\t\t\t\t\tfunction init(bar)\n\t\t\t\t\t{\n\t\t\t\t\t  return;\n\t\t\t\t\t}\n\t\t\t\t\t\n\t\t\t\t\tfunction main(input)\n\t\t\t\t\t{\n\t\t\t\t\t  return;\n\t\t\t\t\t}\n\t\t     function query()\n\t\t\t\t\t{\n\t\t\t\t\t  return;\n\t\t\t\t\t}\n\t\t      \n\t\t          "
                        },
                        "init_balance": 10000000,
                        "priv": {
                            "thresholds": {
                                "tx_threshold": 1
                            }
                        }
                    },
                    "type": 1
                }],
                "source_address": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3"
            },
            "tx_size": 402
        }]
    }
}

```
create contract result

```transactions.error_code```:  0: sucess; other: error

```transactions.error_desc```: when error, error description; success, if have create contract, save the 

create contract result with json desciption 

  ``` json
  [
    {
      "contract_address": "buQm5RazrT9QYjbTPDwMkbVqjkVqa7WinbjM", //contract address
      "operation_index": 0                                        //transaction index, 0 is the first
    }
  ]
  ```

- protocol buffer structure

  ```text
  message OperationCreateAccount
  {
      string dest_address = 1;
      Contract contract = 2;
      AccountPrivilege priv = 3;
      repeated KeyPair metadatas = 4;
      int64    init_balance = 5;
      string  init_input = 6;
  }
  ```

  - dest_address: Address of the new account
  - contract: If you would like to create an account with smart contract functions, please refer to [Contract](#合约). Otherwise, if you would like to create a normal account, you can set the value null. 
  - priv: Distribute the weight of account. Some related definition as following,
      ```text
        message OperationTypeThreshold
        {
            Operation.Type type = 1;
            int64 threshold = 2;
        }
      
        message AccountPrivilege
        {
            int64 master_weight = 1;
            repeated Signer signers = 2;
            AccountThreshold thresholds = 3;
        }
        message Signer
        {
            enum Limit
            {
                SIGNER_NONE = 0;
                SIGNER = 100;
            };
            string address = 1;
            int64 weight = 2;
        }
        message AccountThreshold
        {
            int64 tx_threshold = 1; //required, [-1,MAX(INT64)] -1:set null
            repeated OperationTypeThreshold type_thresholds = 2; // threshold with higher priority 
        }
      ```

  If you would like to create a joint-management account, please refer to [Setting Privilege](#setting-privilege) .

  

  metadatas: Metadata list. You can set a batch of initial metadata in the new account. The format is **Key-Pair**, as  follow: 

  ```text
  message KeyPair
  {
      string key = 1;
      string value = 2;
      int64 version = 3;
  }
  ```

  This part is for a versioning key-pair database. If you do not have the requirement, please ignore it.

  - init_balance: Initial BU balance for a newly created account

  - init_input:  If the newly created account is for contract, then pass the parameter to initialize the contract.

#### Issuing Assets

|Parameters|Description
|:--- | --- 
|amount |  issued amount
|code|  codes of asset

- Function
  A source account issues an amount of asset. After successful completion, the asset will be added to your balance
- Conditions
  - Valid parameters
  - Does not run out of asset setting(int64）
- json format

    ```json
    {
      "type": 2,
      "issue_asset": {
        "amount": 1000,
        "code": "CNY",
      }
    }
    ```
- protocol buffer structure
    ```text
    message OperationIssueAsset
    {
        string code = 1;
        int64 amount = 2;
    }
    ```
    - code: Codes for issuing assets, length [1, 64]
    - Amount of the assets, length (0, MAX(int64)) 
    - After you complete successfully, the assets will be added into your source account balance. 

#### Transferring Assets

|Parameters|Description
|:--- | --- 
|pay_asset.dest_address |  receiver account
|pay_asset.asset.key.issuer|  assets issuer
|pay_asset.asset.key.code|  codes of assets
|pay_asset.asset.amount|  asset amount
|pay_asset.input|  input for tiggering the smart contract (defaults to null)

- Function
  Transfer the asset from sender account to receiver account.
- Conditions
  - valid parameters
  - sender account has sufficient balance
- json format


  ```JSON
    {
      "type": 3,
      "pay_asset": {
        "dest_address": "buQgmhhxLwhdUvcWijzxumUHaNqZtJpWvNsf",
        "asset": {
          "key": {
            "issuer": "buQgmhhxLwhdUvcWijzxumUHaNqZtJpWvNsf",
            "code": "CNY",
            "type": 0 // 0 or set null
          },
          "amount": 100
        },
        "input": "{\"bar\":\"foo\"}"
      }
    }
  ```

- protocol buffer structure
    ```text
    message OperationPayAsset
    {
        string dest_address = 1;
        Asset asset = 2;
        string input = 3;
    }
    ```
    - dest_address: address of receive account
    - asset: asset for transfer
    ```text
    message Asset
    {
         AssetKey key = 1; //asset property
         int64 amount = 2; //amount        
    }
    
    message AssetKey
    {
         string issuer = 1; //asset issuer
         string code = 2; //codes of asset
         int32 type = 3;   //type of asset
    }
    ```
    - input: The asset is transferred as an input to trigger the smart contract of receiver account.

#### Setting Metadata
|Parameters|Description
|:--- | --- 
| set_metadata.key  |required, length:(0, 1024]
| set_metadata.value  |optional, length:[0, 256K]
| set_metadata.version |optional, default 0, 0:no version limitation; >0:the number should be*** equal to the current version;  <0:ilvalid

- Function
  Insert or update metadata to source account metadata database. 
- Conditions
  - Valid parameters
- json format

    ```JSON
    {
      "type": 4,
      "set_metadata": {
        "key": "abc",
        "value": "hello abc!",
        "version": 0
      }
    }
    ```

- protocol buffer structure
    ```text
    message OperationSettingMetadata
    {
        string key = 1;
        string value = 2;
        int64 version = 3;
    }
    ```
    - key: Unique for the account, length [1, 1024] 
    - value: length [0,256K]
    - version: Set null.  If you would like to get more advanced function, please refer to [Control Version](#版本化控制). 

#### Setting Privilege
|Parameters|Description
|:--- | --- 
|master_weight |optional, string format, defaults to "", set self's master weight; "": do nothing; "0": set the weight of master as 0; ("0", "MAX(UINT32)"]: set the weight as the value, others are invalid.
|signers |optional, signer address list, default null. null: do nothing; other: set signers
|address |Signer address ( should be valid )
|weight | optional, defaults to 0; 0: delete the signer; (0, MAX(UINT32)]: Set the weight as the value, others are invalid
|tx_threshold |optional，string format, default to "", denotes the lowest weight of this account. "": do nothing; "0": set the weight 0; (0, MAX(INT64)]: set the weight as the value, others are invalid.
|type |Type of specific operation (0, 100]
|threshold | optional，default 0, 0 : delete the operation; (0,MAX(INT64)): Setting the weight as the value, others are invalid.

- Function 
  Set the weight of the signer and set the threshold required for each operation.
- Conditions
  - Valid parameters
- json format
    ```json
      {
          "set_privilege": 
          {
            "master_weight": "10",
            "signers": 
            [
              {
              "address": "buQqfssWJjyKfFHZYx8WcSgLVUdXPT3VNwJG",
              "weight": 8
              }
            ],
            "tx_threshold": "2",
            "type_thresholds": 
            [
              {
                "type": 1,
                "threshold": 8
              }, 
              {
                "type": 2,
                "threshold": 9
              }
            ]
          },
          "type": 9
      }
    ```

- protocol buffer structure
    ```text
     message OperationSetPrivilege
     {
        string master_weight = 1;
        repeated Signer signers = 2;
        string tx_threshold = 3;
        repeated OperationTypeThreshold type_thresholds = 4;
     }

     message OperationTypeThreshold
     {
        Operation.Type type = 1;
        int64 threshold = 2;
     }

     message Signer 
     {
        enum Limit
        {
            SIGNER_NONE = 0;
            SIGNER = 100;
        };
        string address = 1;
        int64 weight = 2;
     }
      
    ```

#### Transferring BU Assets

|Parameters|Description
|:--- | --- 
|pay_coin.dest_address |  address of receiver account
|pay_coin.amount|  amount
|pay_coin.input|  input for triggering the smart contract (defaults to null) 

- Function
  Transferring the asset form sender account to receiver account.
- Conditions
  - Valid parameters
  - Sender account has sufficient balance
- json format


  ```JSON
    {
      "type": 7,
      "pay_coin": {
          "dest_address": "buQgmhhxLwhdUvcWijzxumUHaNqZtJpWvNsf",
          "amount": 100,
          "input": "{\"bar\":\"foo\"}"
        }
      }
    }
  ```

- protocol buffer structure
    ```text
    message OperationPayCoin
    {
        string dest_address = 1;
        int64 amount = 2;
        string input = 3;
    }
    ```
    - dest_address: Address of BU receiver account
    - amount: Amount of BU
    - input: The asset is transfered as an input to trigger the smart contract of receiver account.

#### Log
|parameters|Description
|:--- | --- 
|log.topic |  log topic, string type, length(0,128]
|log.datas|  log contents, string type in array, length (0,1024]

- Function
  Recording the data on blockchain 
- Conditions
  - Valid parameters
- json format


  ```JSON
    {
      "type": 8,
      "log": {
          "topic": "hello",
          "datas":
          [
            "hello, log 1",
            "hello, log 2"
          ]
      }
    }
  ```

- protocol buffer structure
    ```text
      message OperationLog{
        string topic = 1;
        repeated string datas = 2;
      }
    ```
    - topic: log topic
    - datas: log contents


## Advanced Features

### Distributing weight 

As you are creating a new account, you can assign the weight for this account from setting the `priv`. the following is a simple example, 

```json
{
    "master_weight": "70",// the weight of local address
    "signers": [//distribute weight
        {
            "address": "buQc39cgJDBaFGiiAsRtYKuaiSFdbVGheWWk",
            "weight": 55    //the weight value of this address is 55
        },
        {
            "address": "buQts6DfT5KavtV94JgZy75H9piwmb7KoUWg",
            "weight": 100    //the weight value of this address is 100
        }
    ],
    "tx_threshold": "8",//required thresholds for transaction is 8 
    "type_thresholds": [
        {
            "type": 1,//required thresholds for creating account is 11
            "threshold": 11
        },
        {//required thresholds for issuing asset is 21
            "type": 2,
            "threshold": 21
        },
        {//required thresholds for transferring assets is 31
            "type": 3,
            "threshold": 31
        },
        {//required thresholds for setting metadata is 41
            "type": 4,
            "threshold": 41
        },
        {//required thresholds for changing controler's weight is 51
            "type": 5,
            "threshold": 51
        },
        {//required thresholds for altering every operations is 51
            "type": 6,
            "threshold": 61
        }
    ]
}
```

### Controlling Version

Every metadata of accounts is a micro database. Versioning can prevent the conflictions of altering. 

### Smart Contract

Smart Contract is a section of ECMAScript as specified in ECMA-262. The codes of smart contract should contain two parts of functions. The first is initialization function, `init`. The other one is  entry function, `main`. When you call the main function you have to assign the input (type string). The following is a simple example, 

```javascript
"use strict";
function init(bar)
{
  /*init whatever you want*/
  return;
}

function main(input)
{
  let para = JSON.parse(input);
  if (para.do_foo)
  {
    let x = {
      'hello' : 'world'
    };
  }
}

function query(input)
{ 
  return input;
}
```

The system offers several global variables for getting the information of blockchain and drive the transactions, except setting thresholds and weight. 

**Note: Do not duplicate the name of global variables or default variables as the custom fuctions. It will lead to uncontrollable error.**

#### Syntax

Referenced Documents: [Smart Contract Rules](../src/web/jslint/ContractRules.md) ](../src/web/jslint/ContractRules.md)

#### Default Functions
- ##### Read/Write Rights of functions 
    Every functions have a stable **read-only** or **write** authority. **Read-only** means the interface functions which would not write the data on the blockchain, such as `getbalance`. **Write** means the interface functions which would write data on the blockchain, such as transferring `payCoin`. While coding the smart contract, please pay attention to the authority for calling different entry functions. `init`and `main`functions can call all the default functions, but `query`can only call the **read-only**functions. Or the interface will be undefined during debugging or execution.



- ##### return
   For all the default functions, if they fail, then, return *false* or throw the exceptions directly. Otherwise, they will return other objects.If there is a parameter error, the error description will describe the parameter location, which refers to the index number of the parameter, i.e., counting from __0__. e.g.

   ```
   issueAsset("CNY", 10000);
   /*
      error description:
      Contract execute error,issueAsset parameter 1 should be a string

      Means that the second argument should be a string
   */

- ##### Get account information (except metadata and assets) 

    `getBalance(address);`
    - address: Address of the account

    example,
    ```javascript
    let balance = getBalance('buQsZNDpqHJZ4g5hz47CqVMk5154w1bHKsHY');
    /*
      Authority: Read-Only 
      return, numbers in stype string '999911110000'
    */
    ```

- ##### Store metadata of contract account
  `storageStore(metadata_key, metadata_value);`
  - metadata_key: key of metadata 
  - metadata_value: value of metadata

  ```javascript
  storageStore('abc', 'values');
  /*
    Authority: Read-Only 
    return, Successfully->true; failed->exceptions
  */
  
  ```

- ##### Get metadata of contract account
  `storageLoad(metadata_key);`
  - metadata_key: metadata的key
  ```javascript
  let value = storageLoad('abc');
  /*
    Authority: Read-Only
    return, Successfully->value in type string; failed->exceptions
  */
  
  ```
    then, obtain the custom data 'abc' in this contract account.

- ##### Delete metadata of contract account
  `storageDel(metadata_key);`
  - metadata_key: metadata的key
  ```javascript
  storageDel('abc');
  /*
    Authority: Read-Only
    return, Successfully->true; failed->exceptions
  */
  
  ```
    then, delete the custom data 'abc' in this contract account. 

- ##### Obtain assets information of an account  

    `getAccountAsset(account_address, asset_key);`

    - account_address: address of the account
    - asset_key: property of the assets

    example,
    ```javascript
    let asset_key =
    {
      'issuer' : 'buQsZNDpqHJZ4g5hz47CqVMk5154w1bHKsHY',
      'code' : 'CNY'
    };
    let bar = getAccountAsset('buQsZNDpqHJZ4g5hz47CqVMk5154w1bHKsHY', asset_key);
    
    /*
      Authority: Read-Only
      return, Successfully->assets amount, such as '10000'; failed->exceptions
    */
    ```

- ##### Get block information

    `getBlockHash(offset_seq);`
    - offset_seq: offset from the latest block. The maximun is 1024.

    example
    ```javascript
    let ledger = getBlockHash(4);
    /*
      Authority: Read-Only
      return, Successfully->value in type string, such as 'c2f6892eb934d56076a49f8b01aeb3f635df3d51aaed04ca521da3494451afb3'; failed->false
    */
    
    ```

- ##### Check the validity of the address

    `addressCheck(address);`
    - address: address name in type string

    example,
    ```javascript
    let ret = addressCheck('buQgmhhxLwhdUvcWijzxumUHaNqZtJpWvNsf');
    /*
      Authority: Read-Only
      return, Successfully->true; failed->false
    */
    
    ```

- ##### Check the validity of string number

    `stoI64Check(strNumber);`
    - strNumber：string number in type string

    example,
    ```javascript
    let ret = stoI64Check('12345678912345');
    /*
      Authority: Read-Only
      return, Successfully->true; failed->false
    */
    
    ```

- ##### Addition in 64 bits

    `int64Add(left_value, right_value);`
    - left_value: left value
    - right_value：right value

    example,
    ```javascript
    let ret = int64Add('12345678912345', 1);
    /*
      Authority: Read-Only
      return, Successfully->value in type string, such as'12345678912346'; failed->exceptions
    */
    
    ```

- ##### Subtraction in 64 bits

    `int64Sub(left_value, right_value);`
    - left_value: left value
    - right_value：right value

    example,
    ```javascript
    let ret = int64Sub('12345678912345', 1);
    /*
      Authority: Read-Only
      return, Successfully->value in type string, such as'12345678912346'; failed->exceptions
    */
    
    ```

- #####  Multiplication in 64 bite

    `int64Mul(left_value, right_value);`
    - left_value: left value
    - right_value：right value

    example,
    ```javascript
    let ret = int64Mul('12345678912345', 2);
    /*
      Authority: Read-Only
      return, Successfully->value in type string as '24691357824690'; failed->exceptions
    */
    
    ```

 - ##### Division in 64 bits

    `int64Div(left_value, right_value);`
    - left_value: left value
    - right_value：right value

    example,
    ```javascript
    let ret = int64Div('12345678912345', 2);
    /*
      Authority: Read-Only
      return, Successfully->value in type string as'6172839456172'; failed->exceptions
    */
    
    ```

 - ##### Modulus in 64 bits

    `int64Mod(left_value, right_value);`
    - left_value: left value
    - right_value：right value

    example,

    ```javascript
    let ret = int64Mod('12345678912345', 2);
    /*
      Authority: Read-Only
      return, Successfully->value in type string, such as'1'; failed->exceptions
    */
    
    ```

 - ##### Comparison in 64bits
    `int64Compare(left_value, right_value);`

    - left_value: left value
    - right_value：right value

    example,

    ```javascript
    let ret = int64Compare('12345678912345', 2);
    /*
      Authority: Read-Only
      return, Successfully-> 1(left is larger than right), 0 (equal), -1 (smaller); failed->exceptions
    */
    
    ```

 - ##### Switching units
    `toBaseUnit(value);`

    - return, Successfully->value multiply 10^start^ in type string; failed->false
    - value: only accept string number( including the number with point down to the eighth decimal point )

    example,
    ```javascript
    let ret = toBaseUnit('12345678912');
    /*
      Authority: Read-Only
      return, Successfully->value in type string as'1234567891200000000'; failed->exceptions
    */
    
    ```

- ##### Log

    `log(info);`
     info: log contents

    example,
    ```javascript
    let ret = log('buQsZNDpqHJZ4g5hz47CqVMk5154w1bHKsHY');
    /*
      Authority: Read-Only
      return, Successfully->null; failed->exceptions
    */
    ```

- #### Log

    `tlog(topic,args...);`
     - `tlog` will record a transaction on the block
     - topic: Log topic, type string, length(0,128]
     - args...: At most 5 parameters; type can be string, value, or boolean; length(0,1024] 

    example,
    ```javascript
    tlog('transfer',sender +' transfer 1000',true);
    /*
      Authority: Read-Only
      return, Successfully->true; failed->exceptions
    */
    ```

- ##### Issuing assets

    `issueAsset(code, amount);`
     - code: Codes of assets
     - amount: Amount of assets

    example,
    ```javascript
    issueAsset("CNY", "10000");
    /*
      Authority: Read-Only
      return, Successfully->true; failed->exceptions 
    */
    ```

- ##### Transferring assets

    `payAsset(address, issuer, code, amount[, input]);`
     - address: Address of the receiver account
     - issuer: Assets issuer 
     - code: Codes of assets
     - amount: Amount 
     - input: Optional; defaults to null

    example,
    ```javascript
    payAsset("buQsZNDpqHJZ4g5hz47CqVMk5154w1bHKsHY", "buQgmhhxLwhdUvcWijzxumUHaNqZtJpWvNsf", "CNY", "10000", "{}");
    /*
      Authority: Read-only
      return, Successfully->true; failed->exceptions   
    */
    ```

- ##### Transfer

    `payCoin(address, amount[, input]);`
     - address: Address of BU receiver account
     - amount: BU amount
     - input: Optional; defaults to null

    example,
    ```javascript
    payCoin("buQsZNDpqHJZ4g5hz47CqVMk5154w1bHKsHY", "10000", "{}");
    /*
      Authority: Writable
      return, Successfully->true; failed->exceptions  
    */
    ```

- ##### Condition

    `assert(condition[, message]);`
     - condition: Condition variable
     - message: Optional

    example,
    ```javascript
    assert(1===1, "Not valid");
    /*
      Authority: Read-only
      return, Successfully->true; failed->exceptions  
    */
    ```

#### Default Variables

- #####  address of the contract account
   thisAddress

   

   Global variable `thisAdress` is the address of this contract account. For example, If account X submits a transaction to call contract Y, `thisAddress`is Y's address.

    ```text
    let bar = thisAddress;
    /*
    bar is Y's address 
    */
    ```

- ##### BU coin for current operation
    thisPayCoinAmount

- ##### Asset for this operation
    thisPayAsset

    format=>{"amount": 1000, "key" : {"issuer": "buQsZNDpqHJZ4g5hz47CqVMk5154w1bHKsHY", "code":"CNY"}}

- ##### Current block height *CAPTIALS**
    blockNumber

- ##### Current block timestamp
    blockTimestamp

- ##### ddress of the caller (call the function/variables)
    sender
    

    `sender`is the address of the caller's account. For example, there is an operation (from address X) to call contract Y in a transaction. In this process, address X is the `sender`.

    ```javascript
    let bar = sender;
    /*
    bar is X&#39;s address
    */OOperation 
    ```

- ##### Operation sequence for triggering the contract

    triggerIndex

    

    `triggerIndex`is the operation sequence for triggering the contract. For example, account A submits a transaction *tx0*, and the 0-th (count from 0) operation is transferring assets to  a contract account (calling the contract). Then `triggerIndex`is 0.

    ```javascript
    let bar = triggerIndex;
    /* bar is an int*/
    ```

#### Exceptions

- JavaScript exceptions

  While there is an uncaught JavaScript exception in contract operation,

  1.Failed execution, all transactions in this contract operation are failed.

  2.The transaction triggrted (TRIGGERED) the contract is failed with error code151.

- Failure in executing transactions***
  <font color=red>Contracts can execute several transactions, but only one fault would lead to failure of all transactions *** and throw exceptions. </font>

### Validator Nodes Election

#### Creating Election Contract Account
Before you create an election contract account, you cannot complete the following operations. The new account should be global and without space repetition. 

- Creating a [contract account](#创建账号), and the address of this contract should be `buQtxgoaDrVJGtoPT66YnA2S84yE8FbBqQDJ`.  
- Copying all the source codes from `src\ledger\validators_vote.js`as a section in parameter `payload` of the contract account.  

example,

```
     "contract" :
     {
       "contract_id" : "something identify this contract",
       "payload" : "copy all the source codes from src\ledger\validators_vote.js and paste here"
     },
```

You can alter the following variables from the contract codes which is assigned by `validator_vote.js`.  As the election account is created, these variables can not be changed any more.  

 ```
   let validatorSettingSize       = 100;
   let votePassRate           = 0.7;
   let effectiveVoteInterval  = 15 * 24 * 60 * 60 * 1000 * 1000;
   let minPledgeAmount        = 50000 * 100000000;
   let minSuperadditionAmount = 100 * 100000000;
 ```

 - validatorSettingSize: The number of validator nodes in appointed network
 - votePassRate:  Vote passing rate. If votes  is larger than or equal to validator nodes number * votePassRate after **rounded**, the vote is successful. For example, there are 4 validator nodes and the votePassRate is 0.7. Then the votes should be larger than or equal to 3 (4*0.7=2.8). If there are 6 validator nodes and the votePassRate is 0.7. Then the votes should be larger than or equal to 4 (6**0.7=4.2).
 - effectiveVoteInterval:  Setting the effective interval of the vote (in ms). If the time period is out of the interval, the proposal and vote would be canceled. 
 - minPledgeAmount: The minimun pledge amount 
 - minSuperadditionAmount: The minimum superaddition pledge amount

#### Candidates of Validator Nodes

Any account with network nodes may apply as a validator candidate. This is done through transferring an amount of a pledger as deposit to the validator nodes account to formally apply. However, decision on successful application depends on votes from other validators.  

- Appliers have to  transfer a pledger to election validator  nodes account (refer to [Transfer BU Asset](#转移BU资产)). The pledge can be taken back by [Forfeit the Pledge](##收回押金). 
- Fill in the section { "method" : "pledgeCoin"} as the input of transferring coin. Remember to escape characters.

example,

```
  "pay_coin" :
  {
    "dest_address" : "buQtxgoaDrVJGtoPT66YnA2S84yE8FbBqQDJ",
    "amount" :100000,
    "input":
    "{\"method\":\"pledgeCoin\"}"
  }
```

Validators nodes will vote on the appliers (refer to [Vote on the Validator Nodes Appliers](#对验证节点候选人申请者投票)). If validator nodes vote for the Applier, they can become a formal candidate of validator nodes. 

It depends on the pledge amount for becoming a formal validator nodes. Suppose that the net needs 100 validator nodes (`validatorSettingSize`= 100),  the candidates whose pledge are among top 100 can become the formal validators. The pledge can be added as many times as they want (refer to Candidates of Validator Nodes)

#### Voting on Validator Nodes Appliers

If validator nodes vote for the Appliers, they can become formal candidates of validator nodes. The pledge can be taken back (refer to [Forfeit the Pledge](#Forfeit the Pledge) ) if the candidates would not vote through becoming formal validator nodes. 

- Transfer any number of asset (including 0) to election validator nodes contract account. 
- Fill in the section { "method" : "voteForApplicant", "params" : { "address" : "applier's address" } }as the input of transferring asset or coin. Remember to escape characters. 

example,

```
  "pay_coin" :
  {
    "dest_address" : "buQtxgoaDrVJGtoPT66YnA2S84yE8FbBqQDJ",
    "amount" :0,
    "input":
    "{ 
        \"method\":\"voteForApplicant\",
        \"params\":
        { 
          \"address\":\"buQtZrMdBQqYzfxvqKX3M8qLZD3LNAuoSKj4\"
        }
    }"
  }
```

Note: Only the validator nodes have the authority to vote, and only for once. If the time period is out of the interval, the proposal and vote will be cancelled. 

#### Forfeit the Pledge

Validator nodes, candidates of validator nodes, and Appliers of candidates can forfeit the pledge through this operation.

- Transfer any number of asset (including 0) to election validator nodes contract account. 
- Fill in the section { "method":"takebackCoin" } as the input of transferring coin. Remember to escape characters. 

example,

```
  "pay_coin" :
  {
    "dest_address" : "buQtxgoaDrVJGtoPT66YnA2S84yE8FbBqQDJ",
    "amount" :0,
    "input":"{\"method\":\"takebackCoin\"}"
  }
```

After this operation, election account will return all the pledge to the sender account. If the sender accounts have been validator nodes, they will lose the identity as the validators. Election contract will vote on new validators to replace them.

#### Banning Malicious Nodes

If a validator node finds a malicious or unqualified node, it can submit **banning the malicious nodes**. To ban the malicious nodes, all of the validator nodes should vote on the proposal. 

- The proposers transfer any number of asset (including 0) to election validator nodes contract account. 
- Fill in the section { "method" : "abolishValidator",  "params" : { "address" : "address of the malicious node", "proof" : "reasons of this proposal"} } as the input of transferring coin or assets. Remember to escape characters.  

example,

```
  "pay_coin" :
  {
    "dest_address" : "buQtxgoaDrVJGtoPT66YnA2S84yE8FbBqQDJ",
    "amount" :0,
    "input":
    "{
      \"method\":\"abolishValidator\",
      \"params\":
      {
        \"address\":\"buQmvKW11Xy1GL9RUXJKrydWuNykfaQr9SKE\"，
        \"proof\":\"I_saw_it_uncomfotable.\"
      }
    }"
  }
```

Note: The proposer nodes and the malicious nodes both are validator nodes.



#### Revoking Banning Malicious Nodes
Operations can be revoked if initiated proposes are found to be wrong. However, once a proposal is passed, it can no longer be revoked.

- The proposers transfer any number of asset (including 0) to election validator nodes contract account.
- Fill in the section  { "method" : "quitAbolish",  "params" : { "address" : "address of the malicious node" } } as the input of transferring coin or assets. Remember to escape characters. 

example,

```
  "pay_coin" :
  {
    "dest_address" : "buQtxgoaDrVJGtoPT66YnA2S84yE8FbBqQDJ",
    "amount" :0,
    "input":
    "{ 
      \"method\":\"quitAbolish\",
      \"params\":
      { 
        \"address\":\"buQmvKW11Xy1GL9RUXJKrydWuNykfaQr9SKE\"
      }
    }"
  }
```

Note: Only proposers may revoke the proposal.



#### Voting on Banning Malicious Nodes 
After voting through banning the malicious nodes,  targeted nodes would be banned. The election contract will forfeiture 10% pledge of this malicious node, and dividing the fine equally as the other nodes' pledge.  The remainder of the fine obtained by moduling shall be rewarded to the validator node with largest stake.

- The validator nodes transfer any number of asset (including 0) to election validator nodes contract account. 
- Fill in the section { "method" : "voteForAbolish", "params" : { "address" : "address of the malicious node"} } as the input of transferring coin or assets. Remember to escape characters.   

example,

```
  "pay_coin" :
  {
    "dest_address" : "buQtxgoaDrVJGtoPT66YnA2S84yE8FbBqQDJ",
    "amount" :0,
    "input":
    "{
       \"method\":\"voteForAbolish\",
      \"params\":
      {
         \"address\":\"buQmvKW11Xy1GL9RUXJKrydWuNykfaQr9SKE\"
      }
    }"
  }
```

Note: Only the validator nodes have voting rights. The proposal would be voided once the effective voting period expires. If the time period is out of the interval, the proposal and vote would be cancelled. The target nodes continue to be validator nodes.



#### Querying Functions

By sending appointed parameters to the query interface (query api), you can obtain  the related information. Currently, you can only call the query interface by sending sections `callContract`and `contract_address`, filling in address of validator candidates' election account.

##### Querying Current Validator Nodes Setting

example,

```
  {
    "contract_address" : "buQtxgoaDrVJGtoPT66YnA2S84yE8FbBqQDJ",
    "code" : "",
    "input" : "{\"method\": \"getValidators\"}",
    "opt_type" : 2,
    "source_address" : ""
  }
```

##### Querying List of Validator Nodes Candidates and Pledge 

example:

```
  {
    "contract_address" : "buQtxgoaDrVJGtoPT66YnA2S84yE8FbBqQDJ",
    "code" : "",
    "input" : "{\"method\": \"getCandidates\"}",
    "opt_type" : 2,
    "source_address" : ""
  }
```

##### Querying Proposals of Applying to be Candidate Nodes 
Fill in the appliers' address into section `address`.

example,

```
  {
    "contract_address" : "buQtxgoaDrVJGtoPT66YnA2S84yE8FbBqQDJ",
    "code" : "",
    "input" :
    "{
      \"method\": \"getApplicantProposal\",
      \"params\":
      {
         \"address\":\"buQmvKW11Xy1GL9RUXJKrydWuNykfaQr9SKE\"
      }
    }",
    "opt_type" : 2,
    "source_address" : ""
  }
```

##### Querying Proposals of Banning Malicious Nodes 
Fill in the address of the malicious nodes into section `address`. 

example,

```
  {
    "contract_address" : "buQtxgoaDrVJGtoPT66YnA2S84yE8FbBqQDJ",
    "code" : "",
    "input" :
    "{
      \"method\": \"getAbolishProposal\",
      \"params\":
      {
         \"address\":\"buQmvKW11Xy1GL9RUXJKrydWuNykfaQr9SKE\"
      }
    }",
    "opt_type" : 2,
    "source_address" : ""
  }
```

### Election Contract of Transaction Fee

This contract is for planning the standard of transaction fee. Every standard of transaction fee is submitted and voted on by validator nodes. The selected one will become the new standard of transaction fee, and deployed on the next block. Once the effective voting period (15 days) expires, the proposal and votes will be voided. 

address of this contract:` buQiQgRerQM1fUM3GkqUftpNxGzNg2AdJBpe`

#### Structure of Transaction Fee

  ```text
  message FeeConfig
  {
	enum Type {
		UNKNOWN = 0;
		GAS_PRICE = 1;
		BASE_RESERVE = 2;
	};
	int64 gas_price = 1;
	int64 base_reserve = 2;
  }
  ```

#### Types of Transaction Fee

| Code | Names        | Description   |
| :--- | ------------ | ------------- |
| 1    | GAS_PRICE    | Minimum gas   |
| 2    | BASE_RESERVE | Reserved cost |

#### Querying Transaction Fee History

```text
GET /getLedger?seq=xxxx&with_fee=true
```

| Parameters | Description                                                  |
| :--------- | ------------------------------------------------------------ |
| seq        | sequence of the ledger. Api will return the current ledger as the`seq`is null. |
| with_fee   | Defaults to `false`. Concealing the fee list.                |

- If the ledger exists, then returns,

```json
{
    "error_code": 0,
    "result": {
        "fees": {
            "base_reserve": 5,
            "gas_price": 1000
        },
        "header": {
            "account_tree_hash": "c4e74a027e7fb7a96fa2c028a24fd8296df049decf928ee173ec4106ae016b0c",
            "consensus_value_hash": "2779905317234108f76cf5d65125cc0e9a3fd58e2140226bf79e8917028c8c9c",
            "fees_hash": "d7618678ae72e7154fc4366f0a43154ee6bb786b3cefd46287e608e90e13a651",
            "hash": "0c1b9d4b5a72d42f73917114c71672f78a3d951b7f7b89663d10138ea409c308",
            "seq": 1,
            "validators_hash": "cc4b0517295210c24ab037e3e9e122fd0c71e3c87994b7f913860de0a1930c36",
            "version": 1000
        }
    }
}
```

#### Querying Proposals of Transaction Fee

Call the query interface by sending the section `callContract`. The format of the input is json. 

```json
{
    "method":"queryProposal",
    "params":""
}
```

JSON format contents should be transmitted into type string, then fill in the `callContract`. 

```json
{
    "contract_address" : "buQiQgRerQM1fUM3GkqUftpNxGzNg2AdJBpe",
    "code" : "",
    "input" : "{\"method\":\"queryProposal\",\"params\":\"\"}",
    "opt_type" : 2,
    "source_address" : "",
    "fee_limit":100000,
    "gas_price":1000
}
```

`contract_address` is the election contract address of transaction fee. `opt_type`:2 means calling the query interface. 


- If the contents exist, then returns,

```json
{
    "error_code": 0,
    "error_desc": "",
    "result": {
        "logs": {
            "0-buQiQgRerQM1fUM3GkqUftpNxGzNg2AdJBpe": null
        },
        "query_rets": [
            {
                "result": {
                    "type": "string",
                    "value": "{\"buQft4EdxHrtatWUXjTFD7xAbMXACnUyT8vw1\":{\"accountId\":\"buQft4EdxHrtatWUXjTFD7xAbMXACnUyT8vw\",\"proposalId\":\"buQft4EdxHrtatWUXjTFD7xAbMXACnUyT8vw1\",\"feeType\":1,\"price\":5,\"voteCount\":0,\"time\":1517470155872949}}"
                }
            }
        ],
        "actual_fee": 0,
        "stat": {
            "apply_time": 11342,
            "memory_usage": 1325072,
            "stack_usage": 2564,
            "step": 16
        },
        "txs": null
    }
}
```

The contents in the section `result` is the returns. After deserializing the returns into json format, you can obtain all the proposals of transaction fee. 

#### Querying Votes

Call the query interface by sending the section `callContract`. The format of the input is json. You can query votes by te proposals' ID. 

```json
{
    "method":"queryVote",
    "params":{
      "proposalId": "buQft4EdxHrtatWUXjTFD7xAbMXACnUyT8vw1"
    }
}
```

JSON format contents should be transmitted into type string, then fill in the `callContract`. 

```json
{
    "contract_address" : "buQiQgRerQM1fUM3GkqUftpNxGzNg2AdJBpe",
    "code" : "",
    "input" :"{\"method\":\"queryVote\",\"params\":{\"proposalId\":\"buQft4EdxHrtatWUXjTFD7xAbMXACnUyT8vw1\"}}",
    "opt_type" : 2,
    "source_address" : "",
    "fee_limit":100000,
    "gas_price":1000
}
```

`contract_address` is the election contract address of transaction fee. `opt_type`:2 means calling the query interface. 


- If the contents exist, then returns,

```json
{
    "error_code": 0,
    "error_desc": "",
    "result": {
        "logs": {
            "0-buQiQgRerQM1fUM3GkqUftpNxGzNg2AdJBpe": null
        },
        "query_rets": [
            {
                "result": {
                    "type": "string",
                    "value": "{\"buQft4EdxHrtatWUXjTFD7xAbMXACnUyT8vw\":1}"
                }
            }
        ],
        "actual_fee": 0,
        "stat": {
            "apply_time": 18020,
            "memory_usage": 1326456,
            "stack_usage": 2564,
            "step": 19
        },
        "txs": null
    }
}
```
The contents in the section `result` is the returns. After deserializing the returns into json format, you can obtain all the proposals of transaction fee.

#### Proposals of Transaction Fee

Submitting the transaction fee proposals to the contract by transferring assets or coin.  The input format is json. Every account can only submits a proposal of transaction fee. If proposals are submitted repeatedly, the latter replaces the former. 

```json
{
  "method":"proposalFee",
    "params":{
        "feeType": 1, //types of fee
        "price": 5    //int. >= 0
    }
}
```

JSON format contents should be transmitted into type string, then fill in the `paycoin`.

```json
{
    "type" : "PAY_COIN",
    "pay_coin" : {
       "dest_address" :"buQiQgRerQM1fUM3GkqUftpNxGzNg2AdJBpe",
       "amount":0,
        "input":"{\"method\":\"proposalFee\",\"params\":{\"feeType\":1,\"price\":5}}"
    }
}
```

#### Selecting Transaction Fee

- **rounded**, the vote is successful. For example, there are 4 validator nodes and the votePassRate is 0.7. Then the votes should be larger than or equal to 3 (4\*0.7=2.8). If there are 6 validator nodes and the votePassRate is 0.7. Then the votes should be larger than or equal to 4 (6\*0.7=4.2).

Submitting the transaction fee proposals to the contract by transferring assets or coin.  The input format is json. 

```json
{
  "method":"voteFee",
  "params":{
      "proposalId": "buQft4EdxHrtatWUXjTFD7xAbMXACnUyT8vw1"
   }
}
```

JSON format contents should be transmitted into type string, then fill in the `paycoin`.

```json
{
   "type" : "PAY_COIN",
    "pay_coin" : {
        "dest_address" :"buQiQgRerQM1fUM3GkqUftpNxGzNg2AdJBpe",
        "amount":0,
        "input":"{\"method\":\"voteFee\",\"params\":{\"proposalId\":\"buQft4EdxHrtatWUXjTFD7xAbMXACnUyT8vw1\"}}"
    }
}
```

## Error Code

  Consist of two parts:
- error_code : error code，rough separations of errors  
- error_desc : error description，precise error description, including specific error information 

error list:

| error_code | Names                     | Description                                                  |
| :--------- | ------------------------- | ------------------------------------------------------------ |
| 0          | ERRCODE_SUCCESS           | Successful operation                                         |
| 1          | ERRCODE_INTERNAL_ERROR    | Inner service defect                                         |
| 2          | ERRCODE_INVALID_PARAMETER | Parameters error                                             |
| 3          | ERRCODE_ALREADY_EXIST     | Objects already exist, such as repeated transactions***      |
| 4          | ERRCODE_NOT_EXIST         | Objects do not exist, such as null account, transactions and blocks etc. |
| 5          | ERRCODE_TX_TIMEOUT        | Transactions expired. It means the transaction has been removed from the buffer, but it still has probability to be executed. |
| 7                 | ERRCODE_MATH_OVERFLOW                  | Math calculation is overflown
| 20                | ERRCODE_EXPR_CONDITION_RESULT_FALSE    | The expression returns false. It means the TX failed to be executed currently, but it still has probability to be executed in the following blocks . 
| 21                | ERRCODE_EXPR_CONDITION_SYNTAX_ERROR    | The syntax of the expression returns are false. It means that the TX must fail.                                            |
| 90                | ERRCODE_INVALID_PUBKEY                 | Invalid public key                                                                                      |
| 91                | ERRCODE_INVALID_PRIKEY                 | Invalid private key                                                                                     |
| 92                | ERRCODE_ASSET_INVALID                  | Invalid assets                                                                           |  |  |
| 93                | ERRCODE_INVALID_SIGNATURE              | The weight of the signature does not match the threshold of  the operation.                                                            |
| 94                | ERRCODE_INVALID_ADDRESS                | Invalid address                                                                                     |
| 97                | ERRCODE_MISSING_OPERATIONS             | Absent operation of TX                                                                                |
| 98                | ERRCODE_TOO_MANY_OPERATIONS             | Over 100 operations in a single transaction 
| 99                | ERRCODE_BAD_SEQUENCE                   | Invalid sequence or nonce  of TX |
| 100               | ERRCODE_ACCOUNT_LOW_RESERVE            | Low reserve in the account                                                                                     |
| 101               | ERRCODE_ACCOUNT_SOURCEDEST_EQUAL       | Sender and receiver accounts are the same                                                                             |
| 102               | ERRCODE_ACCOUNT_DEST_EXIST             | The target account already exists                                                                 |
| 103               | ERRCODE_ACCOUNT_NOT_EXIST              | Accounts do not exist***                                                                                  |
| 104               | ERRCODE_ACCOUNT_ASSET_LOW_RESERVE      | Low reserve in the account                                                                           |
| 105               | ERRCODE_ACCOUNT_ASSET_AMOUNT_TOO_LARGE | Amount of assets exceeds the limitation*** ( int64 )资产数量过大，超出了int64的范围                                                              |
| 106               | ERRCODE_ACCOUNT_INIT_LOW_RESERVE       | Insufficient initial reserve for account creation***     创建账号初始化资金不足                                                                       |
| 111               | ERRCODE_FEE_NOT_ENOUGH                 | Low transaction fee                                                                                    |
| 114               | ERRCODE_OUT_OF_TXCACHE                 | TX buffer is full    TX 缓存队列已满                                                                              |
| 120               | ERRCODE_WEIGHT_NOT_VALID               | Invalid weight 权重值不在有效范围内                                                                         |
| 121               | ERRCODE_THRESHOLD_NOT_VALID            | Invalid threshold 门限值不在有效范围内                                                                         |
| 144               | ERRCODE_INVALID_DATAVERSION            | Invalid data version of metadata                                      |
| 146               | ERRCODE_TX_SIZE_TOO_BIG                | TX exceeds upper limitation                                |
| 151               | ERRCODE_CONTRACT_EXECUTE_FAIL          | Failure in contract execution                                                                                 |
| 152               | ERRCODE_CONTRACT_SYNTAX_ERROR          | Failure in syntax analysis                                                                             |
| 153               | ERRCODE_CONTRACT_TOO_MANY_RECURSION    |  The depth of contract recursion exceeds upper limitation                                                                             |
| 154               | ERRCODE_CONTRACT_TOO_MANY_TRANSACTIONS | the TX submitted from the  contract exceeds upper limitation*                                                                           |
| 155               | ERRCODE_CONTRACT_EXECUTE_EXPIRED       | Contract expired                                                          
| 160               | ERRCODE_TX_INSERT_QUEUE_FAIL           | Fail to insert the TX into buffer

## Example

We use address: `buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3`to initiate a transaction with only one operation. The operation is creating an account. 

1. Fill in the transaction contents, then, serialize the transaction.  

    ```text
    POST getTransactionBlob
    ```

    ```json
    {
      "source_address": "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3",
      "nonce": 1,
      "fee_limit":100000,
      "gas_price":1000,
      "operations": [{
          "type": 1,
          "create_account": {
            "dest_address": "buQts6DfT5KavtV94JgZy75H9piwmb7KoUWg",
            "metadatas": [{
                "key": "hello",
                "value": "metadata of the account creation process"
              }
            ],
            "priv": {
              "thresholds": {
                "tx_threshold": 1
              }
            },
            "contract": {
              "payload": "function main(inputStr){\n /*entry functions*/ }"
            }
          }
        }
      ]
    }
    ```
    return,

    ```json
    {
        "error_code": 0,
        "error_desc": "",
        "result": {
            "hash": "8e97ab885685d68b8fa8c7682f77ce17a85f1b4f6c8438eda8ec955890919405",
            "transaction_blob": "0a2e61303032643833343562383964633334613537353734656234393736333566663132356133373939666537376236100122b90108012ab4010a2e61303032663836366337663431356537313934613932363131386363353565346365393939656232396231363461123a123866756e6374696f6e206d61696e28696e707574537472297b0a202f2ae8bf99e698afe59088e7baa6e585a5e58fa3e587bde695b02a2f207d1a06080a1a020807223e0a0568656c6c6f1235e8bf99e698afe5889be5bbbae8b4a6e58fb7e79a84e8bf87e7a88be4b8ade8aebee7bdaee79a84e4b880e4b8aa6d65746164617461"
        }
    }
    ```

2. We use the private key of `buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3`to sign the `transaction_blob. Then we obtain the digital signature. 
   `2f6612eaefbdadbe792201bb5d1e178aff118dfa0a640edb2a8ee91933efb97c4fb7f97be75195e529609a4de9b890b743124970d6bd7072b7029cfe7683ba2d`。

3. Submitting the digital signature, public key, and the transaction blob. 
    ```text
    POST /submitTransaction
    ```

    ```json
    {
      "items" : [{
          "transaction_blob" : "0a2e61303032643833343562383964633334613537353734656234393736333566663132356133373939666537376236100122b90108012ab4010a2e61303032663836366337663431356537313934613932363131386363353565346365393939656232396231363461123a123866756e6374696f6e206d61696e28696e707574537472297b0a202f2ae8bf99e698afe59088e7baa6e585a5e58fa3e587bde695b02a2f207d1a06080a1a020807223e0a0568656c6c6f1235e8bf99e698afe5889be5bbbae8b4a6e58fb7e79a84e8bf87e7a88be4b8ade8aebee7bdaee79a84e4b880e4b8aa6d65746164617461",
          "signatures" : [{
              "sign_data" : "2f6612eaefbdadbe792201bb5d1e178aff118dfa0a640edb2a8ee91933efb97c4fb7f97be75195e529609a4de9b890b743124970d6bd7072b7029cfe7683ba2d",
              "public_key" : "b00204e1c7dddc36d3153adcaa451b0ab525d3def48a0a10fdb492dc3a7263cfb88e80ee974ca4da0e1f322aa84ff9d11340c764ea756ad148e979c121619e9fe52e9054"
            }
          ]
        }
      ]
    }
    ```
