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
mkdir pack
cd pack/
mkdir buchain
mkdir buchain/config
mkdir buchain/data
mkdir buchain/jslib
mkdir buchain/bin
mkdir buchain/log
mkdir buchain/coredump
cp ../build/win32/jslib/jslint.js buchain/jslib/
cp ../build/win32/config/* buchain/config/
cp bumo buchain/bin/
cp ../src/3rd/v8_target/mac/*.bin buchain/bin/
cp ../src/3rd/v8_target/mac/*.dat buchain/bin/

tar czvf bumo-$DATE.tar.gz bumo/
rm -rf buchain/ 

echo "build ok...."
