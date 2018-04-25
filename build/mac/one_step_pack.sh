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
mkdir bumo
mkdir bumo/config
mkdir bumo/data
mkdir bumo/jslib
mkdir bumo/bin
mkdir bumo/log
mkdir bumo/coredump
cp ../build/win32/jslib/jslint.js bumo/jslib/
cp ../build/win32/config/* bumo/config/
cp ../bin/bumo bumo/bin/
cp ../src/3rd/v8_target/mac/*.bin bumo/bin/
cp ../src/3rd/v8_target/mac/*.dat bumo/bin/

tar czvf bumo-$DATE.tar.gz bumo/
rm -rf bumo/ 

echo "build ok...."
