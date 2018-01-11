const vote_records_key = 'vote_records_key';
const enroll_records_key = 'enroll_records_key';
const statistic_records_key = 'statistic-records_key';
const victors_records_key = 'victors_records_key';
var enroll_records = {};
var vote_records = {};
var statistic_records = [];
var victors_records = [];
var validators = [];


function Storage(key, value) {
  var transaction =
    {
      'operations':
        [
          {
            'type': 'SET_METADATA',
            'set_metadata':
              {
                'key': key,
                'value': JSON.stringify(value)
              }
          }
        ]
    };
  var result = callBackDoOperation(transaction);
  return true
}

function get_validators() {
  validators = callBackGetValidators();
}

function load_vote_records() {
  vote_records = callBackGetAccountMetaData(thisAddress, vote_records_key);
  if (vote_records == false) vote_records={};
  else vote_records = JSON.parse(vote_records.value);
  return true;
}

function load_enroll_records() {
  enroll_records = callBackGetAccountMetaData(thisAddress, enroll_records_key);
  if (enroll_records == false) enroll_records={};
  else enroll_records = JSON.parse(enroll_records.value);
  return true;
}

function load_statistic_records() {
  statistic_records = callBackGetAccountMetaData(thisAddress, statistic_records_key);
  if (statistic_records == false) statistic_records=[];
  else statistic_records = JSON.parse(statistic_records.value);
  return true;
}

function load_victors_records() {
  victors_records = callBackGetAccountMetaData(thisAddress, victors_records_key);
  if (victors_records == false) victors_records=[];
  else victors_records = JSON.parse(victors_records.value);
  return true;
}

function enroll_exist(enroll_id) {
  var enroll_id_exist = false;
  if (enroll_records[enroll_id]) {
    enroll_id_exist = true;
  }
  return enroll_id_exist;
}

function account_voted(para) {
  var voted = false;
  if(vote_records[para.vote_id]) throw 'Vote id('+para.vote_id + ') is exist';
  for (var key in vote_records) {
    if(vote_records[key].account == para.account && vote_records[key].enroll_id == para.enroll_id){
      voted = true;
      break;
    }
  }
  return voted;
}

function account_enrolled(enroll_fee) {
  var enrolled = false;
  for (var record in enroll_records) {
    if (record.account == enroll_fee.account && record.fee_type == enroll_fee.fee_type) {
      enrolled = true;
      break;
    }
  }
  return enrolled;
}

function add_vote(vote) {
  v = { 'account': vote.account, 'vote_id': vote.vote_id, 'enroll_id': vote.enroll_id, 'fee_type': enroll_records[vote.enroll_id].fee_type, 'price': enroll_records[vote.enroll_id].price }
  vote_records[vote.vote_id] = v;
}

function del_vote(account_id, vote_id) {
  var be_del = false;
  if (vote_records[vote_id]) {
    if (vote_records[vote_id].account == account_id) {
      be_del = true;
      delete vote_records[vote_id];
    }
  }
  return be_del;
}

function add_enroll(enroll) {
  enroll_records[enroll.enroll_id] = enroll;
}

function del_enroll(account_id, enroll_id) {
  callBackLog('del_enroll account_id:'+ account_id +' enroll_id:'+enroll_id);
  var be_del = false;
  if (enroll_records[enroll_id]) {
    if (enroll_records[enroll_id].account == account_id) {
      be_del = true;
      delete enroll_records[enroll_id];
      for (var k in vote_records) {
        if (vote_records[k].enroll_id == enroll_id) delete vote_records[k];
      }
    }
  }
  return be_del;
}

function do_voting(para) {
  load_enroll_records();
  if (!enroll_exist(para.enroll_id)) {
    throw 'Vote enroll(' + para.enroll_id + ') not exist';
  }

  load_vote_records();
  if (account_voted(para.account, para.enroll_id)) {
    throw 'Account(' + para.account + ') have voted the enroll(' + para.enroll_id + ')';
  }
  add_vote(para);
  Storage(vote_records_key, vote_records);
  return true;
}

function undo_voting(para) {
  load_vote_records();
  if (!del_vote(para.account, para.vote_id)) {
    throw 'Account(' + para.account + ') vote(' + para.vote_id + ') not exist';
  }
  load_enroll_records();
  Storage(vote_records_key, vote_records);
  return true;
}

function enroll_fee(para) {
  load_enroll_records();
  if (account_enrolled(para)) {
    throw 'Account' + para.account + ') has enroll the fee type(' + para.fee_type + ')';
  }
  add_enroll(para);
  Storage(enroll_records_key, enroll_records);
  return true;
}

function cancel_enroll_fee(para) {
  load_vote_records();
  load_enroll_records();
  if (!del_enroll(para.account, para.enroll_id)) {
    throw 'Account(' + para.account + ') enroll(' + para.enroll_id + ') not exist';
  }

  Storage(vote_records_key, vote_records);
  Storage(enroll_records_key, enroll_records);
  return true;
}

function vote_statistic() {
  get_validators();
  var thredhold = Math.floor((validators.length * 2 / 3) + 1);
  var victors_records_tmp = [];
  var statistic_records_tmp = [];
  for (var vote_id in vote_records) {
    var vote = vote_records[vote_id];
    if(!enroll_records[vote.enroll_id]) continue;    
    if(!validators.includes(enroll_records[vote.enroll_id].account)) continue;

    if (statistic_records_tmp[vote.fee_type]) {
      var s = statistic_records_tmp[vote.fee_type].statistic;
      if (s[vote.enroll_id]) {
        s[vote.enroll_id].count += 1;
        callBackLog('thredhold:'+thredhold+" count:"+s[vote.enroll_id].count);
        if (s[vote.enroll_id].count >= thredhold) {
          victors_records_tmp[vote.fee_type] = { 'fee_type': vote.fee_type, 'enroll_id': vote.enroll_id, 'count': s[vote.enroll_id].count, 'price': vote.price };
        }
      }
      else {
        s[vote.enroll_id] = { 'enroll_id': vote.enroll_id, 'price': vote.price, 'count': 1 };
      }
    }
    else {
      var statistic = {};
      statistic[vote.enroll_id] = { 'enroll_id': vote.enroll_id, 'price': vote.price, 'count': 1 };
      statistic_records_tmp[vote.fee_type] = { 'fee_type': vote.fee_type, 'statistic': statistic };
    }
  }

  victors_records = victors_records_tmp;
  statistic_records = statistic_records_tmp;

  var ve_change = false;
  for (var i in victors_records) {    
    var k =victors_records[i].enroll_id;
    callBackLog('delete victors enroll_id:'+victors_records[i].enroll_id);
    if(k !=enroll_records[k].enroll_id) throw 'enroll_records key('+k+') != enroll_id('+enroll_records[k].enroll_id+')';
    var ve_change =del_enroll(enroll_records[k].account ,enroll_records[k].enroll_id);
  }
  if (ve_change) {
    Storage(vote_records_key, vote_records);
    Storage(enroll_records_key, enroll_records);
  }  
  Storage(statistic_records_key, statistic_records);
  if(victors_records.length>0){
    Storage(victors_records_key, victors_records);
    callBackOutputLedger(victors_records);
  }
}

function query_voting() {
  load_vote_records();
  return vote_records;
}

function query_enroll() {
  load_enroll_records();
  return enroll_records;
}

function query_statistic() {
  load_statistic_records();
  return statistic_records;
}

function query_victors() {
  load_victors_records();
  return victors_records;
}

function main(input) {
  if (input == '')
    throw 'main input is empty'

  var para = JSON.parse(input);
  var need_statistic = true;
  result = {};
  if (para.do_voting) {
    do_voting(para.do_voting);
    vote_statistic();
  }
  else if (para.undo_voting) {
    undo_voting(para.undo_voting);
    vote_statistic();
  }
  else if (para.enroll_fee) {
    enroll_fee(para.enroll_fee);
  }
  else if (para.cancel_enroll_fee) {
    cancel_enroll_fee(para.cancel_enroll_fee);
    vote_statistic();
  }
  else {
    throw 'main input para error';
  }
}


function query(input) {
  if (input == '')
    throw 'query input is empty'

  var para = JSON.parse(input);
  if (para.query_voting) {
    return query_voting();
  }
  else if (para.query_enroll) {
    return query_enroll();
  }
  else if (para.query_statistic) {
    return query_statistic();
  }
  else if (para.query_victors) {
    return query_victors();
  }
  else {
    throw 'query input para error';
  }
}
