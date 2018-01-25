'use strict';

let validatorSetsSize       = 5;
let numberOfVotesPassed     = 0.5;
let effectiveVotingInterval = 24 * 60 * 60 * 1000 * 1000;

let applicantsVar    = 'validator_applicants';
let proposalVar      = 'voting_proposal';
let abolishReasonVar = 'abolish_reason';
let ballotVar        = 'ballot';
let abolishVar       = 'abolish_validator';
let startTimeVar     = 'voting_start_time';
let expiredTimeVar   = 'voting_expired_time';
let initiatorVar     = 'voting_initiator';
let equitiesVar      = 'equity_structure';
let auditAddress     = 'buQhv4EyxaLzCjM1fxkNdCTE6NV39yeus8ge'; //privbz7k3ndjUBCo9opo3cb2qWeBK3KWWmLMFRuLDzWHoZiUVPjP7e9k

function by(name, minor)
{
    let fun = function(x,y)
    {
        let a;
        let b;
        if(x && y && typeof x === 'object' && typeof y ==='object')
        {
            a = x[name];
            b = y[name];

            if(a === b)
            {
                return typeof minor === 'function' ? minor(y, x) : 0;
            }

            if(typeof a === typeof b)
            {
                return a > b ? -1:1;
            }

            return typeof a > typeof b ? -1 : 1;
        }
        return;
    };

    return fun;
}

function sortDictionary(Dict)
{
    let i = 0;
    let arr = [];
    let keys = Object.keys(Dict);
    while(i < keys.length)
    {
        arr.push({ 'key': keys[i], 'value': Dict[keys[i]] });
        i += 1;
    }

    arr.sort(by('value', by('key')));

    i = 0;
    let sortMap = {};
    while(i < arr.length)
    {
        let key = arr[i].key;
        let val = arr[i].value;
        sortMap[key] = val;
        i += 1;
    }

    Dict = sortMap;
    return sortMap;
}

function topNKeyInMap(sortedMap, n)
{
    let i = 0;
    let topN = [];

    let keys = Object.keys(sortedMap);
    while(i < n)
    {
        topN[i] = keys[i];
        i += 1;
    }

    return topN;
}

function setMetaData(key, value)
{
    if((typeof key !== 'string') || (typeof value !== 'object'))
    {
        throw 'Args type error. key must be a string, and value must be an object.';
    }

    if(Object.keys(value).length === 0)
    {
        if(true === storageDel(key))
        {
            log('Delete (' + key + ') from metadata succeed.');
        }
        else
        {
            throw 'Delete (' + key + ') from metadata failed.';
        }
    }
    else
    {
        let strVal = JSON.stringify(value);
        if(true === storageStore(key, strVal))
        {
            log('Set key(' + key + '), value(' + strVal + ') in metadata succeed.');
        }
        else
        {
            throw 'Set key(' + key + '), value(' + strVal + ') in metadata failed.';
        }
    }
}

function transferCoin(dest, amount)
{
    if((typeof dest !== 'string') || (typeof amount !== 'number'))
    {
        throw 'Args type error. dest(' + dest + ') must be a string, and amount(' + amount + ') must be a number.';
    }

    if(amount === 0)
    {
        return;
    }

    if(amount < 0)
    {
        throw 'Coin amount must > 0.';
    }

    if(false === payCoin(dest, String(amount)))
    {
        throw 'Pay coin( ' + amount + ') to dest account(' + dest + ') failed.';
    }

    log('Pay coin( ' + amount + ') to dest account(' + dest + ') succeed.');
}

function getPledgors(currentSets)
{
    if(currentSets === undefined)
    {
        currentSets = getValidators();
        if(currentSets === false)
        {
            throw 'Get current validators failed.';
        }
    }

    let pledgors = {};
    let equities = storageLoad(equitiesVar);
    if(equities === false)
    {
        let i = 0;
        while(i < currentSets.length)
        {
            pledgors[currentSets[i]] = 0;
            i += 1;
        }
    }
    else
    {
        pledgors = JSON.parse(equities);
    }

    return pledgors;
}

function setValidatorSets(pledgors)
{
    let sortedPledgors = sortDictionary(pledgors);
    let newSets = topNKeyInMap(sortedPledgors, validatorSetsSize);
    let setsStr = JSON.stringify(newSets);

    if(false === setValidators(setsStr))
    {
        throw 'Set validator sets failed.';
    }

    log('Set new validator sets(' + JSON.stringify(setsStr) + ') succeed.');
}

function clearAllProposalContent()
{
    setMetaData(ballotVar, {});
    setMetaData(proposalVar, {});
}

function executeProposal(currentSets, validator)
{
    log('Execute canceling ' + validator);
    let pledgors = getPledgors(currentSets);

    let forfeit  = pledgors[validator] / 10;
    let leftCoin = pledgors[validator] - forfeit;

    transferCoin(validator, Number(leftCoin));
    delete pledgors[validator];

    let keys = Object.keys(pledgors);
    let average = forfeit / keys.length;

    let i = 0;
    while(i < keys.length)
    {
        pledgors[keys[i]] += average;
        i += 1;
    }

    setMetaData(equitiesVar, pledgors);
    clearAllProposalContent();
    setValidatorSets(pledgors);
}

function getProposalContent(result)
{
    let proposalData = storageLoad(proposalVar);
    if(proposalData !== false)
    {
        let proposal = JSON.parse(proposalData);
        result.Proposal_content_about_ablish_validator = proposal;
    }
}

function getValidatorApplicants(result)
{
    let applicantsData = storageLoad(applicantsVar);
    if(applicantsData !== false)
    {
        let applicants = JSON.parse(applicantsData);
        result.applicants_want_to_become_validator = applicants;
    }
}

function getBallotContent(result)
{
    let ballotData = storageLoad(ballotVar);
    if(ballotData !== false)
    {
        let ballot = JSON.parse(ballotData);
        result.Current_voting_result = ballot;
    }
}

function getEquityStructure(result)
{
    let equitiesData = storageLoad(equitiesVar);
    if(equitiesData !== false)
    {
        let equityStruct = JSON.parse(equitiesData);
        let equities     = sortDictionary(equityStruct);
        result.Current_pledgors_and_pledge_coin = equities;
    }
}

function voteProcess(currentSets, voteChoice)
{
    let ballotDict = {};

    let ballot = storageLoad(ballotVar);
    if(ballot !== false)
    {
        ballotDict = JSON.parse(ballot);
    }

    ballotDict[sender] = voteChoice;

    let i = 0;
    let agreeCnt = 0;
    let keys = Object.keys(ballotDict);
    while(i < keys.length)
    {
        if(ballotDict[keys[i]] === true)
        {
            agreeCnt += 1;
        }
        i += 1;
    }

    let voteCnt    = Object.keys(ballotDict).length;
    let currentCnt = currentSets.length;

    if(agreeCnt / currentCnt >= numberOfVotesPassed)
    {
        return 'through';
    }

    if((voteCnt - agreeCnt) / currentCnt > (1 - numberOfVotesPassed))
    {
        return 'rejected'; 
    }

    if((agreeCnt / currentCnt < numberOfVotesPassed) && ((voteCnt - agreeCnt) / currentCnt <= (1 - numberOfVotesPassed)))
    {
        setMetaData(ballotVar, ballotDict);
        return 'unfinished';
    }
}

function applyAsValidator()
{
    let pledgors = getPledgors();
    if (pledgors[sender] === undefined)
    {
        let applicants = {};
        let applicantsData = storageLoad(applicantsVar);
        if(applicantsData !== false)
        {
            applicants = JSON.parse(applicantsData); 
        }

        if(applicants[sender] === undefined)
        {
            applicants[sender] = payCoinAmount;
        }
        else
        {
            applicants[sender] += payCoinAmount;
        }

        setMetaData(applicantsVar, applicants);
    }
    else
    {
        pledgors[sender] += payCoinAmount;
        setMetaData(equitiesVar, pledgors);
        setValidatorSets(pledgors);
    }
}

function auditApplicant(auditResults)
{
    if(typeof auditResults !== 'object')
    {
        throw 'Args type error, it must be an object.'; 
    }

    if(sender !== auditAddress)
    {
        throw sender + ' has no permission to audit.'; 
    }

    let applicantsData = storageLoad(applicantsVar);
    if(applicantsData === false)
    {
        throw 'No applicant to be audited.'; 
    }

    let applicants = JSON.parse(applicantsData);
    let pledgors   = getPledgors();
    
    let i = 0;
    let keys = Object.keys(auditResults);
    while(i < keys.length)
    {
        if((typeof keys[i] === 'string') && (applicants[keys[i]] !== undefined))
        {
            if(auditResults[keys[i]] === true)
            {
                pledgors[keys[i]] = applicants[keys[i]];
            }
            else
            {
                transferCoin(keys[i], Number(applicants[keys[i]]));
            }

            delete applicants[keys[i]];
        }

        i += 1;
    }

    setMetaData(applicantsVar, applicants);
    setMetaData(equitiesVar, pledgors);
    setValidatorSets(pledgors);
}

function quitValidatorIdentity()
{
    let pledgors = getPledgors();
    if(pledgors[sender] !== undefined)
    {
        transferCoin(sender, Number(pledgors[sender]));
        delete pledgors[sender];
        setMetaData(equitiesVar, pledgors);

        let currentValidators = getValidators();
        if(currentValidators === false)
        {
            throw 'Get current validator sets failed.'; 
        }

        if(currentValidators.includes(sender) === true)
        {
            setValidatorSets(pledgors);
        }
    }
}

function abolishValidator(nodeAddr, proof)
{
    if((typeof nodeAddr !== 'string') || (typeof proof !== 'string'))
    {
        throw 'Args type error, the two of them must be string.'; 
    }

    let currentValidators = getValidators();
    if(currentValidators === false)
    {
        throw 'Get current validator sets failed.'; 
    }

    if(currentValidators.includes(sender) === false)
    {
        throw sender + ' has no permmition to abolish validator.; 
    }

    if(currentValidators.includes(nodeAddr) === false)
    {
        throw 'current validator sets has no ' + nodeAddr; 
    }

    let proposalData = storageLoad(proposalVar);
    if(proposalData !== false)
    {
        let proposal = JSON.parse(proposalData);
        if(blockTimestamp <= proposal[expiredTimeVar])
        {
            log('There is unfinished vote proposal, please submit after that.'); 
            return;
        }
    }

    let votingStartTime = blockTimestamp;
    let newProposal = {};

    newProposal[abolishVar]       = nodeAddr;
    newProposal[abolishReasonVar] = proof;
    newProposal[startTimeVar]     = votingStartTime;
    newProposal[expiredTimeVar]   = votingStartTime + effectiveVotingInterval;
    newProposal[initiatorVar]     = sender;

    setMetaData(proposalVar, newProposal);
}

function quitAbolishValidator()
{
    let proposalData = storageLoad(proposalVar);
    if(proposalData === false)
    {
        throw 'Get proposal failed, maybe no one abolished validator or it be finished or quitted.'; 
    }

    let proposal = JSON.parse(proposalData);
    if(sender !== proposal[initiatorVar])
    {
        throw sender + 'is not initiator, has no permission to quit the proposal.';
    }

    clearAllProposalContent();
}

function voteAbolishValidator(voteChoice)
{
    if(typeof voteChoice !== 'boolean')
    {
        throw 'Args type error, it must be a boolean.'; 
    }

    let currentSets = getValidators();
    if(currentSets === false)
    {
        throw 'Get current validator sets failed.'; 
    }

    if(currentSets.includes(sender) === false)
    {
        throw sender + ' has no permission to vote.'; 
    }

    let proposal = storageLoad(proposalVar);
    if(proposal === false)
    {
        log('Get proposal that to be voted failed, maybe it was finished or quitted.'); 
        return; 
    }

    let proposalContent = JSON.parse(proposal);
    if(blockTimestamp > proposalContent[expiredTimeVar])
    {
        log('Voting time expired.'); 
        clearAllProposalContent();
        return;
    }
    
    let voteResult = voteProcess(currentSets, voteChoice);
    log('voteResult: ' + voteResult);

    if(voteResult === 'through')
    {
        executeProposal(currentSets, proposalContent[abolishVar]);
    }
    else if(voteResult === 'rejected')
    {
        clearAllProposalContent();
    }
}

function query(arg)
{
    let result = {};
    let ope    = parseInt(arg);
    let currentSets;


    switch(ope)
    {
        case 1:
            currentSets = getValidators();
            result.Current_validators_sets = currentSets;
            break;
        case 2:
            getValidatorApplicants(result);
            break;
        case 3:
            getEquityStructure(result);
            break;
        case 4:
            getProposalContent(result);
            break;
        case 5:
            getBallotContent(result);
            break;
        default:
            currentSets = getValidators();
            result.Current_validators_sets = currentSets;
            getEquityStructure(result);
            getProposalContent(result);
            getBallotContent(result);
    }
    log(result);

    return result;
}

function queryInfo(arg)
{
    return contractQuery(thisAddress, arg);
}

function main(input_str)
{
    let result = false;
	let input = JSON.parse(input_str);
    switch(input.type)
    {
        case 1:
	    	log(sender + ' apply to become validator.');
	    	result = applyAsValidator();
            break;
        case 2:
	    	log(sender + ' audits the applicant want to become validator.');
	    	result = auditApplicant(input.arg1);
            break;
        case 3:
	    	log(sender + ' quit validator identity. ');
	    	result = quitValidatorIdentity();
            break;
        case 4:
	    	log(sender + ' apply to abolish validator: ' + input.arg1);
	    	result = abolishValidator(input.arg1, input.arg2);
            break;
        case 5:
	    	log(sender + ' quit to abolish validator.');
	    	result = quitAbolishValidator();
            break;
        case 6:
	    	log(sender + ' votes for abolish validator: ' + input.arg1);
	    	result = voteAbolishValidator(input.arg1);
            break;
        case 7:
	    	log(sender + ' query the info of ' + input.arg1);
            result = queryInfo(input.arg1);
            break;
        default:
	    	log('unidentified operation type.');
    }

    return result;
}
