'use strict';
const vote_records_key = 'vote_records_key';
const enroll_records_key = 'enroll_records_key';
const statistic_records_key = 'statistic-records_key';
const victors_records_key = 'victors_records_key';
const vote_records_detail_key_prefix ='vote_records_';
let enroll_records = {};
let vote_records = {};
let statistic_records = [];
let validators = [];


function storage_set(key, value) {
  let result = storageStore(key,JSON.stringify(value));
  if (result === false) {
    throw 'Storage set faild';
  }
}

function storage_del(key) {
  let result = storageDel(key);
  if (result === false) {
    throw 'Storage del faild';
  }
}

function get_validators() {
  let result = getValidators();
  if(result === false){
    throw 'getValidators failed';
  }
  validators = result;
}

function load_vote_records() {
  let result = storageLoad(vote_records_key);
  if (result === false) {
    return false;
  } 
  vote_records = JSON.parse(result);
  return true;
}
function load_enroll_records() {
  let result = storageLoad(enroll_records_key);
  if (result === false) {
    return false;
  }
  enroll_records = JSON.parse(result);
  return true;
}

function enroll_exist(enroll_id) {
  let exist = false;
  Object.keys(enroll_records).every(
    function (key) {
      if (enroll_records[key].enroll_id === enroll_id) {
        exist = true;
        return false;
      } else {
        return true;
      }
    }
  );
  return exist;
}

function account_enrolled(enroll) {
  let enrolled = false;
  Object.keys(enroll_records).every(
    function(key){
      if(enroll_records[key].account === enroll.account){
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

function del_enroll(account_id, enroll_id) {
  log('del_enroll account_id:'+ account_id +' enroll_id:'+enroll_id);
  let be_del = false;
  if (enroll_records[enroll_id]) {
    if (enroll_records[enroll_id].account === account_id) {
      be_del = true;
      delete enroll_records[enroll_id];
      delete vote_records[enroll_id];
      let key=vote_records_detail_key_prefix+enroll_id;
      storage_del(key);
      storage_set(vote_records_key,vote_records);
      storage_set(enroll_records_key, enroll_records);
    }
  }
  return be_del;
}

function do_voting(para) {
  if (!(para.account && para.enroll_id && para.vote_id)) {
    throw 'do_voting parameter error';
  }

  if(load_enroll_records() === false){
    throw 'enroll records not exist';
  }
  if (!enroll_exist(para.enroll_id)) {
    throw 'Vote enroll(' + para.enroll_id + ') not exist';
  }

  load_vote_records();
  let key = vote_records_detail_key_prefix + para.enroll_id;
  let vote_records_v;
  if (vote_records[para.enroll_id]) {
    let result = storageLoad(key);
    if (result === false) {
      throw 'vote_records exist key(' + para.enroll_id + ') ,but not exist body';
    }
    vote_records_v = JSON.parse(result);
    if (vote_records_v[para.account]) {
      throw 'Account(' + para.account + ') have voted the enroll(' + para.enroll_id + ')';
    }

    vote_records[para.enroll_id] = vote_records[para.enroll_id] + 1;
    vote_records_v[para.account] = para;
    storage_set(key, vote_records_v);


  } else {
    vote_records[para.enroll_id] = 1;
    vote_records_v = {};
    vote_records_v[para.account] = para;
    storage_set(key, vote_records_v);
  }

  storage_set(vote_records_key, vote_records);
  return true;
}


function enroll_fee(para) {
  if(!(para.account && para.enroll_id && para.fee_type && para.price)){
    throw 'enroll_fee parameter error';
  }
  load_enroll_records();
  let exist_same_account =false;
  Object.keys(enroll_records).every(
    function(key){
      if(enroll_records[key].account === para.account){
        exist_same_account =true;
        del_enroll(enroll_records[key].account,enroll_records[key].enroll_id);
        enroll_records[para.enroll_id] = para;
        //vote_records[para.enroll_id]=0;
        storage_set(enroll_records_key, enroll_records);
        //storage_data(vote_records_key, vote_records);
        return false;
      }
      else{
        return true;
      }        
    }
  );

  if(!exist_same_account){
    enroll_records[para.enroll_id] = para;
    storage_set(enroll_records_key, enroll_records);
  }
  load_vote_records();   
  return true;
}


function vote_statistic() {
  get_validators();
  let thredhold = Math.floor((validators.length * 2 / 3) + 1);
  let victors_records_tmp = [];
  let statistic_records_tmp = [];
  let output = {};

  Object.keys(vote_records).every(
    function (enroll_id) {
      let enroll = enroll_records[enroll_id];
      if (!validators.includes(enroll_records[enroll_id].account)) { return true; }

      if (statistic_records_tmp[enroll.fee_type]) {
        let s = statistic_records_tmp[enroll.fee_type].statistic;
        if (s[enroll_id]) {
          s[enroll_id].count = vote_records[enroll_id];
          log('thredhold:' + thredhold + ' count:' + s[enroll_id].count);

          if (s[enroll_id].count >= thredhold) {
            victors_records_tmp[enroll.fee_type] = { 'fee_type': enroll.fee_type, 'enroll_id': enroll_id, 'count': s[enroll_id].count, 'price': enroll.price };
            output[enroll.fee_type] = enroll.price;
            //victors_records[enroll.fee_type] =victors_records_tmp[enroll.fee_type];
          }
        }
      }
      else {
        let statistic = {};
        statistic[enroll_id] = { 'enroll_id': enroll_id, 'price': enroll_records[enroll_id].price, 'count': vote_records[enroll_id] };
        statistic_records_tmp[enroll.fee_type] = { 'fee_type': enroll.fee_type, 'statistic': statistic };

        if (statistic[enroll_id].count >= thredhold) {
          victors_records_tmp[enroll.fee_type] = { 'fee_type': enroll.fee_type, 'enroll_id': enroll_id, 'count': statistic[enroll_id].count, 'price': enroll.price };
          output[enroll.fee_type] = enroll.price;
          //victors_records[enroll.fee_type] =victors_records_tmp[enroll.fee_type];
        }
      }
      return true;
    }
  );

  statistic_records = statistic_records_tmp;
  storage_set(statistic_records_key, statistic_records);

  if (victors_records_tmp.length > 0) {
    storage_set(victors_records_key, victors_records_tmp);

    victors_records_tmp.every(
      function (item) {
        let k = item.enroll_id;
        if (k !== enroll_records[k].enroll_id) { throw 'enroll_records key(' + k + ') != enroll_id(' + enroll_records[k].enroll_id + ')'; }
        del_enroll(enroll_records[k].account, enroll_records[k].enroll_id);
        return true;
      }
    );

    configFee(JSON.stringify(output));
  }
}

function query_voting(para) {
  if(para.enroll_id===null || para.enroll_id===""){
    throw 'query_voting parameter error';
  }
  load_vote_records();
  let key =vote_records_detail_key_prefix+para.enroll_id;
  let vote_records_v={};
  let result = storageLoad(key);
  if(result === false) {
    throw 'vote_records not exist key(' + para.enroll_id + ') vote';
  }
  vote_records_v =JSON.parse(result);
  return vote_records_v;
}

function query_enroll() {
  load_enroll_records();
  return enroll_records;
}

function main(input) {
  if (input === ''){throw 'main input is empty';}

  let para = JSON.parse(input);
  if (para.do_voting) {
    do_voting(para.do_voting);
    vote_statistic();
  }
  else if (para.enroll_fee) {
    enroll_fee(para.enroll_fee);
  }
  else {
    throw 'main input para error';
  }
}


function query(input) {
  if (input === ''){ throw 'query input is empty';}

  let para = JSON.parse(input);
  if (para.query_voting) {
    return query_voting(para.query_voting);
  }
  else if (para.query_enroll) {
    return query_enroll();
  }
  else {
    throw 'query input para error';
  }
}
