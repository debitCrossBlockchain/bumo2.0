'use strict';

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

function sortDictionary(Dict){
    let i = 0;
    let arr = [];
    let keys = Object.keys(Dict);
    while(i < keys.length){
        arr.push({ 'key': keys[i], 'value': Dict[keys[i]] });
        i += 1;
    }

    arr.sort(by('value', by('key')));

    i = 0;
    let sortMap = {};
    while(i < arr.length){
        let key = arr[i].key;
        let val = arr[i].value;
        sortMap[key] = val;
        i += 1;
    }

    Dict = sortMap;
    return sortMap;
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
        assert(true === storageDel(key), 'Delete (' + key + ') from metadata failed.');
        log('Delete (' + key + ') from metadata succeed.');
    }
    else{
        let strVal = JSON.stringify(value);
        assert(true === storageStore(key, strVal), 'Set key(' + key + '), value(' + strVal + ') in metadata failed.');
        log('Set key(' + key + '), value(' + strVal + ') in metadata succeed.');
    }
}

function transferCoin(dest, amount)
{
    assert((typeof dest === 'string') && (typeof amount === 'number'), 'Args type error. dest must be a string, and amount must be a number.');
    assert(amount >= 0, 'Coin amount must >= 0.');
    if(amount === 0){ return; }

    assert(true === payCoin(dest, String(amount)), 'Pay coin( ' + amount + ') to dest account(' + dest + ') failed.');
    log('Pay coin( ' + amount + ') to dest account(' + dest + ') succeed.');
}

function getCurrentValidators(){
    let validatorsStr = getValidators();
    assert(validatorsStr !== false, 'Get validators failed.');

    let validators = JSON.parse(validatorsStr);
    return validators;
}

function insertcandidatesSorted(applicant, amount, candidates){
    assert(typeof applicant === 'string' && typeof amount === 'number', 'args error, arg-applicant must be string and arg-amount must be number.');

    if(candidates === undefined){
        let candidatesStr = getObjectMetaData(candidatesVar);
        candidates = JSON.parse(candidatesStr);
    }

    assert(typeof candidates === 'object', 'candidates must be object.');

    let keys = Object.keys(candidates);
    if(keys.length >= (validatorSetSize * 2)){
        log('Validator candidates is enough');
        return;
    }

    let i = 0;
    let newCandidates = {};
    while(amount < candidates[keys[i]]){
        newCandidates[keys[i]] = candidates[keys[i]];
        i += 1;
    }

    if(amount === candidates[keys[i]]){
        while(applicant > keys[i]){
            newCandidates[keys[i]] = candidates[keys[i]];
            i += 1;
        }
    }

    newCandidates[applicant] = amount;
    while(i < keys.length){
        newCandidates[keys[i]] = candidates[keys[i]];
        i += 1;
    }

    return newCandidates;
}

function setValidatorsFromCandidate(candidates){
    let i = 0;
    let newValidators = {};
    let keys = Object.keys(candidates);

    while(i < validatorSetSize){
        newValidators[keys[i]] = candidates[keys[i]];
    }

    let validatorsStr = JSON.stringify(newValidators);
    assert(true === setValidators(validatorsStr), 'Set validator sets failed.');
    log('Set new validator sets(' + validatorsStr + ') succeed.');
}

function applyAsValidatorCandidate(){
    let candidates = getObjectMetaData(candidatesVar);
    if (candidates[sender] !== undefined){
        let newAmount = candidates[sender] + payCoinAmount;
        delete candidates[sender];
        insertcandidatesSorted(sender, newAmount, candidates);
        setMetaData(candidatesVar, candidates);
        setValidatorsFromCandidate(candidates);
    }
    else{
        let applicant = {};
        let applicantKey = applicantVar + sender;
        let applicantStr = storageLoad(applicantKey);
        if(applicantStr !== false){
            if(payCoinAmount < minSuperadditionAmount){
                log('Superaddition coin amount must more than ' + minSuperadditionAmount);
                transferCoin(sender, payCoinAmount);
                return;
            }

            applicant = JSON.parse(applicantStr); 
            applicant[pledgeAmountVar] += payCoinAmount;
       }
       else{
             if(payCoinAmount < minPledgeAmount){
                log('Pledge coin amount must more than ' + minPledgeAmount);
                transferCoin(sender, payCoinAmount);
                return;
            }

            applicant[pledgeAmountVar] = payCoinAmount;
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

    let validators = getCurrentValidators();
    assert(validators[sender] !== undefined,  sender + ' has no permission to vote.');

    let applicantKey = applicantVar + applicant;
    let applicantData = getObjectMetaData(applicantKey);
    if(blockTimestamp > applicantData[expiredTimeVar]){
        log('Vote time is expired, applicant ' + applicant + ' be refused.');
        transferCoin(applicant, applicantData[pledgeAmountVar]);
        setMetaData(applicantKey);
        return;
    }

    let candidates = getObjectMetaData(candidatesVar);
    if(Object.keys(candidates).length >= (validatorSetSize * 2)){
        log('Validator candidates is enough');
        return;
    }

    applicantData[ballotVar].push(sender);
    if(Object.keys(applicantData[ballotVar]).length / Object.keys(validators).length < votePassRate){
        setMetaData(applicantKey, applicantData);
        return;
    }

    let newCandidates = insertcandidatesSorted(applicant, applicantData[pledgeAmountVar], candidates);
    setMetaData(candidatesVar, newCandidates);
    setMetaData(applicantKey);
    setValidatorsFromCandidate(newCandidates);
    return true;
}

function takebackAllPledgeCoin(){
    let applicantKey = applicantVar + sender;
    let applicantData = storageLoad(applicantKey);
    if(applicantData !== false){
        transferCoin(sender, applicantData[pledgeAmountVar]);
        setMetaData(applicantKey);
        return;
    }

    let candidates = getObjectMetaData(candidatesVar);
    if(candidates[sender] !== undefined){
        transferCoin(sender, Number(candidates[sender]));
        delete candidates[sender];
        setMetaData(candidatesVar, candidates);

        let validators = getCurrentValidators();
        if(validators[sender] !== undefined) {
            setValidatorsFromCandidate(candidates);
        }
    }
    return true;
}

function abolishValidator(malicious, proof){
    assert((typeof malicious === 'string') && (typeof proof === 'string'), 'Args type error, the two of them must be string.'); 

    let validators = getCurrentValidators();
    assert(validators[sender] !== undefined, sender + ' has no permmition to abolish validator.'); 
    assert(validators[malicious] !== undefined, 'current validator sets has no ' + malicious); 

    let abolishKey = abolishVar + malicious;
    let abolishStr = storageLoad(abolishKey);
    if(abolishStr !== false){
        let abolishProposal = JSON.parse(abolishStr);
        if(blockTimestamp >= abolishProposal[expiredTimeVar]){
            log('Update expired time of abolishing validator(' + malicious + ').'); 
            abolishProposal[expiredTimeVar] = blockTimestamp;
        }
        return;
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

    let validators = getCurrentValidators();
    assert(validators[sender] !== undefined, sender + ' has no permission to vote.'); 

    let abolishKey = abolishVar + malicious;
    let abolishProposal = getObjectMetaData(abolishKey);
    if(blockTimestamp >abolishProposal[expiredTimeVar]){
        log('Voting time expired, ' + malicious + ' is still validator.'); 
        setMetaData(abolishKey);
        return;
    }
    
    abolishProposal[ballotVar].push(sender);
    if(Object.keys(abolishProposal[ballotVar]).length / Object.keys(validators).length < votePassRate){
        return;
    }

    let candidates = getObjectMetaData(candidatesVar);
    let forfeit    = candidates[malicious] / 10;
    let leftCoin   = candidates[malicious] - forfeit;

    transferCoin(malicious, leftCoin);
    setMetaData(abolishKey);
    delete candidates[malicious];
    delete validators[malicious];

    let i = 0;
    let keys = Object.keys(validators);
    let average = forfeit / keys.length;
    while(i < keys.length){
        candidates[keys[i]] += average;
        i += 1;
    }

    setMetaData(candidatesVar, candidates);
    setValidatorsFromCandidate(candidates);
    return true;
}

function query(input_str){
    let input  = JSON.parse(input_str);
    let result = {};
    switch(input.type){
        case 1:
            result.Current_validators = getCurrentValidators();
            break;
        case 2:
            result.Current_candidates = getObjectMetaData(candidatesVar);
            break;
        case 3:
            result.application_proposal = getObjectMetaData(applicantVar + input.arg1);
            break;
        case 4:
            result.abolish_proposal = getObjectMetaData(abolishVar + input.arg1);
            break;
        default:
	    	log('unidentified operation type.');
    }

    log(result);
    return result;
}

function main(input_str){
    let result = false;
	let input = JSON.parse(input_str);
    switch(input.type){
        case 1:
	    	result = applyAsValidatorCandidate();
            break;
        case 2:
	    	result = voteForApplicant(input.arg1);
            break;
        case 3:
	    	result = takebackAllPledgeCoin();
            break;
        case 4:
	    	result = abolishValidator(input.arg1, input.arg2);
            break;
        case 5:
	    	result = quitAbolishValidator(input.arg1);
            break;
        case 6:
	    	result = voteAbolishValidator(input.arg1);
            break;
        default:
	    	log('unidentified operation type.');
    }

    return result;
}

function init(){
    let validatorsStr = getValidators();
    assert(validatorsStr !== false, 'Get current validators failed.');

    let validators = JSON.parse(validatorsStr);
    let initCandidates = sortDictionary(validators);

    let result = storageStore(candidatesVar, initCandidates);
    assert(result === true, 'Store candidates failed.');

    return result;
}
