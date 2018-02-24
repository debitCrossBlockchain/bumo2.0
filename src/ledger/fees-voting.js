'use strict';
const proposalRecordsKey = 'proposalRecordsKey';
const voteRecordKeyPrefix ='voteRecords_';
const nonceKey ='nonce';
const expireTime =15;
const microSecOfOneDay =1000000*60*60*24;
let proposalRecords = {};
let validators = {};


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
  let proposalRecordBody = {};

  let result = storageLoad(key);
  assert(result !== false,'proposalId('+proposalId+') not exist voteRecords');
  proposalRecordBody = JSON.parse(result);
  assert(!proposalRecordBody.hasOwnProperty(accountId),'Account(' + accountId + ') have voted the proposal(' + proposalId + ')'); 
  
  proposalRecords[proposalId].voteCount +=1;
  proposalRecordBody[accountId] = 1;

  let thredhold = Math.floor(Object.keys(validators).length*0.8);
  if(proposalRecords[proposalId].voteCount>=thredhold)
  {
    let output = {};
    output[proposalRecords[proposalId].feeType] = proposalRecords[proposalId].price;
    delete proposalRecords[proposalId];
    storageDel(key);   
    configFee(JSON.stringify(output));
  }
  else{
    storageStore(key,JSON.stringify(proposalRecordBody));
  }  
  storageStore(proposalRecordsKey, JSON.stringify(proposalRecords));
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
        proposalRecords[newProposalId] = {'accountId':accountId,'proposalId':newProposalId,'feeType':feeType,'price':price,'voteCount':0,'time':blockTimestamp };               
        storageStore(proposalRecordsKey,JSON.stringify(proposalRecords));
        storageStore(voteRecordKeyPrefix + newProposalId,JSON.stringify({}));
        return false;
      }
      else{
        return true;
      }        
    }
  );

  if (!exist) {
    proposalRecords[newProposalId] = { 'accountId': accountId, 'proposalId': newProposalId, 'feeType': feeType, 'price': price, 'voteCount': 0,'time':blockTimestamp };
    storageStore(proposalRecordsKey, JSON.stringify(proposalRecords));
    storageStore(voteRecordKeyPrefix + newProposalId,JSON.stringify({}));
  }  

  storageStore(nonceKey,nonce.toString());
}

function queryVote(proposalId) {
  let key =voteRecordKeyPrefix+proposalId;
  let result = storageLoad(key);
  //assert(result !== false,'vote records of proposal(' +proposalId +')not exist');
  if(result === false){
    result ='vote records of proposal(' +proposalId +')not exist';
  }
  return result;
}

function queryProposal() {  
  let result = storageLoad(proposalRecordsKey);
  //assert(result !== false,'proposal not exist');
  if(result === false){
    result ='proposal not exist';
  }
  return result;
}

function feeTypeCheck(feeType){
  assert(Number.isInteger(feeType) && feeType>0 && feeType<10,'feeType error');
}


function priceCheck(price){
  assert(typeof price === "string",'price is not string');
  Object.keys(price).every(
    function(i){
        assert(Number.isInteger(parseInt(price[i])),'price contain NaN char');
    }
  );
}

function checkExpire(){
  let proposalChange =false;
  let curTime =blockTimestamp;
  Object.keys(proposalRecords).every(
    function(proposalId){
      //let span = (curTime-proposalRecords[proposalId].time)/(1000000*60*60*24);
      let span = (curTime-proposalRecords[proposalId].time)/microSecOfOneDay;
      if(span>=expireTime){
        delete proposalRecords[proposalId];
        let key =voteRecordKeyPrefix + proposalId;
        storageDel(key); 
        proposalChange =true;
      }
      return true;
    }
  );
  if(proposalChange){
    storageStore(proposalRecordsKey, JSON.stringify(proposalRecords));
  }
}

function main(input) {
  let para = JSON.parse(input);
  if (para.method === 'voteFee') {
    assert(para.params.proposalId !==undefined,'params proposalId undefined');
    voteFee(para.params.proposalId);
    checkExpire();
  }
  else if (para.method === 'proposalFee') {
    assert(para.params.feeType !==undefined && para.params.price !==undefined,'params feeType price undefined');
    feeTypeCheck(para.params.feeType);
    priceCheck(para.params.price);
    proposalFee(para.params.feeType,para.params.price);
    checkExpire();
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
  else {
    throw 'query input para error';
  }
}

function init(){ storageStore(nonceKey,'0');}