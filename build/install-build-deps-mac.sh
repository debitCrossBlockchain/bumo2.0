#/bin/bash

cd mac/
wget bumo.chinacloudapp.cn:36002/v8_target/mac.zip
unzip mac.zip
rm -rf ../../src/3rd/v8_target/mac/ 
mv -f mac ../../src/3rd/v8_target/mac/ 
rm -rf mac.zip 
