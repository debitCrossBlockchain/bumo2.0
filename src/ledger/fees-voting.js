'use strict';
const proposalRecordsKey = 'proposalRecordsKey';
const voteRecordKeyPrefix ='voteRecords_';
const nonceKey ='nonce';
const passRate = 0.7;
const effectiveProposalInterval =15*1000000*60*60*24;
let proposalRecords = {};
let validators = {};

const cfgProposalRecordsKey = 'cfgProposalRecordsKey';
const cfgVoteRecordKeyPrefix ='cfgVoteRecords_';
const cfgNonceKey = 'cfgNonce';
let cfgProposalRecords = {};
let candidates = {};


function loadValidators() {
  let result = getValidators();
  assert(result !== false,'getValidators failed');
  validators = result;
  assert(Object.keys(validators).length !==0,'validators is empty');
}


function loadProposalRecords() {
  let result = storageLoad(proposalRecordsKey);
  if (result === false) {
    return false;
  }
  proposalRecords = JSON.parse(result);
  return true;
}

function isValidator(accountId){
  let found =false;
  validators.every(
    function(item){
      if(item[0] ===accountId) {
        found =true;
        return false;
      }
      return true;
    }
  );
  assert(found,accountId +' is not validator');
}

function voteFee(proposalId) {
  let accountId =sender;
  loadValidators();
  isValidator(accountId);
  if(loadProposalRecords() === false){
    throw 'proposal records not exist';
  }
  assert(proposalRecords.hasOwnProperty(proposalId),'Vote proposal(' + proposalId + ') not exist');

  let key = voteRecordKeyPrefix + proposalId;
  if(blockTimestamp>proposalRecords[proposalId].expireTime){
    delete proposalRecords[proposalId];
    storageStore(proposalRecordsKey, JSON.stringify(proposalRecords));
    storageDel(key);     
    return false;  
  }
  
  let proposalRecordBody = {};
  let result = storageLoad(key);
  assert(result !== false,'proposalId('+proposalId+') not exist voteRecords');
  proposalRecordBody = JSON.parse(result);
  assert(!proposalRecordBody.hasOwnProperty(accountId),'Account(' + accountId + ') have voted the proposal(' + proposalId + ')'); 
  
  proposalRecords[proposalId].voteCount +=1;
  proposalRecordBody[accountId] = 1;


  let thredhold =parseInt(Object.keys(validators).length * passRate + 0.5);
  if(proposalRecords[proposalId].voteCount >= thredhold) {
    let output = {};
    output[proposalRecords[proposalId].feeType] = proposalRecords[proposalId].price;
    delete proposalRecords[proposalId];
    storageDel(key);   
    configFee(JSON.stringify(output));
  }
  else {
    storageStore(key,JSON.stringify(proposalRecordBody));
  }  
  storageStore(proposalRecordsKey, JSON.stringify(proposalRecords));
  return true;
}

function proposalFee(feeType,price) {
  let accountId =sender;
  loadValidators();
  isValidator(accountId);

  let result =storageLoad(nonceKey);
  assert(result !==false,'load nonce failed');
  let nonce = parseInt(result);
  nonce+=1;
  let newProposalId =accountId + nonce;
  loadProposalRecords();
  let exist =false;
  Object.keys(proposalRecords).every(
    function(proposalId){
      if(proposalRecords[proposalId].accountId === accountId) {
        exist =true;
        delete proposalRecords[proposalId];
        let key =voteRecordKeyPrefix + proposalId;
        storageDel(key); 
        proposalRecords[newProposalId] = {'accountId':accountId,'proposalId':newProposalId,'feeType':feeType,'price':price,'voteCount':1,'expireTime':blockTimestamp+effectiveProposalInterval };               
        storageStore(proposalRecordsKey,JSON.stringify(proposalRecords));
        let v={};
        v[accountId] =1;
        storageStore(voteRecordKeyPrefix + newProposalId,JSON.stringify(v));
        return false;
      }
      else{
        return true;
      }        
    }
  );

  if (!exist) {
    proposalRecords[newProposalId] = { 'accountId': accountId, 'proposalId': newProposalId, 'feeType': feeType, 'price': price, 'voteCount': 1,'expireTime':blockTimestamp+effectiveProposalInterval };
    storageStore(proposalRecordsKey, JSON.stringify(proposalRecords));
    let v={};
    v[accountId] =1;
    storageStore(voteRecordKeyPrefix + newProposalId,JSON.stringify(v));
  }  

  storageStore(nonceKey,nonce.toString());
}

function queryVote(proposalId) {
  let key =voteRecordKeyPrefix+proposalId;
  let result = storageLoad(key);
  if(result === false){
    result ='vote records of proposal(' +proposalId +')not exist';
  }
  return result;
}

function queryProposal() {  
  let result = storageLoad(proposalRecordsKey);
  if(result === false){
    result ='No proposal of the fee configuration exist';
  }
  return result;
}


function loadCandidates() {
  let result = getCandidates();
  assert(result !== false,'getCandidates failed');
  candidates = result;
  assert(Object.keys(candidates).length !==0, 'candidates is empty');
}

function loadCfgProposalRecords() {
  let result = storageLoad(cfgProposalRecordsKey);
  if (result === false) {
    return false;
  }
  cfgProposalRecords = JSON.parse(result);
  return true;
}

function isCandidate(accountId){
  let found =false;
  candidates.every(
    function(item){
      if(item[0] ===accountId) {
        found =true;
        return false;
      }
      return true;
    }
  );
  assert(found,accountId +' is not candidate');
}

function queryCfgVote(proposalId) {
  let key =cfgVoteRecordKeyPrefix+proposalId;
  let result = storageLoad(key);
  if(result === false){
    result ='vote records of proposal(' +proposalId +')not exist';
  }
  return result;
}

function queryCfgProposal() {  
  let result = storageLoad(cfgProposalRecordsKey);
  if(result === false){
    result ='No proposal of the election configuration exist';
  }
  return result;
}

function proposalCfg(params) {
  let accountId =sender;
  loadCandidates();
  isCandidate(accountId);

  let result =storageLoad(cfgNonceKey);
  assert(result !==false, 'Failed to load nonce');
  let nonce = parseInt(result);
  nonce+=1;
  let newProposalId =accountId + nonce;
  loadCfgProposalRecords();
  let exist =false;
  Object.keys(cfgProposalRecords).every(
    function(proposalId){
      if(cfgProposalRecords[proposalId].accountId === accountId) {
        exist =true;
        delete cfgProposalRecords[proposalId];
        let key =cfgVoteRecordKeyPrefix + proposalId;
        storageDel(key); 
        cfgProposalRecords[newProposalId] = {'accountId':accountId,'proposalId':newProposalId, 'configuration':params.configuration, 'voteCount':1,'expireTime':blockTimestamp+effectiveProposalInterval };               
        storageStore(cfgProposalRecordsKey,JSON.stringify(cfgProposalRecords));
        let v={};
        v[accountId] =1;
        storageStore(cfgVoteRecordKeyPrefix + newProposalId,JSON.stringify(v));
        return false;
      }
      else{
        return true;
      }
    }
  );

  if (!exist) {
    cfgProposalRecords[newProposalId] = { 'accountId': accountId, 'proposalId': newProposalId, 'configuration':params.configuration, 'voteCount': 1,'expireTime':blockTimestamp+effectiveProposalInterval };
    storageStore(cfgProposalRecordsKey, JSON.stringify(cfgProposalRecords));
    let v={};
    v[accountId] =1;
    storageStore(cfgVoteRecordKeyPrefix + newProposalId,JSON.stringify(v));
  }  

  storageStore(cfgNonceKey,nonce.toString());
}

function voteCfg(proposalId) {
  let accountId =sender;
  loadCandidates();
  isCandidate(accountId);
  if(loadCfgProposalRecords() === false){
    throw 'proposal records not exist';
  }
  assert(cfgProposalRecords.hasOwnProperty(proposalId),'Vote proposal(' + proposalId + ') not exist');

  let key = cfgVoteRecordKeyPrefix + proposalId;
  if(blockTimestamp>cfgProposalRecords[proposalId].expireTime){
    delete cfgProposalRecords[proposalId];
    storageStore(cfgProposalRecordsKey, JSON.stringify(cfgProposalRecords));
    storageDel(key);     
    return false;  
  }
  
  let proposalRecordBody = {};
  let result = storageLoad(key);
  assert(result !== false,'proposalId('+proposalId+') not exist voteRecords');
  proposalRecordBody = JSON.parse(result);
  assert(!proposalRecordBody.hasOwnProperty(accountId),'Account(' + accountId + ') have voted the proposal(' + proposalId + ')'); 
  
  cfgProposalRecords[proposalId].voteCount +=1;
  proposalRecordBody[accountId] = 1;


  let thredhold =parseInt(Object.keys(validators).length * passRate + 0.5);
  if(cfgProposalRecords[proposalId].voteCount >= thredhold) {
    let output = {};
    output[cfgProposalRecords[proposalId].feeType] = cfgProposalRecords[proposalId].price;
    delete cfgProposalRecords[proposalId];
    storageDel(key);   
    configElectionCfg(JSON.stringify(output));
  }
  else {
    storageStore(key,JSON.stringify(proposalRecordBody));
  }  
  storageStore(cfgProposalRecordsKey, JSON.stringify(cfgProposalRecords));
  return true;
	
}

function main(input) {
  let para = JSON.parse(input);
  if (para.method === 'voteFee') {
    assert(para.params.proposalId !==undefined,'params proposalId undefined');
    voteFee(para.params.proposalId);
  }
  else if (para.method === 'proposalFee') {
    assert(para.params.feeType !==undefined && para.params.price !==undefined,'params feeType price undefined');
    assert(Number.isInteger(para.params.feeType) && para.params.feeType>0 && para.params.feeType<3,'feeType error');
    assert(Number.isSafeInteger(para.params.price) && para.params.price>=0,'price should be int type and price>=0');
    proposalFee(para.params.feeType,para.params.price);
  }
  else if (para.method === 'proposalCfg') {
	assert(typeof para.params.configuration === 'object' && para.params.configuration !== null);
	assert(typeof para.params.comments === 'string', 'Arg-comments should be string');
	proposalCfg(para.params);
  }
  else if (para.method === 'voteCfg') {
	assert(typeof para.params.proposalId === 'string', 'Arg-proposalId should be string');
	voteCfg(para.params.proposalId);  
  }
  else {
    throw 'main input para error';
  }
}

function query(input) {
  let para = JSON.parse(input);
  if (para.method === 'queryVote') {    
    assert(para.params.proposalId !==undefined ,'params.proposalId undefined');
    return queryVote(para.params.proposalId);
  }
  else if (para.method === 'queryProposal') {
    return queryProposal();
  }
  else if (para.method === 'queryCfgProposal') {
	return queryCfgProposal();
  }
  else if (para.method === 'queryCfgVote') {
	assert(typeof para.params.proposalId === 'string', 'Arg-comments should be string'); 
	return queryCfgVote(para.params.proposalId); 
  }
  else {
    throw 'query input para error';
  }
}

function init(){ storageStore(nonceKey,'0');}
