'use strict';
//childchain start id
const originChildChainid = 1;
//define the reward of the submitter
const block_reward = '10';
//define abolishValidator effectiveVoteInterval
const effectiveVoteInterval  = 15 * 24 * 60 * 60 * 1000 * 1000;
//define max validators number size
const validatorSetSize       = 100;
//define vote passrate
const passRate               = 0.7;

function findI0(arr, key){
    assert((typeof arr === 'object') && (typeof key === 'string'), 'Args type error. arg-arr must be an object, and arg-key must be a string.');

    let i = 0;
    while(i < arr.length){
        if(arr[i][0] === key){
            break;
        }
        i += 1;
    }

    if(i !== arr.length){
        return i;
    }
    else{
        return false;
    }
}


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

function checkchildChainValadator(chain_id,validator){
    let key = 'childChainid_info_' + chain_id;
    let chaininfo = JSON.parse(storageLoad(key));
    if(chaininfo !== false)
    {
        if(findI0(chaininfo.validators,validator) !== false)
        {
            return true;
        }
    }
    return false;
}

function submitChildBlockHeader(params){
    log('submitChildBlockHeader');
    let input = params;
    assert(checkchildChainValadator(input.chain_id,sender) === true,'submitChildBlockHeader sender is not validator.' );
    let info = JSON.parse(storageLoad('childChainid_info_' + input.chain_id));
    assert(info !== false, 'childChainid_info_' + input.chain_id + ' failed.');
    assert((input.block_header.seq - info.blockheight) === 1, 'input.block_header.seq is not correct blockheight.');
    let blockheight = 0;
    if(info.blockheight === 0)
    {
        assert(input.block_header.seq === 1,'input.block_header.seq is not correct.');
        blockheight = input.block_header.seq;
        info.blockheight = blockheight;
        info.lastblockhash = input.block_header.hash;
        input.sumitter = sender;
        input.currentheight = blockheight;
        storageStore('childChainBlock_' + info.chain_id + '_' + input.block_header.hash, JSON.stringify(input));
        storageStore('childChainid_info_'+ info.chain_id,JSON.stringify(info));
    }
    else
    {
        let preblock = JSON.parse(storageLoad('childChainBlock_' + info.chain_id + '_' + input.block_header.previous_hash));
        assert(preblock !== false,'preblockhash is not exist.');
        assert((input.block_header.seq - preblock.block_header.seq) === 1,'input blockheader.seq is not correct');
        blockheight = input.block_header.seq;
        info.blockheight = blockheight;
        info.lastblockhash = input.block_header.hash;
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
    assert(thisPayCoinAmount<=0, 'BU amount deposited is less than or equal to 0');
    let childChain_id = parseInt(storageLoad('childChainCount'));
    assert((input.chain_id>childChain_id)||(input.chain_id<=0) , 'chain_id is error');
    let Validator_size =  getchildChainValidators(input.chain_id).validators.length;
    assert(Validator_size<=0, 'child chain node not exist.');
    assert(int64Compare(thisPayCoinAmount,input.amount) === 0,'amount is not equels thisPayCoinAmount');
    let assertinfo = JSON.parse(storageLoad('childChainAsset_' + input.chain_id));
    if(assertinfo === false) {
        let assertparam = {};
        assertparam.chain_id = input.chain_id;
        assertparam.totalamount = input.amount;
        assertparam.seq = 1;

        let asset_chanin = {};
        asset_chanin.chain_id = input.chain_id;
        asset_chanin.amount = input.amount;
        asset_chanin.seq = 1;
        asset_chanin.block_number = blockNumber;
        asset_chanin.source_address = sender;
        asset_chanin.address = input.address;

        storageStore('childChainAsset_' + assertparam.chain_id , JSON.stringify(assertparam));
        storageStore('childChainAsset_' + asset_chanin.chain_id+ asset_chanin.seq, JSON.stringify(asset_chanin));
        tlog('deposit',input.chain_id,JSON.stringify(asset_chanin)); 
    } 
    else {
        let asset_chanin = JSON.parse(storageLoad('childChainAsset_' + input.chain_id+assertinfo.seq));
        let totleaamount = assertinfo.totalamount + input.amount;
        assertinfo.totalamount = totleaamount;
        assertinfo.seq = assertinfo.seq + 1;
        asset_chanin.chain_id = input.chain_id;
        asset_chanin.amount = input.amount;
        asset_chanin.seq = assertinfo.seq;
        asset_chanin.block_number = blockNumber;
        asset_chanin.source_address = sender;
        asset_chanin.address = input.address;
        storageStore('childChainAsset_' + assertparam.chain_id , JSON.stringify(assertinfo));
        storageStore('childChainAsset_' + asset_chanin.chain_id+ asset_chanin.seq, JSON.stringify(asset_chanin));
        tlog('deposit',input.chain_id,JSON.stringify(asset_chanin)); 
    }
}

function abolishValidator(params){
    log('abolishValidator');
    let input = params;
    assert(addressCheck(input.address) === true, 'Arg-address is not valid adress.');
    assert(checkchildChainValadator(input.chain_id,sender) === true,'abolishValidator sender is not validator.' );
    let key = "childChainAbolish_" + input.chain_id + "_" + input.address;
    let abolishinfo = JSON.parse(storageLoad(key));
    if(abolishinfo === false)
    {
        input.abolishcout = 1;
        input.starttime = blockTimestamp;
        input.votes = [sender];
        storageLoad(key,JSON.stringify(input));
        tlog('abolishValidator',input.chain_id,JSON.stringify(input));
    }
    else
    {
        if(abolishinfo.starttime + effectiveVoteInterval < blockTimestamp)
        {
            log('Update expired time of abolishing validator(' + input.address + ').');
            input.starttime = blockTimestamp;
            input.abolishcout = 1;
            input.starttime = blockTimestamp;
            input.votes = [sender];
            storageLoad(key,JSON.stringify(input));
            tlog('abolishValidator',input.chain_id,JSON.stringify(input));
        }
        else
        {
            log('Already abolished validator(' + input.address + ').'); 
        }
    }
}

function getchildChainValidators(chain_id){
    let key = 'childChainid_info_' + chain_id;
    let chaininfo = JSON.parse(storageLoad(key));

    let retinfo = [];
    if(chaininfo !== false){
        retinfo = chaininfo.validators;
    }
    return retinfo;
}

function removechildChainValidator(chain_id,address){
    let key = 'childChainid_info_' + chain_id;
    let chaininfo = JSON.parse(storageLoad(key));
    if(chaininfo !== false){
        let postion = findI0(chaininfo.validators,address);
        chaininfo.validators.spice(postion,1);
        storageStore(key,JSON.stringify(chaininfo));
    }
}

function voteForAbolish(params){
    log('voteForAbolish');
    let input = params;
    assert(addressCheck(input.address) === true, 'Arg-address is not valid adress.');
    assert(checkchildChainValadator(input.chain_id,sender) === true,'voteForAbolish sender is not validator.' );
    assert(checkchildChainValadator(input.chain_id,input.address) === true,'voteForAbolish input.address is not validator.' );
    let key = "childChainAbolish_" + input.chain_id + "_" + input.address;
    let abolishinfo = JSON.parse(storageLoad(key));
    assert(abolishinfo !== false,'abolishinfo is not exist.');
    if(blockTimestamp > abolishinfo.starttime + effectiveVoteInterval)
    {
        log('Voting time expired, ' + input.address + ' is still validator.'); 
        storageDel(key);
    }
    else
    {
        assert(abolishinfo.votes.includes(sender) !== true, sender + ' has voted.');
        abolishinfo.votes.push(sender);
        input.abolishcout = int64Add(input.abolishcout,1);
        let childchain_validators = getchildChainValidators(input.chain_id);
        if(input.abolishcout < parseInt(childchain_validators.length * passRate + 0.5))
        {
            storageLoad(key,JSON.stringify(input));
            return true;4 
        }
        tlog('voteForAbolish',input.chain_id,JSON.stringify(abolishinfo));
        removechildChainValidator(input.chain_id,input.address);
        storageDel(key);
    }
}

function queryChildBlockHeader(params){
    log('queryChildBlockHeader');
    let input = params;
    let key = '';
    if(input.header_hash === ''){
        let info = JSON.parse(storageLoad('childChainid_info_' + input.chain_id));
        assert(info !== false, 'queryChildBlockHeader childChainid_info_' + input.chain_id + ' failed.');
        key = 'childChainBlock_' + input.chain_id + '_' + info.lastblockhash;
    } else {
        key = 'childChainBlock_' + input.chain_id + '_' + input.header_hash;
    }
    let blockinfo = JSON.parse(storageLoad(key));
    
    let retinfo = {};
    if(blockinfo === false){
        let blockret = {};
        blockret.chain_id = input.chain_id;
        blockret.seq = 0;
        blockret.hash = '';
        blockret.previous_hash = '';
        blockret.account_tree_hash = '';
        blockret.close_time = '';
        blockret.consensus_value_hash = '';
        blockret.version = '';
        blockret.tx_count = 0;
        blockret.validators_hash = [];
        blockret.fees_hash = '';
        blockret.reserve = '';
        retinfo = blockret;
    } else {
        retinfo = blockinfo.block_header;
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
    else if(input.method === 'abolishValidator'){
        abolishValidator(input.params);
    }
    else if(input.method === 'voteForAbolish'){
        voteForAbolish(input.params);
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
