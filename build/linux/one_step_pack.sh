#/bin/bash

DATE=$(date +%Y_%m%d_%H%M)

cd ../../

echo 'make clean'
make clean

echo "make clean build"
make clean_build


#update git
if [ -n "$1" ];then
    if [ "$1" == "git" ];then
        echo "git fetch..."
		#git fetch --all;
		#git reset --hard origin/release/1.0.0.0
		#git reset --hard origin/develop
        echo "$git fetch error...."
		
		exit
    fi
fi

version=`git log |grep commit |head -1`
echo 'version: ' + $version

#get shortest hash
v=${version:7:7}

#make 
make bumo_version=$v

cd bin/
mkdir bumochain
mkdir bumochain/config
mkdir bumochain/data
mkdir bumochain/jslib
mkdir bumochain/bin
mkdir bumochain/log
mkdir bumochain/coredump
cp ../build/win32/jslib/jslint.js bumochain/jslib/
cp ../build/win32/config/* bumochain/config/
cp bumo bumochain/bin/
cp ../src/3rd/v8_target/linux/*.bin bumochain/bin/
cp ../src/3rd/v8_target/linux/*.dat bumochain/bin/

tar czvf bumochain-$DATE.tar.gz bumochain/
rm bumochain/ -rf

echo "build ok...."



