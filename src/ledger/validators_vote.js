'use strict';

let effectiveVoteInterval = 24 * 60 * 60 * 1000 * 1000;
let minPledgeAmount       = 100000 * 100000000;
const candidateSetSize    = 1000;
const passRate            = 0.7;
const abolishVar      = 'abolish_';
const proposerVar     = 'proposer';
const reasonVar       = 'reason';
const ballotVar       = 'ballot';
const candidatesVar   = 'validator_candidates';
const expiredTimeVar  = 'voting_expired_time';

function getObjectMetaData(key){
    assert(typeof key === 'string', 'Args type error, key must be a string.');

    let data = storageLoad(key);
    assert(data !== false, 'Get ' + key + ' from metadata failed.');

    let value = JSON.parse(data);
    return value;
}

function setMetaData(key, value)
{
    assert(typeof key === 'string', 'Args type error. key must be a string.');

    if(value === undefined){
        storageDel(key);
        log('Delete (' + key + ') from metadata succeed.');
    }
    else{
        let strVal = JSON.stringify(value);
        storageStore(key, strVal);
        log('Set key(' + key + '), value(' + strVal + ') in metadata succeed.');
    }
}

function transferCoin(dest, amount)
{
    assert((typeof dest === 'string') && (typeof amount === 'string'), 'Args type error. arg-dest and arg-amount must be a string.');
    if(amount === '0'){ return true; }

    payCoin(dest, amount);
    log('Pay coin( ' + amount + ') to dest account(' + dest + ') succeed.');
}

function findValidator(addr){
    let validators = getValidators();

    let i = 0;
    while(i < validators.length){
        if(validators[i][0] === addr){
            return true;
        }
        i += 1;
    }

    return false;
}

function applyAsCandidate(){
    let candidate = getValidateCandidate(sender);

    if(candidate === false){
        let com = int64Compare(thisPayCoinAmount, minPledgeAmount);
        assert(com === 1 || com === 0, 'Pledge coin amount must more than ' + minPledgeAmount);
    }

    assert(setValidateCandidate(sender, thisPayCoinAmount) === true, 'Application to become a validator or an additional deposit failed.');
}

function voteForCandidate(candidate, tokenAmount){
	assert(addressCheck(candidate) === true, 'Arg-candidate is not valid address.');
	assert(getValidateCandidate(candidate) !== false, 'No such validator candidate');
	
	assert(setVoteForCandidate(candidate, tokenAmount));
    return;
}

function takebackCoin(tokenAmount){
    let candidate = getValidateCandidate(sender);
    assert(candidate !== false, 'Sender(' + sender + ') is not validator candidate.');

    let left = int64Sub(candidate.pledge, tokenAmount);
    let com = int64Compare(left, minPledgeAmount);
    if(com === -1){
        assert(setValidateCandidate(sender, '-'+ candidate.pledge) === true, 'Quit candidate status failed.');
        assert(transferCoin(sender, candidate.pledge) === true, 'Takeback pledge coin failed.');
    }
    else{
        assert(setValidateCandidate(sender, '-'+ tokenAmount) === true, 'Reduced pledge coin operation failed.');
        assert(transferCoin(sender, tokenAmount) === true, 'Takeback pledge coin failed.');
    }

    if(findValidator(sender) === true){
        //to do: triger validator update, com += 1; just for avoid jslint, must be delete later
        com += 1;
    }
}

function voteForCandidate(candidate, tokenAmount){
    return;
}

function abolishValidator(malicious, proof){
    return;
}

function voteAbolishValidator(malicious){
    return;
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
    else if(input.method === 'getAbolishProposal'){
        result.abolish_proposal = storageLoad(abolishVar + input.params.address);
    }
	else if(input.method === 'getAbnormalRecords') {
		result.abnormal_records = getAbnormalRecords();
	}
    else{
       	throw '<unidentified operation type>';
    }

    log(result);
    return JSON.stringify(result);
}

function main(input_str){
    let input = JSON.parse(input_str);

    if(input.method === 'pledgeCoin'){
        applyAsCandidate();
    }
    else if(input.method === 'voteForCandidate'){
		assert(typeof input.params.address !== string, 'Arg-address should be string');
		assert(typeof input.params.coinAmount !== string, 'Arg-coinAmount should be string')
	    voteForCandidate(input.params.address, input.params.coinAmount);
    }
    else if(input.method === 'takebackCoin'){
	    takebackCoin(input.params.amount);
    }
    else if(input.method === 'abolishValidator'){
    	abolishValidator(input.params.address, input.params.proof);
    }
    else if(input.method === 'voteForAbolish'){
    	voteAbolishValidator(input.params.address);
    }
    else{
        throw '<undidentified operation type>';
    }
}

function init(){
    return true;
}
