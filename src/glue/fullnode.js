'use strict';

const minPledgeAmount = 10000 * 100000000;
const minImpeachSize = 8;

function transferCoin(dest, amount)
{
    assert((typeof dest === 'string') && (typeof amount === 'string'), 'Args type error. arg-dest and arg-amount must be a string.');
    if(amount === '0' || amount.startsWith('-')) { return true; }

    payCoin(dest, amount);
    log('Pay coin( ' + amount + ') to dest account(' + dest + ') succeed.');
}

function ipv4Check(ip) {
    let block = ip.split('.');
    assert(block.length === 4, 'Incorrect ip address');
    let i;
    for(i = 0; i < block.length; i += 1) {
        let sec = Number(block[i]);
        if (i === 0 && sec === 0) { 
            return false;
        }
        assert((sec >= 0 && sec <= 255), 'Incorrect ip address');
	}
    return true;
}

function verifyFullNodeInfo(applyInfo) {
    assert(typeof applyInfo.address === 'string' && typeof applyInfo.endpoint === 'string', 'Parameters type error');
    assert(addressCheck(applyInfo.address) === true, 'Invalid apply address');
    assert(ipv4Check(applyInfo.endpoint.split(':')[0]) === true, 'Invalid endpoint');
	
    let comp = int64Compare(thisPayCoinAmount, minPledgeAmount);
    assert(comp === 1 || comp === 0, 'Pledge coin amount must more than ' + minPledgeAmount);
    
    return true;
}

function applyAsFullNode(params){
    // Verify apply info
    verifyFullNodeInfo(params);
	
    // Add timestamp and last check time
    let fullnode = {};
    fullnode.addr = params.address;
    fullnode.endpoint = params.endpoint;
    fullnode.apply_time = blockTimestamp;
    fullnode.last_check_time = blockTimestamp;
    fullnode.pledge_amount = thisPayCoinAmount;
    fullnode.impeach_list = [];
	
    let fullnodeStr = JSON.stringify(fullnode);
    assert(setFullNode(fullnode, 'add') === true, 'Failed to add full node info');
    log('Set new full node (' + fullnodeStr + ') succeed.');
} 

function verifyImpeachInfo(impeachedAddr, impeachInfo){
    assert(typeof impeachInfo.ledger_seq === 'string' && 
           typeof impeachInfo.result === 'string',
           'Parameters type error');
    
    assert(addressCheck(impeachedAddr), 'Invalid impeach address.');
    
    let comp = int64Compare(blockNumber, int64Add(impeachInfo.ledger_seq, 1));
    assert(comp === 0, 'Impeach on ledger seq: ' + impeachInfo.ledger_seq + ', latest ledger seq: ' + blockNumber);
    let reason = impeachInfo.reason;
    assert(reason === 'out-sync' || reason === 'timeout', 'Invalid impeach reason');
	
    assert(verifyCheckAuthority(sender, impeachedAddr) === true, 'Verify check authority failed');
    
    return true;
}

function impeachFullNode(params){
    let impeachedAddr = params.address;
    let impeachInfo = params.impeach;
    verifyImpeachInfo(impeachedAddr, impeachInfo);
	
    let fullnode = getFullNode(impeachedAddr);
    assert(typeof fullnode === 'object', 'Failed to get full node info of ' + impeachedAddr);
    
    let impeach_array = [];
    impeach_array = fullnode.impeach_list;
    
    let impeach_valid = true;
    let i;
    for(i = 0; i < impeach_array.length; i += 1) {
        let item = impeach_array[i];
        let address = Object.keys(item)[0];
        let info = item[address];
        if (sender === address && impeachInfo.ledger_seq < info.ledger_seq) {
            log('Ignore invalid impeach info from address ' + sender);
            impeach_valid = false;
            break;
        }
    }
    if (impeach_valid === true) {
        let impeach = {};
        impeach[sender] = impeachInfo;
        impeach_array.push(impeach);
    
        if(impeach_array.length >= 8) {
            assert(setFullNode(fullnode, 'remove') === true, 'Failed to remove invalid full node');
        }
        assert(setFullNode(fullnode, 'update') === true, 'Failed to update full node ' + impeachedAddr);
    }
    return true;
}

function unimpeachFullNode(input.params) {
	let impeachedAddr = params.address;
    let unimpeachAddr = params.unimpeachAddr;
    assert(verifyCheckAuthority(sender, impeachedAddr) === true, 'Verify check authority failed');
	
	let fullnode = getFullNode(impeachedAddr);
    assert(typeof fullnode === 'object', 'Failed to get full node info of ' + impeachedAddr);
	
	let i;
    for(i = 0; i < fullnode.impeach_list.length; i += 1) {
        let item = fullnode.impeach_list[i];
        let address = Object.keys(item)[0];
        if(address === unimpeachAddr) {
			break;
        }
    }
	if(i !== impeach_array.length) {
		fullnode.impeach_list.splice(i, 1);
		assert(setFullNode(fullnode, 'update') === true, 'Failed to update full node ' + impeachedAddr);
	}
}

function query(input_str){
    let input  = JSON.parse(input_str);

    let result = {};
    if(input.method === 'getFullNode'){
        result.info = getFullNode(input.address);
    }
    else{
       	throw '<unidentified operation type>';
    }

    log(result);
    return JSON.stringify(result);
}

function main(input_str){
    let input = JSON.parse(input_str);
	let address = input.params.address;
    
    if(input.method === 'apply'){
        assert(typeof input.params.address === 'string' &&
               typeof input.params.endpoint === 'string',
               'Arg-endpoint and arg-address should be string');
        applyAsFullNode(input.params);
        tlog('apply', sender + ' apply as fullnode succeed');
    }
    else if(input.method === 'impeach'){
        assert(typeof input.params.address === 'string', 'Arg-address should be string');
        assert(typeof input.params.impeach === 'object', 'Arg-impeach should be object');
        impeachFullNode(input.params);
        tlog('impeach', sender + ' impeach ' + input.params.address + ' succeed');
	}
	else if(input.method === 'unimpeach'){
		assert(typeof input.params.address === 'string', 'Arg-address should be string');
		assert(typeof input.params.unimpeach_addr === 'string', 'Arg-impeach should be string');
		unimpeachFullNode(input.params);
		tlog('Unimpeach', sender + ' unimpeach ' + input.params.unimpeach_addr + ' from ' + input.params.address + ' succeed');
    else{
        throw '<unidentified operation type>';
    }
}

function init(){
    return true;
}
