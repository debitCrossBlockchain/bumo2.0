var validatorSetsSize       = 5;
var numberOfVotesPassed     = 0.5;
var effectiveVotingInterval = 24 * 60 * 60 * 1000 * 1000;

var applicantsVar    = 'validator_applicants';
var proposalVar      = 'voting_proposal';
var abolishReasonVar = 'abolish_reason';
var ballotVar        = 'ballot';
var abolishVar       = 'abolish_validator';
var startTimeVar     = 'voting_start_time';
var expiredTimeVar   = 'voting_expired_time';
var initiatorVar     = 'voting_initiator';
var equitiesVar      = 'equity_structure';
var auditAddress     = 'buQnwLLuFTNeNJoZLM2WpJg3W99rBxuU4J9g'; //privbzqXhdxx5RdAkFVs2dKe3FGipKJgEZ4oaKSg3d8kdmUEzUwz1SwA

function main(input_str)
{
    var result = false;
	var input = JSON.parse(input_str);
    switch(input.type)
    {
        case 1:
	    	callBackLog(sender + ' apply to become validator.');
	    	result = ApplyAsValidator();
            break;
        case 2:
	    	callBackLog(sender + ' audits the applicant want to become validator.');
	    	result = AuditApplicant(input.arg1);
            break;
        case 3:
	    	callBackLog(sender + ' quit validator identity. ');
	    	result = QuitValidatorIdentity();
            break;
        case 4:
	    	callBackLog(sender + ' apply to abolish validator: ' + input.arg1);
	    	result = AbolishValidator(input.arg1, input.arg2);
            break;
        case 5:
	    	callBackLog(sender + ' quit to abolish validator.');
	    	result = QuitAbolishValidator();
            break;
        case 6:
	    	callBackLog(sender + ' votes for abolish validator: ' + input.arg1);
	    	result = VoteAbolishValidator(input.arg1);
            break;
        case 7:
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

        SetMetaData(applicantsVar, applicants);
    }
    else
    {
        pledgors[sender] += payCoinAmount;
        SetMetaData(equitiesVar, pledgors);
        SetValidatorSets(pledgors);
    }
}

function AuditApplicant(auditResults)
{
    if(typeof auditResults != 'object')
        throw 'Args type error, it must be an object.'; 

    if(sender != auditAddress)
        throw sender + ' has no permission to audit.'; 

    var applicantsData = callBackGetAccountMetaData(thisAddress, applicantsVar);
    if(applicantsData === false)
        throw 'No applicant to be audited.'; 

    var applicants = JSON.parse(applicantsData.value);
    var pledgors   = GetPledgors();
    for(audited in auditResults)
    {
        if((typeof audited != 'string') ||(applicants[audited] === undefined))
            continue;

        if(auditResults[audited] === true)
            pledgors[audited] = applicants[audited];
        else
            PayCoin(audited, applicants[audited]);

        delete applicants[audited];
    }

    SetMetaData(applicantsVar, applicants);
    SetMetaData(equitiesVar, pledgors);
    SetValidatorSets(pledgors);
}

function QuitValidatorIdentity()
{
    var pledgors = GetPledgors();
    if(pledgors[sender] != undefined)
    {
        PayCoin(sender, pledgors[sender]);
        delete pledgors[sender];
        SetMetaData(equitiesVar, pledgors);

        var currentValidators = callBackGetValidators();
        if(currentValidators === false)
            throw 'Get current validator sets failed.'; 

        if(currentValidators.includes(sender) === true)
            SetValidatorSets(pledgors);
    }
}

function AbolishValidator(nodeAddr, proof)
{
    if((typeof nodeAddr != 'string') || (typeof proof != 'string'))
        throw 'Args type error, the two of them must be string.'; 

    var currentValidators = callBackGetValidators();
    if(currentValidators === false)
        throw 'Get current validator sets failed.'; 

    if(currentValidators.includes(nodeAddr) === false)
        throw 'current validator sets has no ' + nodeAddr; 

    var proposal = callBackGetAccountMetaData(thisAddress, proposalVar);
    if(proposal != false)
    {
        proposalContent = JSON.parse(proposal.value);
        if(consensusValue.close_time <= proposalContent[expiredTimeVar])
        {
            callBackLog('There is unfinished vote proposal, please submit after that.'); 
            return;
        }
    }

    var votingStartTime = consensusValue.close_time;
    var newProposal = {};

    newProposal[abolishVar]       = nodeAddr;
    newProposal[abolishReasonVar] = proof;
    newProposal[startTimeVar]     = votingStartTime;
    newProposal[expiredTimeVar]   = votingStartTime + effectiveVotingInterval;
    newProposal[initiatorVar]     = sender;

    SetMetaData(proposalVar, newProposal);
}

function QuitAbolishValidator()
{
    var proposalData = callBackGetAccountMetaData(thisAddress, proposalVar);
    if(proposalData === false)
        throw 'Get proposal failed, maybe no one abolished validator or it be finished or quitted.'; 

    var proposal = JSON.parse(proposalData.value);
    if(sender != proposal[initiatorVar])
        throw sender + 'is not initiator, has no permission to quit the proposal.';

    ClearAllProposalContent();
}

function VoteAbolishValidator(voteChoice)
{
    if(typeof voteChoice != 'boolean')
        throw 'Args type error, it must be a boolean.'; 

    var currentSets = callBackGetValidators();
    if(currentSets === false)
        throw 'Get current validator sets failed.'; 

    if(currentSets.includes(sender) === false)
        throw sender + ' has no permission to vote.'; 

    var proposal = callBackGetAccountMetaData(thisAddress, proposalVar);
    if(proposal === false)
    {
        callBackLog('Get proposal that to be voted failed, maybe it was finished or quitted.'); 
        return; 
    }

    var proposalContent = JSON.parse(proposal.value);
    if(consensusValue.close_time > proposalContent[expiredTimeVar])
    {
        callBackLog('Voting time expired.'); 
        ClearAllProposalContent();
        return;
    }
    
    var voteResult = VoteProcess(currentSets, voteChoice);
    callBackLog('voteResult: ' + voteResult);

    if(voteResult === 'through') ExecuteProposal(currentSets, proposalContent[abolishVar]);
    else if(voteResult === 'rejected') ClearAllProposalContent();
}

function ExecuteProposal(currentSets, validator)
{
    callBackLog('Execute canceling ' + validator);
    var pledgors = GetPledgors(currentSets);

    var forfeit  = pledgors[validator] / 10;
    var leftCoin = pledgors[validator] - forfeit;

    PayCoin(validator, leftCoin);
    delete pledgors[validator];

    var sum = Object.getOwnPropertyNames(pledgors).length;
    var average = forfeit / sum;

    for(key in pledgors)
        pledgors[key] += average;

    SetMetaData(equitiesVar, pledgors);
    ClearAllProposalContent();
    SetValidatorSets(pledgors);
}

function Query(arg)
{
    return callBackContractQuery(thisAddress, arg);
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

    if(agreeCnt / currentCnt >= numberOfVotesPassed) return 'through';
    if((voteCnt - agreeCnt) / currentCnt > (1 - numberOfVotesPassed)) return 'rejected'; 

    if((agreeCnt / currentCnt < numberOfVotesPassed) && ((voteCnt - agreeCnt) / currentCnt <= (1 - numberOfVotesPassed)))
    {
        SetMetaData(ballotVar, ballotDict);
        return 'unfinished';
    }
}

function GetPledgors(currentSets)
{
    if(currentSets === undefined)
    {
        currentSets = callBackGetValidators();
        if(currentSets === false)
            throw 'Get current validators failed.';
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

    if(false === callBackSetValidators(setsStr))
        throw 'Set validator sets failed.';

    callBackLog('Set new validator sets(' + JSON.stringify(setsStr) + ') succeed.');
}

function ClearAllProposalContent()
{
    SetMetaData(ballotVar, {});
    SetMetaData(proposalVar, {});
}

function PayCoin(dest, amount)
{
    if((typeof dest != 'string') || (typeof amount != 'number'))
        throw 'Args type error. dest(' + dest + ') must be a string, and amount(' + amount + ') must be a number.';

    if(amount === 0) return;

    if(amount < 0)
        throw 'Coin amount must > 0.';

    if(false === callBackPayCoin(dest, amount))
        throw 'Pay coin( ' + amount + ') to dest account(' + dest + ') failed.';

    callBackLog('Pay coin( ' + amount + ') to dest account(' + dest + ') succeed.');
}

function SetMetaData(key, value)
{
    if((typeof key != 'string') || (typeof value != 'object'))
        throw 'Args type error. key must be a string, and value must be an object.';

    var metadata = {};
    if(Object.keys(value).length === 0)
        metadata = {'key': key, 'value': 'any', 'delete_flag': true};
    else
        metadata = {'key': key, 'value': JSON.stringify(value)};

    if(false === callBackSetAccountMetaData(metadata))
        throw 'Set key(' + key + '), value(' + JSON.stringify(value) + ') in metadata failed.';

    callBackLog('Set key(' + key + '), value(' + JSON.stringify(value) + ') in metadata succeed.');
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

function SortDictionary(Dict)
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
