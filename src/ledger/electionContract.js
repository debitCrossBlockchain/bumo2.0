var validatorSetsSize       = 5;
var numberOfVotesPassed     = 0.5;
var effectiveVotingInterval = 24 * 60 * 60 * 1000 * 1000; //1 day

var applicantsVar    = 'validator_applicants';
var proposalVar      = 'voting_proposal';
var abolishReasonVar = 'abolish_reason';
var ballotVar      = 'ballot';
var abolishVar     = 'abolish_validator';
var startTimeVar   = 'voting_start_time';
var expiredTimeVar = 'voting_expired_time';
var initiatorVar   = 'voting_initiator';
var equitiesVar    = 'equity_structure';
var auditAddress   = 'a00141213640ec3b86a61baecba33326acfc6e21db5b59';

function main(input_str)
{
    var result = false;
	var input = JSON.parse(input_str);
    switch(input.type)
    {
        case 1://pledgor applys to become validator
	    	callBackLog(sender + ' apply to become validator.');
	    	result = ApplyAsValidator();
            break;
        case 2:
	    	callBackLog(sender + ' audits the applicant want to become validator.');
	    	result = AuditApplicant(input.arg1); //arg1-{'addr1': true, 'addr2':true, 'addr3':false, ...}
            break;
        case 3://validator applys to exit validators sets
	    	callBackLog(sender + ' quit validator identity. ');
	    	result = QuitValidatorIdentity();
            break;
        case 4://someone applys to abolish certain validator
	    	callBackLog(sender + ' apply to abolish validator: ' + input.arg1);
	    	result = AbolishValidator(input.arg1, input.arg2); //arg1-address, arg2-proof
            break;
        case 5://quit abolish certain validator
	    	callBackLog(sender + ' quit to abolish validator.');
	    	result = QuitAbolishValidator();
            break;
        case 6://vote for cancelCertainValidator or quitCancelCertainValidator
	    	callBackLog(sender + ' votes for abolish validator: ' + input.arg1);
	    	result = VoteAbolishValidator(input.arg1);//agreeOrReject
            break;
        case 7: //query interface
	    	callBackLog(sender + ' query the info of ' + input.arg1);
            result = Query(input.arg1);
            break;
        default:
	    	callBackLog('unidentified operation type.');
            break;
    }

    return result;
}

function ApplyAsValidator()
{
    var pledgors = GetPledgors();
    if (pledgors[sender] === undefined)
    {
        var applicants = {};
        var applicantsData = callBackGetAccountMetaData(thisAddress, applicantsVar);
        if(applicantsData != false)
            applicants = JSON.parse(applicantsData.value); 

        if(applicants[sender] === undefined)
            applicants[sender] = payCoinAmount;
        else
            applicants[sender] += payCoinAmount;

        return SetMetaData(applicantsVar, applicants);
    }
    else
    {
        pledgors[sender] += payCoinAmount;
        if(false === SetMetaData(equitiesVar, pledgors))
            return false;

        return SetValidatorSets(pledgors);
    }
}

function AuditApplicant(auditResults)
{
    if(typeof auditResults != 'object')
    {
        callBackLog('Args type error, it must be an object.'); 
        return false;
    }

    if(sender != auditAddress)
    {
        callBackLog(sender + ' has no permission to audit.'); 
        return false;
    }

    var applicantsData = callBackGetAccountMetaData(thisAddress, applicantsVar);
    if(applicantsData === false)
    {
        callBackLog('No applicant to be audited.'); 
        return false;
    }

    var applicants = JSON.parse(applicantsData.value);
    var pledgors   = GetPledgors();
    for(audited in auditResults)
    {
        if((typeof audited != 'string') ||(applicants[audited] === undefined))
            continue;

        if(auditResults[audited] === true)
        {
            pledgors[audited] = applicants[audited];
        }
        else if(auditResults[audited] === false)
        {
            if(PayCoin(audited, applicants[audited]) === false)
                return false;
        }

        delete applicants[audited];
    }

    if(false === SetMetaData(applicantsVar, applicants))
        return false;

    if(false === SetMetaData(equitiesVar, pledgors))
        return false;

    return SetValidatorSets(pledgors);
}

function QuitValidatorIdentity()
{
    var currentValidators = callBackGetValidators();
    if(currentValidators === false)
    {
        callBackLog('Get current validator sets failed.'); 
        return false;
    }

    if(currentValidators.includes(sender) === false)
    {
        callBackLog('current validator sets has no ' + sender); 
        return true;
    }

    var pledgors = GetPledgors(currentValidators);
    if(false === PayCoin(sender, pledgors[sender]))
        return false;

    delete pledgors[sender];
    if(false === SetMetaData(equitiesVar, pledgors))
        return false;

    return SetValidatorSets(pledgors);
}

function AbolishValidator(nodeAddr, proof)
{
    if((typeof nodeAddr != 'string') || (typeof proof != 'string'))
    {
        callBackLog('Args type error, the two of them must be string.'); 
        return false;
    }

    var currentValidators = callBackGetValidators();
    if(currentValidators === false)
    {
        callBackLog('Get current validator sets failed.'); 
        return false;
    }

    if(currentValidators.includes(nodeAddr) === false)
    {
        callBackLog('current validator sets has no ' + nodeAddr); 
        return true;
    }

    var proposal = callBackGetAccountMetaData(thisAddress, proposalVar);
    if(proposal != false)
    {
        proposalContent = JSON.parse(proposal.value);
        var expiredTime = proposalContent[expiredTimeVar];
        var now         = consensusValue.close_time;

        if(now <= expiredTime)
        {
            callBackLog('There is unfinished vote proposal, please submit after that.'); 
            return false;
        }
    }

    var votingStartTime = consensusValue.close_time;
    var newProposal = {};

    newProposal[abolishVar]       = nodeAddr;
    newProposal[abolishReasonVar] = proof;
    newProposal[startTimeVar]   = votingStartTime;
    newProposal[expiredTimeVar] = votingStartTime + effectiveVotingInterval;
    newProposal[initiatorVar]   = sender;

    return SetMetaData(proposalVar, newProposal);
}

function QuitAbolishValidator()
{
    var proposalData = callBackGetAccountMetaData(thisAddress, proposalVar);
    if(proposalData === false)
    {
        callBackLog('Get proposal failed, maybe no one abolished validator or it be finished or quitted.'); 
        return true;
    }

    var proposal = JSON.parse(proposalData.value);
    if(sender != proposal[initiatorVar])
    {
        callBackLog(sender + 'is not initiator, has no permission to quit the proposal.');
        return false; 
    }

    return ClearAllProposalContent();
}

function VoteAbolishValidator(voteChoice) //voteChoice: agree-true or disagree-false
{
    if(typeof voteChoice != 'boolean')
    {
        callBackLog('Args type error, it must be a boolean.'); 
        return false;
    }

    var abolishAddr = CheckAndGetAbolisdAddr();
    if(abolishAddr === true)
        return abolishAddr;

    var currentSets = callBackGetValidators();
    if(currentSets === false)
    {
        callBackLog('Get current validator sets failed.'); 
        return false;
    }

    if(currentSets.includes(sender) === false)
    {
        callBackLog(sender + ' has no permission to vote.'); 
        return true;
    }

    var voteResult = VoteProcess(currentSets, voteChoice);
    callBackLog('voteResult: ' + voteResult); 

    if(voteResult === 'unfinished')
        return true;
    else if(voteResult === 'through')
        return ExecuteProposal(currentSets, abolishAddr);
    else if(voteResult === 'rejected')
        return ClearAllProposalContent();
    else
        return false;
}

function ExecuteProposal(currentSets, validator)
{
    callBackLog('Enter ExecuteProposal.');
    var pledgors = GetPledgors(currentSets);
    if(false === PayCoin(validator, pledgors[validator]))
        return false;

    delete pledgors[validator];
    if(false === SetMetaData(equitiesVar, pledgors))
        return false;

    if(false === ClearAllProposalContent())
        return false;

    return SetValidatorSets(pledgors);
}

function Query(arg)
{
    var result = callBackContractQuery(thisAddress, arg);
    return result;
}

function query(arg)
{
    var result = {};
    var ope    = parseInt(arg);

    switch(ope)
    {
        case 1:
            var currentSets = callBackGetValidators();
            result['Current_validators_sets'] = currentSets;
            break;
        case 2:
            GetValidatorApplicants(result);
            break;
        case 3:
            GetEquityStructure(result);
            break;
        case 4:
            GetProposalContent(result);
            break;
        case 5:
            GetBallotContent(result);
            break;
        default:
            var currentSets = callBackGetValidators();
            result['Current_validators_sets'] = currentSets;
            GetEquityStructure(result);
            GetProposalContent(result);
            GetBallotContent(result);
            break;
    }

    callBackLog('query result:');
    callBackLog(result);

    return result;
}

function GetProposalContent(result)
{
    var proposalData = callBackGetAccountMetaData(thisAddress, proposalVar);
    if(proposalData != false)
    {
        var proposal = JSON.parse(proposalData.value);
        result['Proposal_content_about_ablish_validator'] = proposal;
    }
}

function GetValidatorApplicants(result)
{
    var applicantsData = callBackGetAccountMetaData(thisAddress, applicantsVar);
    if(applicantsData != false)
    {
        var applicants = JSON.parse(applicantsData.value);
        result['applicants_want_to_become_validator'] = applicants;
    }
}

function GetBallotContent(result)
{
    var ballotData = callBackGetAccountMetaData(thisAddress, ballotVar);
    if(ballotData != false)
    {
        var ballot = JSON.parse(ballotData.value);
        result['Current_voting_result'] = ballot;
    }
}

function GetEquityStructure(result)
{
    var equitiesData = callBackGetAccountMetaData(thisAddress, equitiesVar);
    if(equitiesData != false)
    {
        var equityStruct = JSON.parse(equitiesData.value);
        var equities     = SortDictionary(equityStruct);
        result['Current_pledgors_and_pledge_coin'] = equities;
    }
}

function VoteProcess(currentSets, voteChoice)
{
    var ballotDict = {};

    var ballot = callBackGetAccountMetaData(thisAddress, ballotVar);
    if(ballot != false)
        ballotDict = JSON.parse(ballot.value);

    ballotDict[sender] = voteChoice;

    agreeCnt = 0;
    for(vote in ballotDict)
    {
        if(ballotDict[vote] === true)
            agreeCnt++;
    }

    var voteCnt    = Object.getOwnPropertyNames(ballotDict).length;
    var currentCnt = currentSets.length;

    if(agreeCnt / currentCnt >= numberOfVotesPassed) //proposal through
        return 'through';

    if((voteCnt - agreeCnt) / currentCnt > (1 - numberOfVotesPassed))//proposal rejected
        return 'rejected'; 

    if((agreeCnt / currentCnt < numberOfVotesPassed) && (voteCnt - agreeCnt) / currentCnt <= (1 - numberOfVotesPassed))
    {
        if(false === SetMetaData(ballotVar, ballotDict))
            return false;

        return 'unfinished';
    }
}

function GetPledgors(currentSets)
{
    if(currentSets === undefined)
    {
        currentSets = callBackGetValidators();
        if(currentSets === false)
        {
            callBackLog('Get current validators failed.');
            return false;
        }
    }

    var pledgors = {};
    var equities = callBackGetAccountMetaData(thisAddress, equitiesVar);

    if(equities === false)
    {
        for(index in currentSets)
            pledgors[currentSets[index]] = 0;
    }
    else
    {
        pledgors = JSON.parse(equities.value);
    }

    return pledgors;
}

function SetValidatorSets(pledgors)
{
    var sortedPledgors = SortDictionary(pledgors);
    var newSets = TopNKeyInMap(sortedPledgors, validatorSetsSize);
    var setsStr = JSON.stringify(newSets);

    result = callBackSetValidators(setsStr);
    callBackLog('Set validator sets succeed: ' + result);

    return result;
}

function CheckAndGetAbolisdAddr()
{
    var proposal = callBackGetAccountMetaData(thisAddress, proposalVar);
    if(proposal === false)
    {
        callBackLog('Get proposal that to be voted failed, maybe it was finished or quitted.'); 
        return true;
    }

    var proposalContent   = JSON.parse(proposal.value);
    var votingExpiredTime = proposalContent[expiredTimeVar];
    var now               = consensusValue.close_time;
    if(now > votingExpiredTime)
    {
        callBackLog('Voting time expired.'); 
        ClearAllProposalContent();
        return true;
    }
    
    var abolishAddr = proposalContent[abolishVar];
    return abolishAddr;
}

function ClearAllProposalContent()
{
    ballotRet   = SetMetaData(ballotVar, {});
    proposalRet = SetMetaData(proposalVar, {});

    return proposalRet && ballotRet;
}

function PayCoin(dest, amount)
{
    if((typeof dest != 'string') || (typeof amount != 'number'))
    {
        callBackLog('Args type error. dest(' + dest + ') must be a string, and amount(' + amount + ') must be a number.');
        return false;
    }

    if(amount === 0)
    {
        return true;
    }
    else if(amount < 0)
    {
        callBackLog('Coin amount must > 0.');
        return false;
    }

    var result = callBackPayCoin(dest, amount);
    if(result === false)
    {
        callBackLog('Pay coin( ' + amount + ') to dest account(' + dest + ') failed.');
        return false;
    }

    callBackLog('Pay coin( ' + amount + ') to dest account(' + dest + ') succeed.');
    return true;
}

function SetMetaData(key, value)
{
    if((typeof key != 'string') || (typeof value != 'object'))
    {
        callBackLog('Args type error. key must be a string, and value must be an object.');
        return false;
    }

    var metadata = {};
    if(Object.keys(value).length === 0)
        metadata = {'key': key, 'value': 'any', 'delete_flag': true};
    else
        metadata = {'key': key, 'value': JSON.stringify(value)};

    var ret = callBackSetAccountMetaData(metadata);
    if(ret === false)
    {
        callBackLog('Set ' + key + ' in metadata failed.');
        return false;
    }

    callBackLog('Set key:(' + key + '), value:(' + JSON.stringify(value) + ') in metadata succeed.');
    return true;
}

function by(name, minor)
{
    return function(x,y)
    {
        var a,b;
        if(x && y && typeof x === 'object' && typeof y ==='object')
        {
            a = x[name];
            b = y[name];

            if(a === b)
                return typeof minor === 'function' ? minor(y, x) : 0;

            if(typeof a === typeof b)
                return a > b ? -1:1;

            return typeof a > typeof b ? -1 : 1;
        }
    }
}

function SortDictionary(Dict) //value: desc sort first, key: asce sort second
{
    var tmpArr = [];
    for (key in Dict) 
        tmpArr.push({ 'key': key, 'value': Dict[key] });

    tmpArr.sort(by('value', by('key')));

    var sortMap = {};
    for (var i = 0; i < tmpArr.length; i++) 
    {
        var key = tmpArr[i]['key'];
        var val = tmpArr[i]['value'];
        sortMap[key] = val;
    }

    Dict = sortMap;

    return sortMap;
}

function TopNKeyInMap(sortedMap, n)
{
    var cnt = 0;
    var topN = [];
    for (key in sortedMap)
    {
        topN[cnt] = key;
        cnt++;

        if (cnt >= n)
            break;
    }

    return topN;
}
