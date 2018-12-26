#/bin/bash

cd mac/
rm -rf mac.zip
wget depends.bumocdn.com:36002/v8_target/mac.zip
down_md5_nums=`md5 mac.zip | cut -d ' ' -f4`
true_md5_nums="a9be6e38ba963abc3ff6406c3c38ec20"
echo $true_md5_nums
echo $down_md5_nums

if [ "$true_md5_nums"x != "$down_md5_nums"x ]; then  
	echo "$md5_nums error, true md5:" $true_md5_nums "down md5:" $down_md5_nums
	exit
fi

unzip mac.zip
rm -rf ../../src/3rd/v8_target/mac/ 
mv -f mac ../../src/3rd/v8_target/mac 
rm -rf mac.zip 
mkdir -p ../../bin/

cp ../../src/3rd/v8_target/mac/*.bin ../../bin/
cp ../../src/3rd/v8_target/mac/*.dat ../../bin/
