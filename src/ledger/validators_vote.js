'use strict';

let payCoinNumber          = Number(payCoinAmount);
let validatorSetSize       = 100;
let votePassRate           = 0.8;
let effectiveVoteInterval  = 15 * 24 * 60 * 60 * 1000 * 1000;
let minPledgeAmount        = 100000000;
let minSuperadditionAmount = 1000000;

let applicantVar    = 'applicant_';
let abolishVar      = 'abolish_';
let proposerVar     = 'proposer';
let reasonVar       = 'reason';
let ballotVar       = 'ballot';
let candidatesVar   = 'validator_candidates';
let pledgeAmountVar = 'pledge_coin_amount';
let expiredTimeVar  = 'voting_expired_time';

function by(name, minor){
    let fun = function(x,y){
        assert(x && y && typeof x === 'object' && typeof y ==='object', 'x or y undefined, or their type are not object.');

        let a = x[name];
        let b = y[name];
        if(a === b){
            return typeof minor === 'function' ? minor(y, x) : 0;
        }

        if(typeof a === typeof b){
            return a > b ? -1:1;
        }

        return typeof a > typeof b ? -1 : 1;
    };
    return fun;
}

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
        assert(false !== storageDel(key), 'Delete (' + key + ') from metadata failed.');
        log('Delete (' + key + ') from metadata succeed.');
    }
    else{
        let strVal = JSON.stringify(value);
        assert(false !== storageStore(key, strVal), 'Set key(' + key + '), value(' + strVal + ') in metadata failed.');
        log('Set key(' + key + '), value(' + strVal + ') in metadata succeed.');
    }
}

function transferCoin(dest, amount)
{
    assert((typeof dest === 'string') && (typeof amount === 'number'), 'Args type error. arg-dest must be a string, and arg-amount must be a number.');
    assert(amount >= 0, 'Coin amount must >= 0.');
    if(amount === 0){ return; }

    assert(false !== payCoin(dest, String(amount)), 'Pay coin( ' + amount + ') to dest account(' + dest + ') failed.');
    log('Pay coin( ' + amount + ') to dest account(' + dest + ') succeed.');
}

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

function insertcandidatesSorted(applicant, amount, candidates){
    assert(typeof applicant === 'string' && typeof amount === 'number' && typeof candidates === 'object', 'args error, arg-applicant must be string, arg-amount must be number, arg-candidates must be arrary.');

    if(candidates.length >= (validatorSetSize * 2)){
        log('Validator candidates is enough.');
        return;
    }

    let i = 0;
    while(amount < candidates[i][1]){ i += 1; }

    if(amount === candidates[i][1]){
        while(applicant > candidates[i][0]){ i += 1; }
    }

    let element = [applicant, amount];
    candidates.splice(i, 0, element);

    return candidates;
}

function setValidatorsFromCandidate(candidates){
    let validators    = candidates.slice(0, validatorSetSize);
    let validatorsStr = JSON.stringify(validators);
    let result        = setValidators(validatorsStr);
    assert(result !== false, 'Set validator sets failed.');
    log('Set new validator sets(' + validatorsStr + ') succeed.');
}

function applyAsValidatorCandidate(){
    let candidates = getObjectMetaData(candidatesVar);
    let position   = findI0(candidates, sender);

    if (position !== false){
        let newAmount = candidates[position][1] + payCoinNumber;
        candidates.splice(position, 1);
        let newCandidates = insertcandidatesSorted(sender, newAmount, candidates);
        setMetaData(candidatesVar, newCandidates);

        if(findI0(newCandidates, sender) < validatorSetSize){
            setValidatorsFromCandidate(newCandidates);
        }
    }
    else{
        let applicant = {};
        let applicantKey = applicantVar + sender;
        let applicantStr = storageLoad(applicantKey);
        if(applicantStr !== false){
            if(payCoinNumber < minSuperadditionAmount){
                log('Superaddition coin amount must more than ' + minSuperadditionAmount);
                transferCoin(sender, payCoinNumber);
                return;
            }

            applicant = JSON.parse(applicantStr); 
            applicant[pledgeAmountVar] += payCoinNumber;
       }
       else{
             if(payCoinNumber < minPledgeAmount){
                log('Pledge coin amount must more than ' + minPledgeAmount);
                transferCoin(sender, payCoinNumber);
                return;
            }

            applicant[pledgeAmountVar] = payCoinNumber;
            applicant[ballotVar] = [];
       }

       /*superaddition pledge coin can update vote expired time*/
       applicant[expiredTimeVar] = blockTimestamp + effectiveVoteInterval;
       setMetaData(applicantKey, applicant);
   }

   return true;
}

function voteForApplicant(applicant){
    assert(typeof applicant === 'string', 'Args type error, it must be an object.'); 

    let validators = getValidators();
    assert(validators !== false, 'Get validators failed.');
    assert(findI0(validators, sender) !== false,  sender + ' has no permission to vote.');

    let applicantKey = applicantVar + applicant;
    let applicantData = getObjectMetaData(applicantKey);
    if(blockTimestamp > applicantData[expiredTimeVar]){
        log('Vote time is expired, applicant ' + applicant + ' be refused.');
        transferCoin(applicant, applicantData[pledgeAmountVar]);
        setMetaData(applicantKey);
        return false;
    }

    let candidates = getObjectMetaData(candidatesVar);
    if(candidates.length >= (validatorSetSize * 2)){
        log('Validator candidates is enough');
        return false;
    }

    assert(applicantData[ballotVar].includes(sender) !== true, sender + ' has voted.');
    applicantData[ballotVar].push(sender);
    if(Object.keys(applicantData[ballotVar]).length / validators.length < votePassRate){
        setMetaData(applicantKey, applicantData);
        return true;
    }

    let newCandidates = insertcandidatesSorted(applicant, applicantData[pledgeAmountVar], candidates);
    setMetaData(candidatesVar, newCandidates);
    setMetaData(applicantKey);

    if(findI0(newCandidates, applicant) < validatorSetSize){
        setValidatorsFromCandidate(newCandidates);
    }

    return true;
}

function takebackAllPledgeCoin(){
    let applicantKey = applicantVar + sender;
    let applicantStr = storageLoad(applicantKey);
    if(applicantStr !== false){
        let applicantData = JSON.parse(applicantStr);
        transferCoin(sender, applicantData[pledgeAmountVar]);
        setMetaData(applicantKey);
        return true;
    }

    let candidates = getObjectMetaData(candidatesVar);
    let position   = findI0(candidates, sender);
    if(position !== false){
        transferCoin(sender, candidates[position][1]);
        candidates.splice(position, 1);
        setMetaData(candidatesVar, candidates);

        let validators = getValidators();
        assert(validators !== false, 'Get validators failed.');
        if(findI0(validators, sender) !== false) {
            setValidatorsFromCandidate(candidates);
        }
    }
    return true;
}

function abolishValidator(malicious, proof){
    assert((typeof malicious === 'string') && (typeof proof === 'string'), 'Args type error, the two of them must be string.'); 

    let validators = getValidators();
    assert(validators !== false, 'Get validators failed.');
    assert(findI0(validators, sender) !== false, sender + ' has no permmition to abolish validator.'); 
    assert(findI0(validators, malicious) !== false, 'current validator sets has no ' + malicious); 

    let abolishKey = abolishVar + malicious;
    let abolishStr = storageLoad(abolishKey);
    if(abolishStr !== false){
        let abolishProposal = JSON.parse(abolishStr);
        if(blockTimestamp >= abolishProposal[expiredTimeVar]){
            log('Update expired time of abolishing validator(' + malicious + ').'); 
            abolishProposal[expiredTimeVar] = blockTimestamp;
            setMetaData(abolishKey, abolishProposal);
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
    newProposal[expiredTimeVar] = blockTimestamp + effectiveVoteInterval;
    newProposal[ballotVar]      = [];

    setMetaData(abolishKey, newProposal);
    return true;
}

function quitAbolishValidator(malicious){
    assert(typeof malicious === 'string', 'Args type error, the malicious must be string.'); 

    let abolishKey = abolishVar + malicious;
    let abolishProposal = getObjectMetaData(abolishKey);
    assert(sender === abolishProposal[proposerVar], sender + 'is not proposer, has no permission to quit the abolishProposal.');

    setMetaData(abolishKey);
    return true;
}

function voteAbolishValidator(malicious){
    assert(typeof malicious === 'string', 'Args type error, the malicious must be string.'); 

    let validators = getValidators();
    assert(validators !== false, 'Get validators failed.');
    assert(findI0(validators, sender) !== false, sender + ' has no permission to vote.'); 
    assert(findI0(validators, malicious) !== false, malicious + ' is not validator.'); 

    let abolishKey = abolishVar + malicious;
    let abolishProposal = getObjectMetaData(abolishKey);
    if(blockTimestamp >abolishProposal[expiredTimeVar]){
        log('Voting time expired, ' + malicious + ' is still validator.'); 
        setMetaData(abolishKey);
        return false;
    }
    
    assert(abolishProposal[ballotVar].includes(sender) !== true, sender + ' has voted.');
    abolishProposal[ballotVar].push(sender);
    if(Object.keys(abolishProposal[ballotVar]).length / validators.length < votePassRate){
        setMetaData(abolishKey, abolishProposal);
        return true;
    }

    let candidates = getObjectMetaData(candidatesVar);
    let position = findI0(candidates, malicious);
    let forfeit  = candidates[position][1] / 10;
    let leftCoin = candidates[position][1] - forfeit;

    transferCoin(malicious, leftCoin);
    setMetaData(abolishKey);
    candidates.splice(position, 1);

    let i = 0;
    let leftValidatorsCnt = validators.length - 1;
    let average = forfeit / leftValidatorsCnt;
    while(i < leftValidatorsCnt){
        candidates[i][1] += average;
        i += 1;
    }

    setMetaData(candidatesVar, candidates);
    setValidatorsFromCandidate(candidates);
    return true;
}

function query(input_str){
    let input  = JSON.parse(input_str);

    let result = {};
    if(input.method === 'getValidators'){
        result.Current_validators = getValidators();
    }
    else if(input.method === 'getCandidates'){
        result.Current_candidates = storageLoad(candidatesVar);
    }
    else if(input.method === 'getApplicantProposal'){
        result.application_proposal = storageLoad(applicantVar + input.params.address);
    }
    else if(input.method === 'getAbolishProposal'){
        result.abolish_proposal = storageLoad(abolishVar + input.params.address);
    }
    else{
       	result.default = '<unidentified operation type>';
    }

    log(result);
    return JSON.stringify(result);
}

function main(input_str){
    let input = JSON.parse(input_str);

    if(input.method === 'pledgeCoin'){
        applyAsValidatorCandidate();
    }
    else if(input.method === 'voteForApplicant'){
	    voteForApplicant(input.params.address);
    }
    else if(input.method === 'takebackCoin'){
	    takebackAllPledgeCoin();
    }
    else if(input.method === 'abolishValidator'){
    	abolishValidator(input.params.address, input.params.proof);
    }
    else if(input.method === 'quitAbolish'){
    	quitAbolishValidator(input.params.address);
    }
    else if(input.method === 'voteForAbolish'){
    	voteAbolishValidator(input.params.address);
    }
    else{
    	log('<unidentified operation type>');
    }
}

function init(){
    let validators = getValidators();
    assert(validators !== false, 'Get validators failed.');

    let initCandidates = validators.sort(by(1, by(0)));
    let candidateStr   = JSON.stringify(initCandidates);
    assert(storageStore(candidatesVar, candidateStr) !== false, 'Store candidates failed.');
    assert(setValidators(candidateStr) !== false, 'Set validators failed.');

    return true;
}
