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
		#git fetch
		git fetch --all;

		#git reset 
		git reset --hard origin/release/1.0.0.0
		git pull
        echo "$git fetch ok"
    fi
fi

version=`git log |grep commit |head -1`
echo 'version: ' + $version

#get shortest hash
v=${version:7:7}

#make 
make bumo_version=$v

cd bin/

tar czvf bumo-$DATE.tar.gz bumo





