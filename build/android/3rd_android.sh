echo "start make all 3rd"
echo "rm -rf /bumo_3rd/"
rm -rf /bumo_3rd/
echo "mkdir -p /bumo_3rd/"
mkdir -p /bumo_3rd/
echo "start make curl"
echo "mkdir -p /bumo_3rd/curl"
mkdir -p /bumo_3rd/curl/
chmod 777 -R/bumo_3rd/curl/
echo "cd /bumo_3rd/curl/"
cd $BUMO_SRC_PATH/3rd/curl/
sh $BUMO_SRC_PATH/3rd/curl/build_all.sh

echo "end make curl"

echo "start make openssl"
echo "mkdir -p /bumo_3rd/openssl/"

mkdir -p /bumo_3rd/openssl/
chmod 777 -R /bumo_3rd/openssl/

echo "cd $BUMO_SRC_PATH/3rd/openssl/"
cd $BUMO_SRC_PATH/3rd/openssl/

sh build-android-openssl.sh

echo "end make openssl"




