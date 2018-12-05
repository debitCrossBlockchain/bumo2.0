'use strict';

const validatorSetSize       = 100;
const passRate               = 0.7;
const effectiveVoteInterval  = 50 * 1000 * 1000;
const minPledgeAmount        = 50000 * 100000000;
const minSuperadditionAmount = 100 * 100000000;
const applicantVar    = 'applicant_';
const abolishVar      = 'abolish_';
const proposerVar     = 'proposer';
const reasonVar       = 'reason';
const ballotVar       = 'ballot';
const depositsVar   = 'deposit_lists';
const pledgeAmountVar = 'pledge_coin_amount';
const expiredTimeVar  = 'voting_expired_time';
const buassert        = 'BU';
const statusinit      = 'init';
const statuschallenging   = 'challenging';
const statuserror         = 'error';
const depositcurrentseq         = 'error';


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

function transferBUAsset(dest, amount)
{
    assert((typeof dest === 'string') && (typeof amount === 'string'), 'Args type error. arg-dest and arg-amount must be a string,' + typeof dest + ',' + typeof amount);
    if(amount === '0'){ return true; }

    issueAsset(buassert, amount);
    payAsset(dest,thisAddress,buassert, amount);
    log('Pay Asset( ' + amount + ') to dest account(' + dest + ') succeed.');
}

function deposit(params){

    assert(checkchildChainValadator(params.chain_id,sender) === true,'deposit sender is not validator.' );
    let validators = getValidators();
    assert(validators !== false, 'Get validators failed.');
    assert(findI0(validators, sender) !== false, sender + ' has no permmition to commit  deposit.'); 
    let input = {};
    let depositseq = 'childdeposit_' + params.chain_id;
    let key = 'childdeposit_' + params.chain_id + '_' + params.seq;

    let depositinfo = JSON.parse(storageLoad(key));
    
    if(depositinfo === false)
    {
        /*
        let datahash = sha256(params.deposit_data,1);
        if(datahash !== params.hash)
        {
            log('data hash is ' + datahash + ' but params.hash is '+ params.hash); 
            return false;
        }*/
        input.votedcount = 1;
        input.starttime = blockTimestamp;
        input.votes = [sender];
        input.depositdata = params.deposit_data;
        input.seq = params.seq;
        storageStore(key,JSON.stringify(input));
        storageStore(depositseq,JSON.stringify(input));
    }
    else
    {
        if(blockTimestamp > depositinfo.starttime + effectiveVoteInterval)
        {
            log('Voting time expired, ' + input.address + ' is still validator.'); 
            storageDel(key);
        }
        else
        {
            assert(depositinfo.votes.includes(sender) !== true, sender + ' has voted.');
            depositinfo.votes.push(sender);
            input.votedcount = int64Add(input.votedcount,1);
            let validators = getValidators();
            if(input.votedcount < parseInt(validators.length * passRate + 0.5))
            {
                storageStore(key,JSON.stringify(input));
                storageStore(depositseq,JSON.stringify(input));
                return true; 
            }
            //tlog('deposit',params.chain_id,JSON.stringify(depositinfo));

            transferBUAsset(depositinfo.deposit_data['address'],depositinfo.deposit_data['amount']);
            storageDel(key);
        }
        let dealedseqkey = 'dealeddeposit_' + params.chain_id;
        //childChainCount = int64Add(childChainCount, 1);
       // storageStore('childChainCount',childChainCount.toString());
        storageStore(key,JSON.stringify(input));
    }
    

   return true;
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


function buildWithdrawalProofs(params){
    //input dest_address
    let key = "childwithdrawal_"  + params.chain_id + "_"+  params.seq;
    let withdrawalInfo = JSON.parse(storageLoad(key));
    if(withdrawalInfo === false)
    {
        return;
    }
    withdrawalInfo.status = statuschallenging;
    withdrawalInfo.merkelProof = params.merkel_proof;

    storageStore(key,JSON.stringify(withdrawalInfo));
   
    return false;
}


function withdrawal(params){
    //input dest_address
    
    let withdrawalSeqKey = "childwithdrawal_"  + params.chain_id + "_seq";
    let seqStr = storageLoad(withdrawalSeqKey);
     if(seqStr === false)
    {
        seq = 0;
    }
    else
    {
        seq = parseInt(seqStr);
    }
    seq = int64Add(seq,1);
    let assert_info = thisPayAsset;
    if(assert_info.key.code !== buassert)
    {
        return false;
    }
    let input = {};
    let key = "childwithdrawal_"  + params.chain_id + "_"+ seq;
    input.assert = assert_info.amount;
    input.dest_address = params.dest_address
    input.starttime = blockTimestamp;
    input.status = statusinit;
    input.seq = seq;
    input.merkelProof = '';
;
    storageStore(key,JSON.stringify(input));
    storageStore(withdrawalSeqKey,seq.toString());
    tlog('createChildChain',childChainid,JSON.stringify(params)); 
    return false;
}

function queryLastestChildDeposit(params){
    log('queryLastestChildDeposit');
    let input = params;
    let info = JSON.parse(storageLoad('childdeposit_' + params.chain_id));
    let retinfo = {};
    if(info === false){
        retinfo = 'queryLastestChildDeposit failed,' + params.chain_id;
    } else {
        retinfo = info;
    }
    return retinfo;

}

function query(input_str){
    let input  = JSON.parse(input_str);

    let result = {};
    if(input.method === 'getValidators'){
        result.current_validators = getValidators();
    }
    else if(input.method === 'queryLastestChildDeposit'){
        result = queryLastestChildDeposit(input.params);
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
