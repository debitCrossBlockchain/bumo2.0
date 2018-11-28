'use strict';

function testdelegatecaller(a_number,b_number){
    log('testdelegatecaller');
    let calleraddress ='buQfjY4tRr6DdC1AKAgUKenrMkDP3E4P2A6W';
    let method = 'addTestInfo';
    let params = {};
    params.a_number = a_number;
    params.b_number = b_number;
    let retorigh = int64Add(a_number,b_number);
    let ret = delegateCall(calleraddress,method,JSON.stringify(params));
    tlog('testdelegatecaller',String(ret) + ',' + String(retorigh));
}

function init(inputStr){
    log('init');
}

function main(inputStr){
    let input = JSON.parse(inputStr);
    if(input.method === 'testdelegatecaller'){
        // assert(false, 'input.a_number:' + String(input.a_number) + ',input.b_number:' + String(input.b_number));
        testdelegatecaller(input.a_number,input.b_number);
    }
    else{
        throw '<Main interface passes an invalid operation type>';
    }
}

function query(input_str){
    let input  = JSON.parse(input_str);
    let result = {};
    log(result);
    return JSON.stringify(result);
}
