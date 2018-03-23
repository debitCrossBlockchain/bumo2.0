var http = require('http');
var querystring = require('querystring');

var g_host='127.0.0.1';
var g_port = 36002;
var g_address = "buQs9npaCq9mNFZG18qu88ZcmXYqd6bqpTU3";
var g_priv_key = "privbvYfqQyG3kZyHE4RX4TYVa32htw8xG4WdpCTrymPUJQ923XkKVbM";
var g_dest = "";
var g_nonce= 0;
var global = 0;
var g_contract_address = "buQXy5nUbss4feUGS2fri19pZus3YMLZbU7w";

 //setInterval(function(){
	 GetAccountNonce();
// }, 10000);

function GetAccountNonce(){
	var http = require('http');
  var querystring = require('querystring');
  var url = '/getAccount?address='+ g_address; 
  var options = {
    host : g_host,
    port : g_port,
    path : url,
    method : 'GET',
    headers : {
      'Content-Type' : 'application/json'
    }
  };
  http.get(options, function (res) {
    var resData = "";
    res.on("data", function (data) {
      resData += data;
    });
    
    res.on("end", function () {
     g_nonce = JSON.parse(resData).result.nonce == undefined ? 0 : JSON.parse(resData).result.nonce;
     console.log(g_nonce);
	  
		f1();
 
    });
  })
}
 
function f1(){
  var http = require('http');
  var querystring = require('querystring');
  var options = {
    host : g_host,
    port : g_port,
    path : '/createAccount',
    method : 'GET',
    headers : {
      'Content-Type' : 'application/json'
    }
  };
  http.get(options, function (res) {
    var resData = "";
    res.on("data", function (data) {
      resData += data;
    });
    res.on("end", function () {
      var address = JSON.parse(resData).result.address;
      
      setInterval(function(){
      createNonStopContract(address, "'use strict';function main(input) 1{let i = 0;while(true){i=i+1;}return input;}\r\nfunction init(){return;}");
      }, 100);
	 //createNonStopContract(address, "function main(input) { callBackGetLedgerInfo(input); callBackLog('1'+'2'+'2',11);callBackLog(2.5);callBackLog('2');callBackLog({});callBackLog(transaction);var transaction = {};if (callBackGetAccountMetaData(thisAddress, 'status')) { callBackLog('not found'); } }");
	  //createNonStopContract(address, source_file);
	  //createNonStopContract(address, "function main(input) { var a = \"123333333333333333333333333333333321312312331sfdsffffffffffffffffffffffffffffffffffffffffffffffffqweqwr1231231233333333333333333333333333333333333333333333333333333333fksahfdk\";throw input;}");
    });
  })
}

function createNormal(address) {
	console.log(address);
  var txItems = {
    "items" : [{
        "transaction_json" : {
          "source_address" : g_address,
          "nonce" : ++g_nonce,
          "expr_condition" : "account(\"a0025e6de5a793da4b5b00715b7774916c06e9a72b7c18\")",
          "operations" : [{
              "type" : "CREATE_ACCOUNT",
              "create_account" : {
                "dest_address" : g_contract_address,
							"priv":{
								"master_weight" :1,
								"thresholds":{
									"tx_threshold":1
									}
								}
              }
            }
          ]
        },
        "private_keys" : [
         g_priv_key
        ]
      }
    ]
  };

  var content = JSON.stringify(txItems);

  var options = {
    host : g_host,
    port : g_port,
    path : '/submitTransaction',
    method : 'POST',
    headers : {
      'Content-Type' : 'application/x-www-form-urlencoded',
      'Content-Length' : content.length
    }
  };

	var ret ="";
  var req = http.request(options, function (res) {
      res.on('data', function (data) {
        ret += data;
      });
      res.on('error', function (err) {
        console.log(err);
      });
      res.on("end", function () {
  			console.log(ret);
    });
    });

  req.write(content);
  req.end();
  console.log(content);
}

function createContract(address) {
	console.log(address);

  var txItems = {
    "items" : [{
        "transaction_json" : {
          "source_address" : g_address,
          "nonce" : ++g_nonce,
          "fee" : 40000,
          "ceil_ledger_seq" : 5,
          "operations" : [{
              "type" : "CREATE_ACCOUNT",
              "create_account" : {
                "dest_address" : g_contract_address,
			          "contract" : 
					     {
					       "payload" : "function main(input) {  var a = callBackGetAccountInfo('a0025e6de5a793da4b5b00715b7774916c06e9a72b7c18');  if (a) callBackLog(a);  var b = callBackGetAccountMetaData('a0025e6de5a793da4b5b00715b7774916c06e9a72b7c18', input['key']);  if (b) callBackLog(b);  var tx = { 'operations' : [{ 'type' : 'ISSUE_ASSET', 'source_address' : 'a0025e6de5a793da4b5b00715b7774916c06e9a72b7c18', 'issue_asset' : {  'code' : 'cny',  'amount' : input.amount }  } ]  };  callBackDoOperation(tx); } "
					     },
					     "metadatas": [{
			            "key": " 111",
			            "value": "hello 111!",
			            "version": 0
			          }, {
			            "key": "=222",
			            "value": "hello 222!",
			            "version": 0
			          }
			        ],
							"priv":{
								"master_weight" :1,
								"thresholds":{
									"tx_threshold":1
									}
								}
              }
            }
          ]
        },
        "private_keys" : [
          g_priv_key
        ]
      }
    ]
  };

  var content = JSON.stringify(txItems);

  var options = {
    host : g_host,
    port : g_port,
    path : '/submitTransaction',
    method : 'POST',
    headers : {
      'Content-Type' : 'application/x-www-form-urlencoded',
      'Content-Length' : content.length
    }
  };

	var ret ="";
  var req = http.request(options, function (res) {
      res.on('data', function (data) {
        ret += data;
      });
      res.on('error', function (err) {
        console.log(err);
      });
      res.on("end", function () {
  			console.log(ret);
    });
    });

  req.write(content);
  req.end();
  console.log(content);
}

function createNonStopContract(address, payload) {
	console.log('Create new address : '+address);
	console.log('Contract payload :'+payload);

  var txItems = {
    "items" : [{
        "transaction_json" : {
          "source_address" : g_address,
          "nonce" : ++g_nonce,
          "fee" : 1000000,
          "ceil_ledger_seq" : 1520000,
          "operations" : [{
              "type" : "CREATE_ACCOUNT",
          		"expr_condition" : "",
          	  "create_account" : {
          	  	"init_balance" : 100000000,
                "dest_address" : address,
			          "contract" : 
					     {
					       "payload" : payload
					     },
					     "metadatas": [{
			            "key": " 111",
			            "value": "hello 111!",
			            "version": 0
			          }, {
			            "key": "=222",
			            "value": "hello 222!",
			            "version": 0
			          }
			        ],
							"priv":{
								"thresholds":{
									"tx_threshold":1
									}
								}
              }
            }
          ]
        },
        "private_keys" : [
         g_priv_key
        ]
      }
    ]
  };

  var content = JSON.stringify(txItems);

  var options = {
    host : g_host,
    port : g_port,
    path : '/submitTransaction',
    method : 'POST',
    headers : {
      'Content-Type' : 'application/x-www-form-urlencoded',
      'Content-Length' : content.length
    }
  };

	var ret ="";
  var req = http.request(options, function (res) {
      res.on('data', function (data) {
        ret += data;
      });
      res.on('error', function (err) {
        console.log(err);
      });
      res.on("end", function () {
  			console.log(ret);
    });
    });

  req.write(content);
  req.end();
  console.log(content);
}

