'use strict';

function createChildChain(){
    log('createChildChain');
    return true;
}

function queryChildBlockHeader(){
    log('queryChildBlockHeader');
    return '{"hello": 5}';
}

function init(inputStr){
    log('init');
}

function main(inputStr){
    let input = JSON.parse(inputStr);

    if(input.method === 'createChildChain'){
        createChildChain();
    }
    else{
        throw '<Main interface passes an invalid operation type>';
    }
}

function query(inputStr){
    let result = {};
    let input  = JSON.parse(inputStr);

    if(input.method === 'queryChildBlockHeader'){
        result.hello = JSON.parse(queryChildBlockHeader());
    }
    else{
        throw '<Query interface passes an invalid operation type>';
    }
    return JSON.stringify(result);
}
