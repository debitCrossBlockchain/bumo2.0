#!/bin/bash

for arch in armeabi-v7a 
do
    #chmod 777 -R $BUMO_SRC_PATH/3rd/curl/
    bash build_curl.sh $arch
    #chmod 777 -R $BUMO_SRC_PATH/3rd/curl/
    make clean
    make -j4
    #sudo make install
done
