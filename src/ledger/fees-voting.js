'use strict';
const voteRecordsKey = 'voteRecordsKey';
const enrollRecordsKey = 'enrollRecordsKey';
const statisticRecordsKey = 'statisticRecordsKey';
const victorsRecordsKey = 'victorsRecordsKey';
const voteRecordsDetailKeyPrefix ='voteRecords_';
let enrollRecords = {};
let voteRecords = {};
let statisticRecords = [];
let validators = [];


function storageSet(key, value) {
  let result = storageStore(key,JSON.stringify(value));
  if (result === false) {
    throw 'Storage set faild';
  }
}

function storageDelete(key) {
  let result = storageDel(key);
  if (result === false) {
    throw 'Storage del faild';
  }
}

function loadValidators() {
  let result = getValidators();
  if(result === false){
    throw 'getValidators failed';
  }
  validators = result;
}

function loadVoteRecords() {
  let result = storageLoad(voteRecordsKey);
  if (result === false) {
    return false;
  } 
  voteRecords = JSON.parse(result);
  return true;
}
function loadEnrollRecords() {
  let result = storageLoad(enrollRecordsKey);
  if (result === false) {
    return false;
  }
  enrollRecords = JSON.parse(result);
  return true;
}

function enrollExist(enrollID) {
  let exist = false;
  Object.keys(enrollRecords).every(
    function (key) {
      if (enrollRecords[key].enrollID === enrollID) {
        exist = true;
        return false;
      } else {
        return true;
      }
    }
  );
  return exist;
}

function accountEnrolled(enroll) {
  let enrolled = false;
  Object.keys(enrollRecords).every(
    function(key){
      if(enrollRecords[key].accountID === enroll.accountID){
        enrolled  = true;
        return false;
      }
      else{
        return true;
      }        
    }
  );
  return enrolled;
}

function delEnroll(accountID, enrollID) {
  log('delEnroll accountID:'+ accountID +' enrollID:'+enrollID);
  let be_del = false;
  if (enrollRecords[enrollID]) {
    if (enrollRecords[enrollID].accountID === accountID) {
      be_del = true;
      delete enrollRecords[enrollID];
      delete voteRecords[enrollID];
      let key=voteRecordsDetailKeyPrefix+enrollID;
      storageDelete(key);
      storageSet(voteRecordsKey,voteRecords);
      storageSet(enrollRecordsKey, enrollRecords);
    }
  }
  return be_del;
}

function doVoting(accountID,enrollID,voteID) {
  if (!(accountID && enrollID && voteID)) {
    throw 'doVoting parameter error';
  }

  if(loadEnrollRecords() === false){
    throw 'enroll records not exist';
  }
  if (!enrollExist(enrollID)) {
    throw 'Vote enroll(' + enrollID + ') not exist';
  }

  loadVoteRecords();
  let key = voteRecordsDetailKeyPrefix + enrollID;
  let voteRecordBody;
  if (voteRecords[enrollID]) {
    let result = storageLoad(key);
    if (result === false) {
      throw 'voteRecords exist key(' + enrollID + ') ,but not exist body';
    }
    voteRecordBody = JSON.parse(result);
    if (voteRecordBody[accountID]) {
      throw 'Account(' + accountID + ') have voted the enroll(' + enrollID + ')';
    }

    voteRecords[enrollID] = voteRecords[enrollID] + 1;
    voteRecordBody[accountID] = {'accountID':accountID,'enrollID':enrollID,'voteID':voteID };
    storageSet(key, voteRecordBody);


  } else {
    voteRecords[enrollID] = 1;
    voteRecordBody = {};
    voteRecordBody[accountID] =  {'accountID':accountID,'enrollID':enrollID,'voteID':voteID };
    storageSet(key, voteRecordBody);
  }

  storageSet(voteRecordsKey, voteRecords);
  return true;
}


function enrollFee(accountID,enrollID,feeType,price) {
  if(!(accountID && enrollID && feeType && price)){
    throw 'enrollFee parameter error';
  }
  loadEnrollRecords();
  let exist =false;
  Object.keys(enrollRecords).every(
    function(key){
      if(enrollRecords[key].accountID === accountID){
        exist =true;
        delEnroll(enrollRecords[key].accountID,enrollRecords[key].enrollID);
        enrollRecords[enrollID] = {'accountID':accountID,'enrollID':enrollID,'feeType':feeType,'price':price};
        //voteRecords[enrollID]=0;
        storageSet(enrollRecordsKey, enrollRecords);
        //storage_data(voteRecordsKey, voteRecords);
        return false;
      }
      else{
        return true;
      }        
    }
  );

  if(!exist){
    enrollRecords[enrollID] = {'accountID':accountID,'enrollID':enrollID,'feeType':feeType,'price':price};
    storageSet(enrollRecordsKey, enrollRecords);
  }
  loadVoteRecords();   
  return true;
}


function voteStatistic() {
  loadValidators();
  let thredhold = Math.floor((validators.length * 2 / 3) + 1);
  let victorsRecordsTmp = [];
  let statisticRecordsTmp = [];
  let output = {};

  Object.keys(voteRecords).every(
    function (enrollID) {
      let enroll = enrollRecords[enrollID];
      if (!validators.includes(enrollRecords[enrollID].accountID)) { return true; }

      if (statisticRecordsTmp[enroll.feeType]) {
        let s = statisticRecordsTmp[enroll.feeType].statistic;
        if (s[enrollID]) {
          s[enrollID].count = voteRecords[enrollID];

          if (s[enrollID].count >= thredhold) {
            victorsRecordsTmp[enroll.feeType] = { 'feeType': enroll.feeType, 'enrollID': enrollID, 'count': s[enrollID].count, 'price': enroll.price };
            output[enroll.feeType] = enroll.price;
          }
        }
      }
      else {
        let statistic = {};
        statistic[enrollID] = { 'enrollID': enrollID, 'price': enrollRecords[enrollID].price, 'count': voteRecords[enrollID] };
        statisticRecordsTmp[enroll.feeType] = { 'feeType': enroll.feeType, 'statistic': statistic };

        if (statistic[enrollID].count >= thredhold) {
          victorsRecordsTmp[enroll.feeType] = { 'feeType': enroll.feeType, 'enrollID': enrollID, 'count': statistic[enrollID].count, 'price': enroll.price };
          output[enroll.feeType] = enroll.price;
        }
      }
      return true;
    }
  );

  statisticRecords = statisticRecordsTmp;
  storageSet(statisticRecordsKey, statisticRecords);

  if (victorsRecordsTmp.length > 0) {
    storageSet(victorsRecordsKey, victorsRecordsTmp);

    victorsRecordsTmp.every(
      function (item) {
        let k = item.enrollID;
        if (k !== enrollRecords[k].enrollID) { throw 'enroll_records key(' + k + ') != enrollID(' + enrollRecords[k].enrollID + ')'; }
        delEnroll(enrollRecords[k].accountID, enrollRecords[k].enrollID);
        return true;
      }
    );

    configFee(JSON.stringify(output));
  }
}

function queryVoting(enrollID) {
  if(enrollID===null || enrollID===""){
    throw 'queryVoting parameter error';
  }
  loadVoteRecords();
  if(voteRecords[enrollID] ===null){
    throw 'vote_records not exist key(' + enrollID + ') vote';
  }
  let key =voteRecordsDetailKeyPrefix+enrollID;
  let vote_records_v={};
  let result = storageLoad(key);
  if(result === false) {
    throw 'vote_records not exist key(' + enrollID + ') vote';
  }

  return result;
}

function queryEnroll() {
  
  let result = storageLoad(enrollRecordsKey);
  if (result === false) {
    throw 'queryEnroll error';
  }  
  return result;
}

function main(input) {
  if (input === ''){throw 'main input is empty';}

  let para = JSON.parse(input);
  if (para.method === 'doVoting') {
    let p =para.params;
    doVoting(p.accountID,p.enrollID,p.voteID);
    voteStatistic();
  }
  else if (para.method === 'enrollFee') {
    let p =para.params;
    enrollFee(p.accountID,p.enrollID,p.feeType,p.price);
  }
  else {
    throw 'main input para error';
  }
}


function query(input) {
  if (input === ''){ throw 'query input is empty';}

  let para = JSON.parse(input);
  if (para.method === 'queryVoting') {
    let p =para.params;
    return queryVoting(p.enrollID);
  }
  else if (para.method === 'queryEnroll') {
    return queryEnroll();
  }
  else {
    throw 'query input para error';
  }
}
