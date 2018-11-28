'use strict';

function addTestInfo(a_number,b_number){
    tlog('delegate,addTestInfo,a_number:',a_number + ',b_number:' + b_number);
    let retnumber = a_number + b_number;
    return retnumber;
}

function init(inputStr){
    log('init');
}

function main(inputStr){
    let input = JSON.parse(inputStr);
    if(input.method === 'addTestInfo'){
        return addTestInfo(input.params.a_number,input.params.b_number);
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

function delegate(input_str){
    let input = JSON.parse(input_str);
    let result = {};
    tlog('delegate,input',input_str);
    if(input.method === 'addTestInfo'){
        tlog('delegate,addTestInfo',JSON.stringify(input.params));
        result = addTestInfo(input.params.a_number,input.params.b_number);
    }
    else{
        throw '<delegate interface passes an invalid operation type>';
    }
    tlog('delegate',JSON.stringify(result));
    return JSON.stringify(result);
}
