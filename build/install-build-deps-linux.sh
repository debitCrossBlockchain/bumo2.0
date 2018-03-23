#/bin/bash

cd linux/
wget bumo.chinacloudapp.cn:36002/v8_target/linux.zip
unzip linux.zip
rm ../../src/3rd/v8_target/linux/ -rf
mv  linux ../../src/3rd/v8_target/linux/ -f
rm linux.zip -rf
mkdir -p ../../bin/

cp ../../src/3rd/v8_target/linux/*.bin ../../bin/
cp ../../src/3rd/v8_target/linux/*.dat ../../bin/
