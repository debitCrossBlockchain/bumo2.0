'use strict';

const PROPOSAL_NEW_INDEX = 'proposal_new_index';
const PROPOSAL_HISTORY = 'proposal_history_';

function findValidatorIndex(array, key){
    let i = 0;
    for(i = 0; i < array.length; i += 1){
        if(array[i][0] === key) { return i; }
    }
    return false;
}

function checkUpdateSystemValidators(executed, proposalValidators, curValidators, addValidator, deleteValidator){
    //It has already been executed and does not need to be executed again
    if(executed){
        return false;
    }

    //May be 0.75
    if(proposalValidators.length !== curValidators.length){
        return false;
    }

    if(addValidator.length > 0){
        curValidators.push([addValidator, 0]);
    }

    if(deleteValidator.length > 0){
        curValidators.splice(findValidatorIndex(curValidators, deleteValidator), 1);
    }

    setValidators(JSON.stringify(curValidators));
    return true;
}

function changeValidators(curValidators, mainChainTxHash, addValidator, deleteValidator, index){
    assert(typeof mainChainTxHash === 'string' && 
               typeof index === 'string', 
              'Parameter format error, must be string.');
    assert((addValidator !==undefined || deleteValidator !==undefined), 'Failed to check address, all empty.');
	if(addValidator === undefined){
		addValidator = '';
	}
	if(deleteValidator === undefined){
		deleteValidator = '';
	}
	
    if(addValidator.length > 0){
        assert(addressCheck(addValidator) === true, 'Failed to check address');
        assert(findValidatorIndex(curValidators, addValidator) === false, 'Failed to add validator, already existed.');
    }
    if(deleteValidator.length > 0){
        assert(addressCheck(deleteValidator) === true, 'Failed to check address');
        assert(findValidatorIndex(curValidators, deleteValidator) !== false, 'Failed to delete validator, can not find it');
    }

    let curIndex = storageLoad(PROPOSAL_NEW_INDEX);
    let nextIndex = int64Add(curIndex, 1);
    assert((index === curIndex) ||(index === nextIndex) , 'The index of the transaction is not correct,' + curIndex + ',' + nextIndex);
    let isNewProposal = (index === nextIndex);

    //New record
    if(isNewProposal){
        //1.Check the last one has been verified
        if(curIndex !== '0'){
            let curProposalHash = storageLoad(PROPOSAL_HISTORY + curIndex);
            assert(curProposalHash !== false, 'Failed to check the last proposal hash.');
            let curProposalSrc= storageLoad(curProposalHash);
            let curProposalObj = JSON.parse(curProposalSrc);
            assert(curProposalSrc !== false, 'Failed to check the last proposal.');
            assert(curProposalObj.index === curIndex, 'Failed to check the last proposal, index error' + curProposalObj.index + ',' + curIndex);
            assert(curProposalObj.executed === true, 'Failed to check proposal executed false.' + curProposalObj.index + ',' + curIndex);
        }

        //2.Check that all local records are empty
        assert(storageLoad(PROPOSAL_HISTORY + nextIndex) === false, 'Failed to load history');
        assert(storageLoad(mainChainTxHash) === false, 'Failed to load main chain trans hash');

        //3. Check if immediate execution is required
        let proposalValidators = [];
        proposalValidators.push([sender, 0]);
        let executed = false;
        if(checkUpdateSystemValidators(executed, proposalValidators, curValidators, addValidator, deleteValidator)){
            executed = true;
        }

        //4.Three new records were written
        let newProposalObj = {};
        newProposalObj.index = index;
        newProposalObj.executed = executed;
        newProposalObj.validators = proposalValidators;
        newProposalObj.add_validator = addValidator;
        newProposalObj.delete_validator = deleteValidator;
        storageStore(mainChainTxHash, JSON.stringify(newProposalObj));
        storageStore(PROPOSAL_HISTORY + nextIndex, mainChainTxHash);
        storageStore(PROPOSAL_NEW_INDEX, nextIndex);
        return;
    }

    //If it's an old record
    //1.Verify the last three records are valid
    assert(storageLoad(PROPOSAL_HISTORY + curIndex) !== false, 'Failed to check old history');
    let oldProposalStr = storageLoad(mainChainTxHash);
    assert(oldProposalStr !== false, 'Failed to check old proposal.');
    let oldProposal = JSON.parse(oldProposalStr);
    assert(oldProposal.index === curIndex, 'Failed to check old proposal index and cur index:' + oldProposal.index + ',' + curIndex);

    //2.Change the voting record and switch validation nodes
    assert(findValidatorIndex(oldProposal.validators, sender) === false, 'Failed to check sender duplicate submissions,');
    oldProposal.validators.push([sender, 0]);

    //3.Check if immediate execution is required
    if(checkUpdateSystemValidators(oldProposal.executed, oldProposal.validators, curValidators, addValidator, deleteValidator)){
        oldProposal.executed = true;
    }
    
    storageStore(mainChainTxHash, JSON.stringify(oldProposal));
    return;
}

function queryProposal(mainChainTxHash){
    let curIndex = storageLoad(PROPOSAL_NEW_INDEX);
    if(curIndex === '0'){
        return false;
    }

    if(mainChainTxHash === ''){
        mainChainTxHash = storageLoad(PROPOSAL_HISTORY + curIndex);
    }
    assert(typeof mainChainTxHash === 'string', 'Failed to check main chain tx hash, must be string');
    let proposalStr = storageLoad(mainChainTxHash);
    if(proposalStr === false){
        return false;
    }

    return JSON.parse(proposalStr);
}

function query(inputStr){
    let inputObj  = JSON.parse(inputStr);

    let result = {};
    if(inputObj.method === 'getValidators'){
        result.curValidators = getValidators();
    }
    else if(inputObj.method === 'queryProposal'){
		let main_hash = '';
		if(inputObj.params !== undefined){
			if(inputObj.params.main_chain_tx_hash !== undefined){
				main_hash = inputObj.params.main_chain_tx_hash;
			}
		}
        result.proposal = queryProposal(main_hash);
    }
    else{
       	throw '<unidentified operation type>';
    }

    log(result);
    return JSON.stringify(result);
}

function main(inputStr){
    //Check it is a valid verification node
    let curValidators = getValidators();
    assert(findValidatorIndex(curValidators, sender) !== false, 'Failed to check validator');
    let inputObj = JSON.parse(inputStr);
    if(inputObj.method === 'changeValidator'){
        let params = inputObj.params;
        changeValidators(curValidators, params.main_chain_tx_hash, params.add_validator, params.delete_validator, params.index);
    }
    else{
        throw '<undidentified operation type>';
    }
}

function init(){
    let validators = getValidators();
    assert(validators !== false, 'Get validators failed.');
    setValidators(JSON.stringify(validators));
    storageStore(PROPOSAL_NEW_INDEX, '0');
}
