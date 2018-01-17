const vote_records_key = 'vote_records_key';
const enroll_records_key = 'enroll_records_key';
const statistic_records_key = 'statistic-records_key';
const victors_records_key = 'victors_records_key';
const vote_records_detail_key_prefix ='vote-records-';
var enroll_records = {};
var vote_records = {};
var statistic_records = [];
var victors_records = [];
var validators = [];


function Storage(key, value,del) {
  if(!del) del =false;
  var transaction =
    {
      'operations':
        [
          {
            'type': 'SET_METADATA',
            'set_metadata':
              {
                'key': key,
                'value': JSON.stringify(value),
                'delete_flag':del
              }
          }
        ]
    };
  var result = callBackDoOperation(transaction);
  if(result==false) throw 'Storage faild';
  return result
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

function del_expire_enroll(){
  var cur_seq =consensusValue.sequence;
  for(var enroll_id in enroll_records){
    if(cur_seq -enroll_records[enroll_id].start_seq>100){
      del_enroll(enroll_records[enroll_id].account,enroll_id)
    }
  }
}

function add_enroll(enroll) {
  enroll['start_seq'] =consensusValue.sequence;
  enroll_records[enroll.enroll_id] = enroll;
}

function del_enroll(account_id, enroll_id) {
  callBackLog('del_enroll account_id:'+ account_id +' enroll_id:'+enroll_id);
  var be_del = false;
  if (enroll_records[enroll_id]) {
    if (enroll_records[enroll_id].account == account_id) {
      be_del = true;
      delete enroll_records[enroll_id];
      delete vote_records[enroll_id];
      var key=vote_records_detail_key_prefix+enroll_id;
      var value='';
      Storage(key,value,true);
      Storage(vote_records_key, vote_records);
      Storage(enroll_records_key, enroll_records);
    }
  }
  return be_del;
}

function do_voting(para) {
  if(!(para.account && para.enroll_id && para.vote_id))
    throw 'do_voting parameter error';

  load_enroll_records();
  if (!enroll_exist(para.enroll_id)) {
    throw 'Vote enroll(' + para.enroll_id + ') not exist';
  }

  load_vote_records();
  var key=vote_records_detail_key_prefix+para.enroll_id;
  if(vote_records[para.enroll_id]){    
    var vote_records_v = callBackGetAccountMetaData(thisAddress, key);
    if(vote_records_v ==false) 
      throw 'vote_records exist key(' + para.enroll_id + ') ,but not exist body';
    else
    {
      if(vote_records_v[para.account]){
        throw 'Account(' + para.account + ') have voted the enroll(' + para.enroll_id + ')';
      }
      else{
        vote_records[para.enroll_id]=vote_records[para.enroll_id]+1;
        vote_records_v[para.account] =para;
        Storage(key,vote_records_v);
      }
    }
  }
  else{
    vote_records[para.enroll_id]=1;
    var vote_records_v ={};
    vote_records_v[para.account] =para;
    Storage(key,vote_records_v);
  }

  Storage(vote_records_key, vote_records);
  return true;
}


function enroll_fee(para) {
  if(!(para.account && para.enroll_id && para.fee_type && para.price))
    throw 'enroll_fee parameter error';

  load_enroll_records();
  if (account_enrolled(para)) {
    throw 'Account' + para.account + ') has enroll the fee type(' + para.fee_type + ')';
  }
  load_vote_records();
  add_enroll(para);
  Storage(enroll_records_key, enroll_records);
  return true;
}


function vote_statistic() {
  //load_victors_records();
  get_validators();
  var thredhold = Math.floor((validators.length * 2 / 3) + 1);
  var victors_records_tmp = [];
  var statistic_records_tmp = [];
  var output={};
  for (var enroll_id in vote_records) {

    var enroll =enroll_records[enroll_id];

    if (!validators.includes(enroll_records[enroll_id].account)) continue;

    if (statistic_records_tmp[enroll.fee_type]) {
      var s = statistic_records_tmp[enroll.fee_type].statistic;
      if (s[enroll_id]) {
        s[enroll_id].count = vote_records[enroll_id];
        callBackLog('thredhold:' + thredhold + ' count:' + s[enroll_id].count);

        if (s[enroll_id].count >= thredhold) {
          victors_records_tmp[enroll.fee_type] = { 'fee_type': enroll.fee_type, 'enroll_id': enroll_id, 'count': s[enroll_id].count, 'price': enroll.price };
          output[enroll.fee_type]=enroll.price;
          //victors_records[enroll.fee_type] =victors_records_tmp[enroll.fee_type];
        }
      }
    }
    else {
      var statistic = {};
      statistic[enroll_id] = { 'enroll_id': enroll_id, 'price': enroll_records[enroll_id].price, 'count': vote_records[enroll_id] };
      statistic_records_tmp[enroll.fee_type] = { 'fee_type': enroll.fee_type, 'statistic': statistic };

      if (statistic[enroll_id].count >= thredhold) {
        victors_records_tmp[enroll.fee_type] = { 'fee_type': enroll.fee_type, 'enroll_id': enroll_id, 'count': statistic[enroll_id].count, 'price': enroll.price };
        output[enroll.fee_type]=enroll.price;
        //victors_records[enroll.fee_type] =victors_records_tmp[enroll.fee_type];
      }
    }

    statistic_records = statistic_records_tmp;

    for (var i in victors_records_tmp) {
      var k = victors_records_tmp[i].enroll_id;
      if (k != enroll_records[k].enroll_id) throw 'enroll_records key(' + k + ') != enroll_id(' + enroll_records[k].enroll_id + ')';
      del_enroll(enroll_records[k].account, enroll_records[k].enroll_id);
    }
    Storage(statistic_records_key, statistic_records);
    if (victors_records_tmp.length > 0) {
      Storage(victors_records_key, victors_records_tmp);
      callBackConfigFee(output);
    }
  }
}

function query_voting(para) {
  if(para['enroll_id']==null)
    throw 'query_voting parameter error';
  load_vote_records();
  var key =vote_records_detail_key_prefix+para['enroll_id'];
  var vote_records_v = callBackGetAccountMetaData(thisAddress, key);
  if(vote_records_v ==false) 
    throw 'vote_records not exist key(' + para.enroll_id + ') vote';
  return vote_records_v;
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
    del_expire_enroll();
  }
  else if (para.enroll_fee) {
    enroll_fee(para.enroll_fee);
    del_expire_enroll();
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
    return query_voting(para.query_voting);
  }
  else if (para.query_enroll) {
    return query_enroll();
  }
  else {
    throw 'query input para error';
  }
}
