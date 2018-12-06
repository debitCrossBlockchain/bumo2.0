'use strict';
//childchain start id
const originChildChainid = 1;
//define the reward of the submitter
const BLOCK_REWARD = '10';
const CONST_COIN_BU = '100000';
const CONST_PERIOD_BLOCK_NUMS = '100';

const CHILD_CHAIN_ID_INFO = 'childChainid_info_';
const CHILD_CHAIN_ID = 'childChainid_';
const CHILD_CHAIN_COUNT = 'childChainCount';
const CHILD_CHAIN_BLOCK = 'childChainBlock_';
const CHILD_CHAIN_ASSET = 'childChainAsset_';
const CHILD_CHAIN_VALIDATOR_HISTORY = 'childChainValidatorHistory_';

const TLOG_CREATE_CHILD_CHAIN = 'createChildChain';
const TLOG_DEPOSIT = 'deposit';
const TLOG_WITHDRAWAL = 'withdrawal';
const TLOG_CHANGE_VALIDATOR = 'changeValidator';

//define  effectiWithdrawalInterval
const effectiWithdrawalInterval = 15 * 24 * 60 * 6;

function findValidatorIndex(array, key){
    let i = 0;
    for(i = 0; i < array.length; i += 1){
        if(array[i][0] === key) { return i; }
    }
    return false;
}

function createChildChain(params){
    let input = params;
    assert(addressCheck(input.genesis_account) === true, 'genesis_account is not valid adress.');
    assert(sender === input.genesis_account, 'sender is not genesis_account.');
    assert(int64Compare(thisPayCoinAmount,input.cost) === 0,'cost is not equels thisPayCoinAmount ' + thisPayCoinAmount + ',' + input.cost);
    assert(input.reserve_validator.length > 0, 'Failed to check reserve validator size, must gt 0');
    
    let saveValidators = [];
    let i = 0;
    for(i = 0; i < input.reserve_validator.length; i += 1){
        assert(addressCheck(input.reserve_validator[i]) === true, 'Failed to check amount reserve validator address.');
        saveValidators.push([input.reserve_validator[i], 0]);
    }
    let childChainCount = parseInt(storageLoad(CHILD_CHAIN_COUNT));
    let childChainid = originChildChainid + childChainCount;

    let info_params = {};
    info_params.chain_id = childChainid;
    info_params.chain_creator = sender;
    info_params.cost = input.cost;
    info_params.block_cost_ready = true;
    info_params.blockheight = 0;
    info_params.validatorHistoryIndex = '0';
    info_params.reserve_validators = saveValidators;
    info_params.validators = saveValidators;
    info_params.bufferValidators = [];
    storageStore(CHILD_CHAIN_ID_INFO + childChainid, JSON.stringify(info_params));
    params.chain_id = childChainid;
    storageStore(CHILD_CHAIN_ID + childChainid, JSON.stringify(params));
    tlog(TLOG_CREATE_CHILD_CHAIN, childChainid, JSON.stringify(params)); 
    childChainCount = int64Add(childChainCount, 1);
    storageStore(CHILD_CHAIN_COUNT, childChainCount.toString());
}

function payCost(params){
    log('payCost');
    let input = params;
    assert(addressCheck(input.chain_creator) === true, 'chain_creator is not valid adress.');
    assert(sender === input.chain_creator, 'sender is not chain_creator.');
    assert(int64Compare(thisPayCoinAmount,input.cost) === 0,'cost is not equels thisPayCoinAmount ' + thisPayCoinAmount + ',' + input.cost);
    let info = JSON.parse(storageLoad(CHILD_CHAIN_ID_INFO + input.chain_id));
    assert(info !== false, 'payCost childChainid_info_' + input.chain_id + ' failed.');
    assert(info.chain_creator === sender ,'payCost sender is not equels info chain_creator.');
    let totalcost = int64Add(info.cost, input.cost);
    info.cost = totalcost;
    storageStore(CHILD_CHAIN_ID_INFO + input.chain_id, JSON.stringify(info));
}

function transferCoin(dest, amount){
    assert((typeof dest === 'string') && (typeof amount === 'string'), 'Args type error. arg-dest and arg-amount must be a string,' + typeof dest + ',' + typeof amount);
    if(amount === '0'){ return true;}

    payCoin(dest, amount);
    log('Pay coin( ' + amount + ') to dest account(' + dest + ') succeed.');
}

function checkchildChainValadator(chain_id, validator){
    let key = CHILD_CHAIN_ID_INFO + chain_id;
    let chaininfoSrc = storageLoad(key);
    if(chaininfoSrc === false){
        return false;
    }

    if(findValidatorIndex(JSON.parse(chaininfoSrc).validators, validator) === false) {
        return false;
    }
    return true;
}

function getchildChainValidators(chain_id){
    let key = CHILD_CHAIN_ID_INFO + chain_id;
    let chaininfo = JSON.parse(storageLoad(key));

    let retinfo = [];
    if(chaininfo !== false){
        retinfo = chaininfo.validators;
    }
    return retinfo;
}

function withdrawCoinOnTimer(chainObj){
    //1.If there's a buffer validators and it's out of time
    let i = 0;
    let finded_index = -1;
    for(i = 0; i < chainObj.bufferValidators.length; i += 1){
        let gap_nums = int64Sub(blockNumber, chainObj.bufferValidators[i][2]);
        if(int64Compare(gap_nums, CONST_PERIOD_BLOCK_NUMS) > 1){
            finded_index = i;
            break;
        }
    }
    if(finded_index === -1){
        return;
    }

    //2.Refund and delete record
    payCoin(sender, chainObj.bufferValidators[finded_index][1]);
    chainObj.bufferValidators.splice(finded_index, 1);
    storageStore(CHILD_CHAIN_ID_INFO + chainObj.chain_id, JSON.stringify(chainObj)); 
}

function checkWithdrawal(params) {
    log('checkWithdrawal');
    let input = params;
    let retinfo = JSON.parse(storageLoad('withdrawal_' + input.chain_id));
    if (retinfo === false) {
        return;
    }

    let withdrawal = JSON.parse(storageLoad('withdrawal_' + input.chain_id + (retinfo.complete_seq + 1)));
    if (blockNumber < withdrawal.withdrawal_block_number) {
        return;
    }

    let totleaamount = retinfo.totalamount;
    retinfo.totalamount = totleaamount;
    retinfo.seq = retinfo.seq;
    retinfo.complete_seq = retinfo.complete_seq + 1;
    withdrawal.chain_id = withdrawal.chain_id;
    withdrawal.amount = withdrawal.amount;
    withdrawal.seq = withdrawal.seq;
    withdrawal.block_hash = withdrawal.block_hash;
    withdrawal.main_source_address = withdrawal.main_source_address;
    withdrawal.source_address = withdrawal.source_address;
    withdrawal.address = withdrawal.address;
    withdrawal.merkel_proof = withdrawal.merkel_proof;
    withdrawal.state = 3;
    withdrawal.withdrawal_block_number = withdrawal.withdrawal_block_number;
    
    storageStore('withdrawal_' + retinfo.chain_id, JSON.stringify(retinfo));
    storageStore('withdrawal_' + withdrawal.chain_id + '_' + withdrawal.seq, JSON.stringify(withdrawal));
    transferCoin(withdrawal.address, withdrawal.amount);
    tlog('withdrawal', input.chain_id, JSON.stringify(withdrawal));
}


function submitChildBlockHeader(params){
    log('submitChildBlockHeader');
    let input = params;
    assert(checkchildChainValadator(input.chain_id,sender) === true,'submitChildBlockHeader sender is not validator.' );
    let info = JSON.parse(storageLoad(CHILD_CHAIN_ID_INFO + input.chain_id));
    assert(info !== false, CHILD_CHAIN_ID_INFO + input.chain_id + ' failed.');
    assert((input.block_header.seq - info.blockheight) === 1, 'input.block_header.seq is not correct blockheight.');
    let blockheight = 0;
    if(info.blockheight === 0) {
        assert(input.block_header.seq === 1,'input.block_header.seq is not correct.');
        blockheight = input.block_header.seq;
        info.blockheight = blockheight;
        info.lastblockhash = input.block_header.hash;
        input.sumitter = sender;
        input.currentheight = blockheight;
        storageStore(CHILD_CHAIN_BLOCK + info.chain_id + '_' + input.block_header.hash, JSON.stringify(input));
        storageStore(CHILD_CHAIN_ID_INFO + info.chain_id,JSON.stringify(info));
    }
    else {
        let preblock = JSON.parse(storageLoad(CHILD_CHAIN_BLOCK + info.chain_id + '_' + input.block_header.previous_hash));
        assert(preblock !== false,'preblockhash is not exist.');
        assert((input.block_header.seq - preblock.block_header.seq) === 1,'input blockheader.seq is not correct');
        blockheight = input.block_header.seq;
        info.blockheight = blockheight;
        info.lastblockhash = input.block_header.hash;
        input.sumitter = sender;
        input.currentheight = blockheight;
        storageStore(CHILD_CHAIN_BLOCK + info.chain_id + '_' + input.block_header.hash, JSON.stringify(input));
        storageStore(CHILD_CHAIN_ID_INFO + info.chain_id,JSON.stringify(info));
    }
    transferCoin(sender, BLOCK_REWARD);
    checkWithdrawal(params);
    //withdrawCoinOnTimer(info);
}

function depositToChildChain(params){
    let input = params;
    assert(thisPayCoinAmount>0, 'BU amount deposited is less than or equal to 0');
    let childChain_id = parseInt(storageLoad(CHILD_CHAIN_COUNT));
    assert(!((input.chain_id>childChain_id)||(input.chain_id<=0)) , 'chain_id is error');
    let validators_list = getchildChainValidators(input.chain_id);
    assert(validators_list.length>0, 'child chain node not exist.');
    assert(int64Compare(thisPayCoinAmount,input.amount) === 0,'amount is not equels thisPayCoinAmount');
    let assertinfo = JSON.parse(storageLoad(CHILD_CHAIN_ASSET + input.chain_id));
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

        storageStore(CHILD_CHAIN_ASSET + assertparam.chain_id , JSON.stringify(assertparam));
        storageStore(CHILD_CHAIN_ASSET + asset_chanin.chain_id+ '_'+ asset_chanin.seq, JSON.stringify(asset_chanin));
        tlog(TLOG_DEPOSIT, input.chain_id,JSON.stringify(asset_chanin)); 
    } 
    else {
        let asset_chanin_ = JSON.parse(storageLoad(CHILD_CHAIN_ASSET + input.chain_id+'_'+assertinfo.seq));
        let totleaamount = assertinfo.totalamount + input.amount;
        assertinfo.totalamount = totleaamount;
        assertinfo.seq = assertinfo.seq + 1;
        asset_chanin_.chain_id = input.chain_id;
        asset_chanin_.amount = input.amount;
        asset_chanin_.seq = assertinfo.seq;
        asset_chanin_.block_number = blockNumber;
        asset_chanin_.source_address = sender;
        asset_chanin_.address = input.address;
        storageStore(CHILD_CHAIN_ASSET + assertinfo.chain_id , JSON.stringify(assertinfo));
        storageStore(CHILD_CHAIN_ASSET + asset_chanin_.chain_id + '_'+ asset_chanin_.seq, JSON.stringify(asset_chanin_));
        tlog(TLOG_DEPOSIT, input.chain_id,JSON.stringify(asset_chanin_)); 
    }
}

function withdrawalChildChain(params){
    log('withdrawalChildChain');
    let input = params;
    let childChain_id = parseInt(storageLoad(CHILD_CHAIN_COUNT));
    assert(!((input.chain_id>childChain_id)||(input.chain_id<=0)) , 'chain_id is error');
    let validators_list = getchildChainValidators(input.chain_id);
    assert(validators_list.length>0, 'child chain node not exist.');
    let depositinfo = JSON.parse(storageLoad(CHILD_CHAIN_ASSET + input.chain_id));
    let assertinfo = JSON.parse(storageLoad('withdrawal_' + input.chain_id));
    assert(depositinfo !== false, 'chain is not exist');
    assert(depositinfo.totalamount > 0, 'totalamount less than 0');
    assert(depositinfo.totalamount - assertinfo.totalamount - input.amount > 0, 'totalamount greater than 0');
    assert(!verifyMerkelProof(input.chain_id, input.block_hash, input.merkel_proof), 'verify merkel proof error');
    if (assertinfo === false) {
        let assertparam = {};
        assertparam.chain_id = input.chain_id;
        assertparam.totalamount = input.amount;
        assertparam.seq = 1;
        assertparam.complete_seq = 0;

        let asset_chanin = {};
        asset_chanin.chain_id = input.chain_id;
        asset_chanin.amount = input.amount;
        asset_chanin.seq = 1;

        asset_chanin.block_hash = input.block_hash;
        asset_chanin.main_source_address = sender;
        asset_chanin.source_address = input.source_address;
        asset_chanin.address = input.address;
        asset_chanin.merkel_proof = input.merkel_proof;
        asset_chanin.state = 1;
        asset_chanin.withdrawal_block_number = blockNumber + effectiWithdrawalInterval;
        storageStore('withdrawal_' + assertparam.chain_id , JSON.stringify(assertparam));
        storageStore('withdrawal_' + asset_chanin.chain_id+ '_'+ asset_chanin.seq, JSON.stringify(asset_chanin));
        tlog(TLOG_WITHDRAWAL, input.chain_id,JSON.stringify(asset_chanin)); 
    } 
    else {
        let asset_chanin_ = JSON.parse(storageLoad('withdrawal_' + input.chain_id+'_'+assertinfo.seq));
        assert((assertinfo.seq + 1)===input.seq,'Wrong order of withdrawal');
        let totleaamount = assertinfo.totalamount + input.amount;
        assertinfo.totalamount = totleaamount;
        assertinfo.seq = assertinfo.seq + 1;
        assertinfo.complete_seq = assertinfo.complete_seq;
     
        asset_chanin_.chain_id = input.chain_id;
        asset_chanin_.amount = input.amount;
        asset_chanin_.seq = input.seq;
        asset_chanin_.block_hash = input.block_hash;
        asset_chanin_.main_source_address = sender;
        asset_chanin_.source_address = input.source_address;
        asset_chanin_.address = input.address;
        asset_chanin_.merkel_proof = input.merkel_proof;
        asset_chanin_.state = 1;
        asset_chanin_.withdrawal_block_number = blockNumber + effectiWithdrawalInterval;

        storageStore('withdrawal_' + assertinfo.chain_id , JSON.stringify(assertinfo));
        storageStore('withdrawal_' + asset_chanin_.chain_id + '_'+ asset_chanin_.seq, JSON.stringify(asset_chanin_));
        tlog(TLOG_WITHDRAWAL, input.chain_id,JSON.stringify(asset_chanin_)); 
    }
}

function updateChangeValidatorHistory(chainId, newHistoryIndex, addValidator, deleteValidator){
    let historyObj = {};
    historyObj.index = newHistoryIndex;
    historyObj.addValidator = addValidator;
    historyObj.deleteValidator = deleteValidator;
    historyObj.mainChainTxHash = '';

    let historyStr = JSON.stringify(historyObj);
    let key = CHILD_CHAIN_VALIDATOR_HISTORY + chainId + '_' + newHistoryIndex;
    storageStore(key, historyStr);
    tlog(TLOG_CHANGE_VALIDATOR, chainId, historyStr);
}

function depositCoin(params){
    //1.Check is validator, check parameter
    let chainId = params.chainId;
    assert(typeof chainId === 'string', 'Failed to check deposit coin params, must be string');
    assert(int64Compare(thisPayCoinAmount, toBaseUnit(CONST_COIN_BU)) >= 0, 'Failed to deposit coin, coin must gt hundred thousand.' + thisPayCoinAmount);

    let chainObj = JSON.parse(storageLoad(CHILD_CHAIN_ID_INFO + chainId));
    let index = findValidatorIndex(chainObj.validators, sender);
    //2.If exists, then updates
    if(index !== false){
        chainObj.validators[index][1] = int64Add(chainObj.validators[index][1], thisPayCoinAmount);
        storageStore(CHILD_CHAIN_ID_INFO + chainId, JSON.stringify(chainObj));   
        return;
    }

    //3.If not exsit, write CHILD_CHAIN_ID_INFO and CHILD_CHAIN_VALIDATOR_HISTORY, create tlog
    let newHistoryIndex = int64Add(chainObj.validatorHistoryIndex, 1);
    chainObj.validators.push([sender, thisPayCoinAmount]);
    chainObj.validatorHistoryIndex = newHistoryIndex;
    storageStore(CHILD_CHAIN_ID_INFO + chainId, JSON.stringify(chainObj));   

    updateChangeValidatorHistory(chainId, newHistoryIndex, sender, '');
}

function abolishValidator(params){
    //1.Check whether the parameters are normal, whether they are validators, and whether they exist
    let chainId = params.chainId;
    assert(typeof chainId === 'string', 'Failed to check deposit coin params, chain id must be string');
    let chainObj = JSON.parse(storageLoad(CHILD_CHAIN_ID_INFO + chainId));
    assert(findValidatorIndex(chainObj.validators, sender) !== false, 'Failed to check validator, sender must be validator' + sender);
    let index = findValidatorIndex(chainObj.validators, params.address);
    assert(index !== false, 'Failed to check validator, abolish address must be exsit:' + params.address);
    assert(chainObj.validators.length > 1, 'Failed to check validator, must be keep one validator.');

    //2.Delete the node, add history, reward
    let remainAmount = chainObj.validators[index][1];
    chainObj.validators.splice(index, 1);
    let averageAmount = int64Div(remainAmount, chainObj.validators.length);
    let i = 0;
    for(i = 0; i < chainObj.validators.length; i += 1){
        chainObj.validators[i][1] = int64Add(chainObj.validators[i][1], averageAmount);
        remainAmount = int64Sub(remainAmount, averageAmount);
    }
    if(remainAmount > 0){
        chainObj.validators[0][1] = int64Add(chainObj.validators[0][1], remainAmount);
    }

    //3.write CHILD_CHAIN_ID_INFO and CHILD_CHAIN_VALIDATOR_HISTORY, create tlog
    let newHistoryIndex = int64Add(chainObj.validatorHistoryIndex, 1);
    chainObj.validatorHistoryIndex = newHistoryIndex;
    storageStore(CHILD_CHAIN_ID_INFO + chainId, JSON.stringify(chainObj));   
    updateChangeValidatorHistory(chainId, newHistoryIndex, '', params.address);
}

function withdrawCoin(params){
    //1.Check whether it is a validator, whether it is in buffer, and whether the parameters are correct
    let chainId = params.chainId;
    assert(typeof chainId === 'string', 'Failed to check withdraw coin params, chain id must be string');
    let chainObjSrc = storageLoad(CHILD_CHAIN_ID_INFO + chainId);
    assert(chainObjSrc !== false, 'Failed to load chain info, may be not exsit.' + chainId);
    let chainObj = JSON.parse(chainObjSrc);
    assert(findValidatorIndex(chainObj.reserve_validators, sender) === false, 'Failed to withdraw coin, you are reserve validators.' + sender);
    let index = findValidatorIndex(chainObj.validators, sender);
    assert((index !== false), 'Failed to check validator, can not find:' + sender);

    //2.delete validator, record buffer. Store to CHILD_CHAIN_ID_INFO and CHILD_CHAIN_VALIDATOR_HISTORY
    let amount = chainObj.validators[index][1];
    chainObj.validators.splice(index, 1);
    chainObj.bufferValidators.push([sender, amount, blockNumber]);
    let newHistoryIndex = int64Add(chainObj.validatorHistoryIndex, 1);
    chainObj.validatorHistoryIndex = newHistoryIndex;
    storageStore(CHILD_CHAIN_ID_INFO + chainId, JSON.stringify(chainObj)); 
    updateChangeValidatorHistory(chainId, newHistoryIndex, '', sender);
}

function queryChildBlockHeader(params){
    log('queryChildBlockHeader');
    let input = params;
    let key = '';
    if(input.header_hash === ''){
        let info = JSON.parse(storageLoad(CHILD_CHAIN_ID_INFO + input.chain_id));
        assert(info !== false, 'queryChildBlockHeader childChainid_info_' + input.chain_id + ' failed.');
        key = CHILD_CHAIN_BLOCK + input.chain_id + '_' + info.lastblockhash;
    } 
    else {
        key = CHILD_CHAIN_BLOCK + input.chain_id + '_' + input.header_hash;
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

function queryChildFreshDeposit(params){
    log('queryChildFreshDeposit');
    let input = params;
    let info = JSON.parse(storageLoad(CHILD_CHAIN_ASSET + params.chain_id));
    assert(info !== false, 'queryChildFreshDeposit childChainAsset_' + input.chain_id + ' failed.');
    let retinfo = JSON.parse(storageLoad(CHILD_CHAIN_ASSET + info.chain_id + '_' + info.seq));
    assert(info !== false, 'queryChildFreshDeposit childChainAsset_' + input.chain_id + '_' + info.seq + ' failed.');
    return retinfo;
}

function queryChildDeposit(params){
    log('queryChildDeposit');
    let input = params;
    let retinfo = JSON.parse(storageLoad(CHILD_CHAIN_ASSET + input.chain_id + '_' + input.seq));
    assert(retinfo !== false, 'queryChildDeposit childChainAsset_' + input.chain_id + '_' + input.seq + ' failed.');
    return retinfo;
}

function queryChildChainInfo(params){
    log('queryChildChainInfo');
    let key = CHILD_CHAIN_ID + params.chain_id;
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
    let key = CHILD_CHAIN_ID_INFO + params.chain_id;
    let chaininfo = JSON.parse(storageLoad(key));

    let retinfo = {};
    if(chaininfo === false){
        retinfo = 'queryChildChainInfo failed.';
    } 
    else {
        let validatorArray  = [];
        let i = 0;
        for(i = 0; i < chaininfo.validators.length; i += 1){
            validatorArray.push(chaininfo.validators[i][0]);
        }
        retinfo.validators = validatorArray;
    }
    return retinfo;
}

function queryChangeValidatorHistory(params){
    let newHistoryIndex = 0;
    let chainId = params.chainId;
    if(params.index.length === 0){
        let chainObjSrc = storageLoad(CHILD_CHAIN_ID_INFO + chainId);
        assert(chainObjSrc !== false, 'Failed to query change validator history, cannot find chain id' + chainId);
        let chainObj = JSON.parse(chainObjSrc);
        newHistoryIndex = chainObj.validatorHistoryIndex;
    }
    else{
        newHistoryIndex = params.index;
    }
    let key = CHILD_CHAIN_VALIDATOR_HISTORY + chainId + '_' + newHistoryIndex;
    let historyStr = storageLoad(key);
    if(historyStr === false){
        let historyObj = {};
        historyObj.index = '-1';
        historyObj.addValidator = '';
        historyObj.deleteValidator = '';
        historyObj.mainChainTxHash = '';
        return historyObj;
    }
    return JSON.parse(historyStr);
}

function init(inputStr){
    log('init');
    storageStore(CHILD_CHAIN_COUNT, '0');
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
    else if(input.method === 'depositCoin'){
        depositCoin(input.params);
    }
    else if(input.method === 'abolishValidator'){
        abolishValidator(input.params);
    }
    else if(input.method === 'withdrawCoin'){
        withdrawCoin(input.params);
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
    else if(input.method === 'queryChildFreshDeposit'){
        result = queryChildFreshDeposit(input.params);
    }
    else if(input.method === 'queryChildDeposit'){
        result = queryChildDeposit(input.params);
    }
    else if(input.method === 'queryChangeValidatorHistory'){
        result = queryChangeValidatorHistory(input.params);
    }
    else{
        throw '<Query interface passes an invalid operation type>';
    }

    log(result);
    return JSON.stringify(result);
}
