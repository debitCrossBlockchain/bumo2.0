'use strict';

let effectiveVoteInterval = 24 * 60 * 60 * 1000 * 1000;
let effectiveAbolishVoteInterval = 15 * effectiveVoteInterval;
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
    let candidate = getValidatorCandidate(sender);

    if(candidate === false){
        let com = int64Compare(thisPayCoinAmount, minPledgeAmount);
        assert(com === 1 || com === 0, 'Pledge coin amount must more than ' + minPledgeAmount);
    }

    assert(setValidatorCandidate(sender, thisPayCoinAmount) === true, 'Application to become a validator or an additional deposit failed.');
}

function voteForCandidate(candidate, tokenAmount){
	assert(addressCheck(candidate) === true, 'Arg-candidate is not valid address.');
	assert(getValidatorCandidate(candidate) !== false, 'No such validator candidate');
	
	assert(setVoteForCandidate(candidate, tokenAmount));
    return;
}

function takebackCoin(tokenAmount){
    let candidate = getValidatorCandidate(sender);
    assert(candidate !== false, 'Sender(' + sender + ') is not validator candidate.');

    let left = int64Sub(candidate.pledge, tokenAmount);
    let com = int64Compare(left, minPledgeAmount);
    if(com === -1){
        assert(setValidatorCandidate(sender, '-'+ candidate.pledge) === true, 'Quit candidate status failed.');
        assert(transferCoin(sender, candidate.pledge) === true, 'Takeback pledge coin failed.');
    }
    else{
        assert(setValidatorCandidate(sender, '-'+ tokenAmount) === true, 'Reduced pledge coin operation failed.');
        assert(transferCoin(sender, tokenAmount) === true, 'Takeback pledge coin failed.');
    }

    if(findValidator(sender) === true){
        //to do: triger validator update, com += 1; just for avoid jslint, must be delete later
        com += 1;
    }
}


function voteAbolishValidator(malicious){

    assert(addressCheck(malicious) === true, 'Arg-malicious is not valid adress.');
    let abolishKey = abolishVar + malicious;
    let abolishStr = storageLoad(abolishKey);
    if(abolishStr === false){
        log(abolishKey + ' is not existed, voting maybe passed or expired.');
        return false;
    }
    let validators = getValidators();
    let candidate = getValidatorCandidate(malicious);
    assert(candidate !== false, 'Sender(' + sender + ') is not validator candidate.');
    assert(validators !== false, 'Get validators failed.');
    assert(validators.length > 1, 'The number of validators must > 1.');
    assert(findValidator(sender) !== false, sender + ' has no permission to vote.'); 
    assert(findValidator(malicious) !== false, malicious + ' is not validator.'); 

    let abolishProposal = JSON.parse(abolishStr);
    if(blockTimestamp >abolishProposal[expiredTimeVar]){
        log('Voting time expired, ' + malicious + ' is still validator.'); 
        setMetaData(abolishKey);
        return false;
    }
    
    assert(abolishProposal[ballotVar].includes(sender) !== true, sender + ' has voted.');
    abolishProposal[ballotVar].push(sender);
    let halfVotes = 0;/*The vote not in validators is halved */
    let i = 0;
    while(i < abolishProposal[ballotVar].length){
        if(findValidator(abolishProposal[ballotVar]) === false){
            halfVotes += 1;
        }
        i += 1;
    }
    let validVotes = Object.keys(abolishProposal[ballotVar]).length - parseInt(halfVotes * 0.5);

    if(validVotes < parseInt(validators.length * passRate + 0.5)){
        setMetaData(abolishKey, abolishProposal);
        return true;
    }

    let forfeit    = candidate.pledge;/*step here, logic promising position !== false*/

    let leftValidatorsCnt = validators.length - 1;
    let award   = int64Mod(forfeit, leftValidatorsCnt);
    let average = int64Div(forfeit, leftValidatorsCnt);
    let index = 0;
    let newTokenAmount;
    if(award !== '0'){
        if (validators[index][0] === malicious){
            candidate = getValidatorCandidate(malicious);
            assert(setValidatorCandidate(malicious, '-'+ candidate.pledge) === true, 'Abolish candidate status failed.');
            index += 1;
        }
        candidate = getValidatorCandidate(validators[index][0]);
        newTokenAmount = candidate.pledge + award + average;
        assert(setVoteForCandidate(candidate, newTokenAmount));
    }
    while(index < leftValidatorsCnt){
        candidate = getValidatorCandidate(validators[index][0]);
        newTokenAmount = candidate.pledge + average;
        assert(setVoteForCandidate(validators[index][0], newTokenAmount));
        index += 1;
    }

    setMetaData(abolishKey);
    return true;
}
function abolishValidator(malicious, proof){
    assert(addressCheck(malicious) === true, 'Arg-malicious is not valid adress.');
    assert(typeof proof === 'string', 'Args type error, arg-proof must be string.'); 

    let validators = getValidators();
    assert(validators !== false, 'Get validators failed.');
    assert(findValidator(sender) !== false, sender + ' has no permmition to abolish validator.'); 
    assert(findValidator(malicious) !== false, 'current validator sets has no ' + malicious); 

    let abolishKey = abolishVar + malicious;
    let abolishStr = storageLoad(abolishKey);
    if(abolishStr !== false){
        let abolishProposal = JSON.parse(abolishStr);
        if(blockTimestamp >= abolishProposal[expiredTimeVar]){
            log('Update expired time of abolishing validator(' + malicious + ').'); 
            voteAbolishValidator(malicious);
        }
        else{
            log('Already abolished validator(' + malicious + ').'); 
        }
        return true;
    }

    let newProposal = {};
    newProposal[abolishVar]     = malicious;
    newProposal[reasonVar]      = proof;
    newProposal[proposerVar]    = sender;
    newProposal[expiredTimeVar] = blockTimestamp + effectiveAbolishVoteInterval;
    newProposal[ballotVar]      = [sender];

    setMetaData(abolishKey, newProposal);
    return true;
}

function query(input_str){
    let input  = JSON.parse(input_str);

    let result = {};
    if(input.method === 'getValidators'){
        result.current_validators = getValidators();
    }
    else if(input.method === 'getCandidate'){
        result.candidate = getValidatorCandidate(input.address);
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
		assert(typeof input.params.address === 'string', 'Arg-address should be string');
		assert(typeof input.params.coinAmount === 'string', 'Arg-coinAmount should be string');
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
