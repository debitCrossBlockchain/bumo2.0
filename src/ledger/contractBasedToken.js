'use strict';

let globalCfg = {};

function globalCfgKey(){
    return 'globalConfig';
}

function getGlobalCfgFromMetadata(){
    let value = storageLoad(globalCfgKey());
    assert(value !== false, 'get globalCfg from metadata failed.');
    globalCfg = JSON.parse(value);
}

function makeBalanceKey(address){
    return 'balance_' + address;
}
function makeAllowanceKey(owner, spender){
    return 'allow_' + owner + '_to_' + spender;
}

function approve(spender, value){
    assert(addressCheck(spender) === true, 'Arg-spender is not valid adress.');
    assert(typeof value === 'string', 'Arg-value must be string type.');

    let key = makeAllowanceKey(sender, spender);
    storageStore(key, value);

    tlog('approve', sender + ' approve ' + spender + ' ' + value + ' succeed.');

    return true;
}

function allowance(owner, spender){
    assert(addressCheck(owner) === true, 'Arg-owner is not valid adress.');
    assert(addressCheck(spender) === true, 'Arg-spender is not valid adress.');

    let key = makeAllowanceKey(owner, spender);
    let value = storageLoad(key);
    assert(value !== false, 'Get allowance ' + owner + ' to ' + spender + ' from metadata failed.');

    return value;
}

function transfer(to, value){
    assert(addressCheck(to) === true, 'Arg-to is not valid adress.');
    assert(typeof value === 'string', 'Arg-value must be string type.');

    let senderKey = makeBalanceKey(sender);
    let senderValue = storageLoad(senderKey);
    assert(senderValue !== false, 'Get balance of ' + sender + ' from metadata failed.');
    assert(senderValue >= value, 'Balance:' + senderValue + ' of sender:' + sender + ' < transfer value:' + value + '.');

    let toKey = makeBalanceKey(to);
    let toValue = storageLoad(toKey);
    toValue = (toValue === false) ? value : int64Plus(toValue, value); 
    storageStore(toKey, toValue);

    senderValue = int64Sub(senderValue, value);
    storageStore(senderKey, senderValue);

    tlog('transfer', sender + ' transfer ' + value + ' to ' + to + ' succeed.');

    return true;
}

function transferFrom(from, to, value){
    assert(addressCheck(from) === true, 'Arg-from is not valid adress.');
    assert(addressCheck(to) === true, 'Arg-to is not valid adress.');
    assert(typeof value === 'string', 'Arg-value must be string type.');

    let fromKey = makeBalanceKey(from);
    let fromValue = storageLoad(fromKey);
    assert(fromValue !== false, 'Get value failed, maybe ' + from + ' has no value.');
    assert(fromValue >= value, from + ' balance:' + fromValue + ' < transfer value:' + value + '.');

    let allowValue = allowance(from, to);
    assert(allowValue >= value, 'Allowance value:' + allowValue + ' < transfer value:' + value + ' from ' + from + ' to ' + to  + '.');

    let toKey = makeBalanceKey(to);
    let toValue = storageLoad(toKey);
    toValue = (toValue === false) ? value : int64Plus(toValue, value); 
    storageStore(toKey, toValue);

    fromValue = int64Sub(fromValue, value);
    storageStore(fromKey, fromValue);

    let allowKey = makeAllowanceKey(from, to);
    allowValue   = int64Sub(allowValue, value);
    storageStore(allowKey, allowValue);

    tlog('transferFrom', sender + ' triggering ' + from + ' transfer ' + value + ' to ' + to + ' succeed.');

    return true;
}

function transferContract(address){
    assert(addressCheck(address) === true, 'Arg-address is not valid adress.');

    getGlobalCfgFromMetadata();
    assert(sender === globalCfg.contractOwner, sender + ' has no permission modify contract ownership.');

    globalCfg.contractOwner = address;
    let cfgStr = JSON.stringify(globalCfg);
    storageStore(globalCfgKey(), cfgStr);

    tlog('transferContract', sender + ' transfer contract ownership to ' + address + ' succeed.');
}

function name() {
    return globalCfg.name;
}

function symbol(){
    return globalCfg.symbol;
}

function decimals(){
    return globalCfg.decimals;
}

function totalSupply(){
    return globalCfg.totalSupply;
}

function contractInfo(){
    return globalCfg;
}

function balanceOf(address){
    assert(addressCheck(address) === true, 'Arg-address is not valid address.');

    let key = makeBalanceKey(address);
    let value = storageLoad(key);
    assert(value !== false, 'Get balance of ' + address + ' from metadata failed.');

    return value;
}

function init(input_str){
    assert(input_str !== undefined, 'Arg-input_str is undefined.');
    let input = JSON.parse(input_str);

    assert(addressCheck(input.params.contractOwner) === true &&
           typeof input.params.name === 'string' &&
           typeof input.params.symbol === 'string' &&
           typeof input.params.decimals === 'number' &&
           typeof input.params.totalSupply === 'string',
           'Args check failed.');

    globalCfg.name = input.params.name;
    globalCfg.symbol = input.params.symbol;
    globalCfg.decimals = input.params.decimals;
    globalCfg.totalSupply = int64Mul(input.params.totalSupply, 10 ** globalCfg.decimals);
    globalCfg.contractOwner = input.params.contractOwner;

    storageStore(globalCfgKey(), JSON.stringify(globalCfg));
    storageStore(makeBalanceKey(globalCfg.contractOwner), globalCfg.totalSupply);
}

function main(input_str){
    let input = JSON.parse(input_str);

    if(input.method === 'transfer'){
        transfer(input.params.to, input.params.value);
    }
    else if(input.method === 'transferFrom'){
        transferFrom(input.params.from, input.params.to, input.params.value);
    }
    else if(input.method === 'approve'){
	    approve(input.params.spender, input.params.value);
    }
    else if(input.method === 'transferContract'){
	    transferContract(input.params.address);
    }
    else{
        throw '<undidentified operation type>';
    }
}

function query(input_str){
    getGlobalCfgFromMetadata();

    let result = {};
    let input  = JSON.parse(input_str);
    if(input.method === 'name'){
        result.name = name();
    }
    else if(input.method === 'symbol'){
        result.symbol = symbol();
    }
    else if(input.method === 'decimals'){
        result.decimals = decimals();
    }
    else if(input.method === 'totalSupply'){
        result.totalSupply = totalSupply();
    }
    else if(input.method === 'contractInfo'){
        result.contractInfo = contractInfo();
    }
    else if(input.method === 'balanceOf'){
        result.balance = balanceOf(input.params.address);
    }
    else if(input.method === 'allowance'){
        result.allowance = allowance(input.params.owner, input.params.spender);
    }
    else{
       	throw '<unidentified operation type>';
    }

    log(result);
    return JSON.stringify(result);
}
