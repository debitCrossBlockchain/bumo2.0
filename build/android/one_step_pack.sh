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
rm -rf buchain/ 
mkdir buchain
mkdir buchain/config
mkdir buchain/data
mkdir buchain/jslib
mkdir buchain/bin
mkdir buchain/log
mkdir buchain/web
cp ../build/win32/jslib/jslint.js buchain/jslib/
cp ../build/win32/config/bumo-single.json  buchain/config/bumo.json
cp ../src/3rd/v8_target/android/*.bin buchain/bin/
cp ../build/android/libs/armeabi-v7a/* buchain/bin/
cp ../src/web/jslint/* buchain/web/

tar czvf buchain-android-$DATE-$v.tar.gz buchain/
rm -rf buchain/ 

echo "build ok...."



