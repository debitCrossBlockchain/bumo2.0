#!/bin/bash

for arch in armeabi-v7a 
do
    bash build_curl.sh $arch
    make clean
    make -j4
    sudo make install
done
