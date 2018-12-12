'use strict';

const passRate               = 0.7;
const effectiveVoteInterval  = 24 * 3600 * 1000 * 1000;
const buassert        = 'BU';

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

function queryBalance(params){
    assert(params.dest !== undefined, 'have not dest address .');
    
    let info = JSON.parse(storageLoad(params.dest));
    log('queryBalance dest='+ params.dest );
    let retinfo = {};
    if(info === false){
        retinfo = '0';
    } else {

        retinfo = info;
    }
    
    //let retinfo = balanceOf(params.dest);
    log('queryBalance retinfo '+ retinfo);
    return retinfo;
}


function addBalance(dest, amount)
{
    assert((typeof dest === 'string') && (typeof amount === 'string'), 'Args type error. arg-dest and arg-amount must be a string,' + typeof dest + ',' + typeof amount);
    assert(addressCheck(dest) === true, 'dest address is not valid adress.');
    let current_balance = storageLoad(dest);
    if(current_balance === false){
        storageStore(dest,amount);
    }
    else{
        let balance = int64Add(current_balance,parseInt(amount));
        storageStore(dest,balance);
    }
    log('addBalance ' + amount + ') to dest account(' + dest + ') succeed.');
}

function reduceBalance(dest, amount)
{
    assert((typeof dest === 'string') && (typeof amount === 'string'), 'Args type error. arg-dest and arg-amount must be a string,' + typeof dest + ',' + typeof amount);
    assert(addressCheck(dest) === true, 'dest address is not valid adress.');
    let current_balance = storageLoad(dest);
    if(current_balance === false){
        storageStore(dest,amount);
    }
    else{
        let balance = int64Sub(current_balance,parseInt(amount));
        storageStore(dest,balance);
    }
    log('addBalance ' + amount + ') to dest account(' + dest + ') succeed.');
}

function deposit(params){
    //chain_id seq deposit_data
    let validators = getValidators();
    assert(validators !== false, 'Get validators failed.');
    assert(findI0(validators, sender) !== false, sender + ' has no permmition to commit  deposit.'); 
    
    assert(typeof params.seq === 'string', 'seq is not string');
    let depositseq = 'recentlydeposit';
    let key = 'childdeposit_' + params.seq;

    let depositinfo = JSON.parse(storageLoad(key));
    let recentlydepositinfo = JSON.parse(storageLoad(depositseq));
    let recentlySeq;
    if(recentlydepositinfo === false){
        recentlySeq = 0;
    }
    else{
        recentlySeq = recentlydepositinfo.seq;
    }
    
    if(depositinfo === false){
        let input = {};
        /*
        let datahash = sha256(params.deposit_data,1);
        if(datahash !== params.hash)
        {
            log('data hash is ' + datahash + ' but params.hash is '+ params.hash); 
            return false;
        }*/
        assert(parseInt(recentlySeq) + 1 === parseInt(params.seq),  'receive seq='+params.seq +' but recently is'+ recentlySeq);
        input.votedcount = '1';
        input.starttime = blockTimestamp;
        input.votes = [sender];
        input.depositdata = params.deposit_data;
        input.seq = params.seq;
        if(validators.length === 1){
            addBalance(params.deposit_data.address,params.deposit_data.amount);
            //transfer(params.deposit_data.address,params.deposit_data.amount);
            input.status = '1';
        }
        else{
            input.status = '0';
        }
        storageStore(key,JSON.stringify(input));
        storageStore(depositseq,JSON.stringify(input));
    
    }
    else{

        if(blockTimestamp > depositinfo.starttime + effectiveVoteInterval){
            log('cpc-log Voting time expired, ' + depositinfo.address + ' is still validator.'); 
            return false;
        }

        assert(depositinfo.votes.includes(sender) !== true, sender + ' has voted.');
        depositinfo.votes.push(sender);
        depositinfo.votedcount = int64Add(depositinfo.votedcount,1);
        if(parseInt(depositinfo.votedcount) < parseInt(validators.length * passRate + 0.5)){
            storageStore(key,JSON.stringify(depositinfo));
            log('cpc-log depositinfo.votedcount = ' + depositinfo.votedcount + ' validators.length=' + validators.length);
            return true; 
        }
        
        depositinfo.status = '1';
        //transfer(depositinfo.depositdata.address,depositinfo.depositdata.amount);
        addBalance(depositinfo.depositdata.address,depositinfo.depositdata.amount);
        storageStore(key,JSON.stringify(depositinfo));
        storageStore(depositseq,JSON.stringify(depositinfo));
    }

   return true;
}




function buildWithdrawalProofs(params){
    let key = 'childwithdrawal_'  + params.chain_id + '_'+  params.seq;
    let withdrawalInfo = JSON.parse(storageLoad(key));
    if(withdrawalInfo === false)
    {
        return;
    }
    withdrawalInfo.block_hash = params.block_hash;
    withdrawalInfo.merkelProof = params.merkel_proof;

    storageStore(key,JSON.stringify(withdrawalInfo));
    tlog('withdrawal','0',JSON.stringify(withdrawalInfo)); 
    return false;
}


function withdrawal(params){
    // chain_id、address、amount
    let queryBalanceParams = {};
    queryBalanceParams.dest = sender;
    let senderBalance = queryBalance(queryBalanceParams);
    if(parseInt(senderBalance) < parseInt(params.amount)){
        log('withdrawal  sender:' + sender + ',balance=' + senderBalance+ ' < ' + params.amount);
        return false;
    }

    let recentlyWithdrawalKey = 'recentlywithdrawal' ;
    let recentlyWithdrawal = storageLoad(recentlyWithdrawalKey);
    let seq;
     if(recentlyWithdrawal === false){
        seq = '0';
    }
    else{
        let recentInfo = JSON.parse(recentlyWithdrawal);
        seq = recentInfo.seq;
    }
    seq = int64Add(seq,1);
    let key = 'childwithdrawal_' + seq;

    let input = {};
    input.amount = params.amount;
    input.source_address = sender;
    input.chain_id = params.chain_id;
    //input.block_hash = '';
    input.address = params.address;
    input.seq = seq;
    //input.merkelProof = '';
    
    storageStore(key,JSON.stringify(input));
    storageStore(recentlyWithdrawalKey,JSON.stringify(input));
    //approve(sender,params.amount);
    reduceBalance(sender,params.amount);
    tlog('withdrawal','0',JSON.stringify(input)); 
    return true;
}

function queryChildDeposit(params){
    let querykey='';
    if(params === undefined){
        querykey = 'recentlydeposit';
    }
    else{
        querykey = 'childdeposit_' + params.seq;
    }
    log('queryChildDeposit querykey='+ querykey );
    let info = JSON.parse(storageLoad(querykey));
    let retinfo = {};
    if(info === false){
        retinfo = 'queryChildDeposit failed,' + querykey;
    } else {
        retinfo.index = info.seq;
        retinfo.executed = info.status;
        retinfo.validators = info.votes;
    }
    log('queryChildDeposit retinfo '+ retinfo);
    return retinfo;
}



function query(input_str){
    let input  = JSON.parse(input_str);

    let result = {};
    if(input.method === 'getValidators'){
        result.current_validators = getValidators();
    }
    else if(input.method === 'queryChildDeposit'){
        result = queryChildDeposit(input.params);
    }
    else if(input.method === 'queryBalance'){
        result = queryBalance(input.params);
    }
    else{
       	throw '<unidentified operation type>';
    }

    log(result);
    return JSON.stringify(result);
}

function main(input_str){
    let input = JSON.parse(input_str);

    if(input.method === 'deposit'){
        deposit(input.params);
    }
    else if(input.method === 'withdrawal'){
	    withdrawal(input.params);
    }
    else if(input.method === 'buildWithdrawalProofs'){
	    buildWithdrawalProofs(input.params);
    }
    else{
        throw '<undidentified operation type>';
    }
}

function init(){
     return true;
}
