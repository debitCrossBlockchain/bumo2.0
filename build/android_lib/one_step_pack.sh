#/bin/bash

#git fetch --all;
#git reset --hard origin/release/1.0.0.0
#git reset --hard origin/develop

cd ../
./install-build-deps-android.sh

cd -

DATE=$(date +%Y_%m%d_%H%M)

cd ../../
rm -rf pack/

cd -

echo 'build 3rd...'
sh 3rd_android.sh

echo "build bumo..."
ndk-build clean
ndk-build -j 8


version=`git log |grep commit |head -1`
echo 'version: ' + $version

#get shortest hash
v=${version:7:7}

#make 

cd ../../
mkdir -p pack
cd pack/
rm -rf bu/ 
mkdir bu
mkdir bu/buchain
mkdir bu/buchain/config
mkdir bu/buchain/data
mkdir bu/buchain/jslib
mkdir bu/buchain/bin
mkdir bu/buchain/log
mkdir bu/buchain/web
cp ../build/win32/jslib/jslint.js bu/buchain/jslib/
cp ../build/win32/config/bumo-single.json  bu/buchain/config/bumo.json
cp ../src/3rd/v8_target/android/*.bin bu/buchain/bin/
cp ../src/web/jslint/*  bu/buchain/web/


mkdir bu/demo
cp ../build/android_lib/demo/* bu/demo/ -rf
cp ../build/android_lib/libs/armeabi-v7a/lib* bu/demo/distribution/bu/lib/armeabi-v7a/

mkdir bu/include 
cp ../build/android_lib/interface/bu.h bu/include/

mkdir bu/lib 
cp ../build/android_lib/libs/armeabi-v7a/lib* bu/lib/

tar czvf bu-android-$DATE-$v.tar.gz bu/
rm -rf bu/ 

echo "build ok...."



