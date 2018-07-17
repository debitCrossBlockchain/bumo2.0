#!/bin/bash

for arch in armeabi-v7a 
do
    chmod +x $BUMO_SRC_PATH/3rd/curl/configure
    bash build_curl.sh $arch
    make clean
    make -j4
done
