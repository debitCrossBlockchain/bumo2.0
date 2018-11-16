'use strict';
//childchain start id
const originChildChainid = 1;
//define the reward of the submitter
const block_reward = '10';


function createChildChain(params){
    log('createChildChain');
    let input = params;
    assert(addressCheck(input.genesis_account) === true, 'genesis_account is not valid adress.');
    assert(sender === input.genesis_account, 'sender is not genesis_account.');
    assert(int64Compare(thisPayCoinAmount,input.cost) === 0,'cost is not equels thisPayCoinAmount ' + thisPayCoinAmount + ',' + input.cost);
    let childChainCount = parseInt(storageLoad('childChainCount'));
    let childChainid = originChildChainid + childChainCount;

    let info_params = {};
    info_params.chain_id = childChainid;
    info_params.chain_creator = sender;
    info_params.cost = input.cost;
    info_params.block_cost_ready = true;
    info_params.blockheight = 0;
    info_params.validators = input.reserve_validator;
    storageStore('childChainid_info_'+ childChainid,JSON.stringify(info_params));
    params.chain_id = childChainid;
    storageStore('childChainid_' + childChainid,JSON.stringify(params));
    tlog('createChildChain',childChainid,JSON.stringify(params)); 
    childChainCount = int64Add(childChainCount, 1);
    storageStore('childChainCount',childChainCount.toString());
}

function payCost(params){
    log('payCost');
    let input = params;
    assert(addressCheck(input.chain_creator) === true, 'chain_creator is not valid adress.');
    assert(sender === input.chain_creator, 'sender is not chain_creator.');
    assert(int64Compare(thisPayCoinAmount,input.cost) === 0,'cost is not equels thisPayCoinAmount ' + thisPayCoinAmount + ',' + input.cost);
    let info = JSON.parse(storageLoad('childChainid_info_' + input.chain_id));
    assert(info !== false, 'payCost childChainid_info_' + input.chain_id + ' failed.');
    assert(info.chain_creator === sender ,'payCost sender is not equels info chain_creator.');
    let totalcost = int64Add(info.cost, input.cost);
    info.cost = totalcost;
    storageStore('childChainid_info_'+ input.chain_id,JSON.stringify(info));
}

function transferCoin(dest, amount)
{
    assert((typeof dest === 'string') && (typeof amount === 'string'), 'Args type error. arg-dest and arg-amount must be a string,' + typeof dest + ',' + typeof amount);
    if(amount === '0'){ return true; }

    payCoin(dest, amount);
    log('Pay coin( ' + amount + ') to dest account(' + dest + ') succeed.');
}

function checkchildChainValadator(validator){
    return true;
}

function submitChildBlockHeader(params){
    log('submitChildBlockHeader');
    let input = params;
    assert(checkchildChainValadator(sender) === true,'submitChildBlockHeader sender is not validator.' );
    let info = JSON.parse(storageLoad('childChainid_info_' + input.chain_id));
    assert(info !== false, 'childChainid_info_' + input.chain_id + ' failed.');
    let blockheight = 0;
    if(info.blockheight === 0)
    {
        blockheight = int64Add(info.blockheight, 1);
        info.blockheight = blockheight;
        input.sumitter = sender;
        input.currentheight = blockheight;
        storageStore('childChainBlock_' + info.chain_id + '_' + input.block_header.hash, JSON.stringify(input));
        storageStore('childChainid_info_'+ info.chain_id,JSON.stringify(info));
    }
    else
    {
        let preblock = storageLoad('childChainBlock_' + info.chain_id + '_' + input.block_header.previous_hash);
        assert(preblock !== false,'preblockhash is not exist.');
        blockheight = int64Add(info.blockheight, 1);
        info.blockheight = blockheight;
        input.sumitter = sender;
        input.currentheight = blockheight;
        storageStore('childChainBlock_' + info.chain_id + '_' + input.block_header.hash, JSON.stringify(input));
        storageStore('childChainid_info_'+ info.chain_id,JSON.stringify(info));
    }
    transferCoin(sender,block_reward);
}

function depositToChildChain(params){
    log('depositToChildChain');
    let input = params;
    assert(sender === input.address, 'sender is not input_address.');
    assert(int64Compare(thisPayCoinAmount,input.amount) === 0,'amount is not equels thisPayCoinAmount');
    let assertinfo = JSON.parse(storageLoad('childChainAsset_' + input.chain_id));
    if(assertinfo === false) {
        let assertparam = {};
        assertparam.chain_id = input.chain_id;
        assertparam.totalamount = input.amount;
        storageStore('childChainAsset_' + assertparam.chain_id , JSON.stringify(assertparam));
    } 
    else {
        let totleaamount = assertinfo.totalamount + input.amount;
        assertinfo.totalamount = totleaamount;
        storageStore('childChainAsset_' + assertinfo.chain_id , JSON.stringify(assertinfo));
    }
}

function queryChildBlockHeader(params){
    log('queryChildBlockHeader');
    let input = params;
    let key = 'childChainBlock_' + input.chain_id + '_' + input.header_hash;
    let blockinfo = JSON.parse(storageLoad(key));
    
    let retinfo = {};
    if(blockinfo === false){
        retinfo = 'queryChildBlockHeader failed.';
    } else {
        retinfo = JSON.stringify(blockinfo.block_header);
    }
    return retinfo;
}

function queryChildChainInfo(params){
    log('queryChildChainInfo');
    let key = 'childChainid_' + params.chain_id;
    let genesisinfo = JSON.parse(storageLoad(key));

    let retinfo = {};
    if(genesisinfo === false){
        retinfo = 'queryChildChainInfo failed,' + key;
    } else {
        retinfo = genesisinfo;
    }
    return retinfo;
}

function queryChildChainValidators(params){
    log('queryChildChainValidators');
    let key = 'childChainid_info_' + params.chain_id;
    let chaininfo = JSON.parse(storageLoad(key));

    let retinfo = {};
    if(chaininfo === false){
        retinfo = 'queryChildChainInfo failed.';
    } else {
        retinfo.validators = chaininfo.validators;
    }
    return retinfo;
}

function init(inputStr){
    log('init');
    storageStore('childChainCount', '0');
}

function main(inputStr){
    let input = JSON.parse(inputStr);
    let result = {};
    if(input.method === 'createChildChain'){
        createChildChain(input.params);
    }
    else if(input.method === 'payCost'){
        payCost(input.params);
    }
    else if(input.method === 'depositToChildChain'){
        depositToChildChain(input.params);
    }
    else if(input.method === 'submitChildBlockHeader'){
        submitChildBlockHeader(input.params);
    }
    else{
        throw '<Main interface passes an invalid operation type>';
    }
}

function query(inputStr){
    let input  = JSON.parse(inputStr);
    let result = {};
    if(input.method === 'queryChildBlockHeader'){
        result = queryChildBlockHeader(input.params);
    }
    else if(input.method === 'queryChildChainInfo'){
        result = queryChildChainInfo(input.params);
    }
    else if(input.method === 'queryChildChainValidators'){
        result = queryChildChainValidators(input.params);
    }
    else{
        throw '<Query interface passes an invalid operation type>';
    }

    log(result);
    return JSON.stringify(result);
}
