'use strict';

let name_ = '';
let symbol_ = '';
let decimals_ = 0;
let totalSupply_ = '';
const ownBalance_ = 'own_balance';

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

    return true;
}

function name() {
    return name_;
}

function symbol(){
    return symbol_;
}

function decimals(){
    return decimals_;
}

function totalSupply(){
    return totalSupply_;
}

function balanceOf(address){
    assert(addressCheck(address) === true, 'Arg-address is not valid address.');

    let key = makeBalanceKey(address);
    let value = storageLoad(key);
    assert(value !== false, 'Get balance of ' + address + ' from metadata failed.');

    return value;
}

function contractInfo(){
    let result = {};

    result.name = name_;
    result.symbol = symbol_;
    result.decimals = decimals_;
    result.totalSupply = totalSupply_;

    return result;
}

function init(input_str){
    assert(input_str !== undefined, 'Arg-input_str is undefined.');
    let input = JSON.parse(input_str);

    name_ = input.params.name;
    symbol_ = input.params.symbol;
    decimals_ = parseInt(input.params.decimals);

    let unit = 10 ** decimals_;
    let entire = int64Mul(input.params.totalSupply, unit);
    totalSupply_ = entire;

    storageStore(ownBalance_, totalSupply_);
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
    else{
        throw '<undidentified operation type>';
    }
}

function query(input_str){
    let input  = JSON.parse(input_str);

    let result = {};
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
        result.balanceOf = balanceOf(input.params.address);
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
