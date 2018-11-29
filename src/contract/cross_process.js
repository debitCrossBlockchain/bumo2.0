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
    let key = "childdeposit_" + params.chain_id + "_" + params.hash;
    let depositinfo = JSON.parse(storageLoad(key));
    assert(depositinfo !== false,'depositinfo is not exist.');
    
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
        storageStore(key,JSON.stringify(input));
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
                return true; 
            }
            //tlog('deposit',params.chain_id,JSON.stringify(depositinfo));

            transferBUAsset(depositinfo.deposit_data['dest_address'],depositinfo.deposit_data['amount']);
            storageDel(key);
        }
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

function query(input_str){
    let input  = JSON.parse(input_str);

    let result = {};
    if(input.method === 'getValidators'){
        result.current_validators = getValidators();
    }
    else if(input.method === 'getCandidates'){
        result.current_candidates = storageLoad(candidatesVar);
    }
    else if(input.method === 'getApplicantProposal'){
        result.application_proposal = storageLoad(applicantVar + input.params.address);
    }
    else if(input.method === 'getAbolishProposal'){
        result.abolish_proposal = storageLoad(abolishVar + input.params.address);
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
    else{
        throw '<undidentified operation type>';
    }
}

function init(){
 
    return true;
}
