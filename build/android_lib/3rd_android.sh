echo "start make all 3rd"
echo "start make curl"
cd $BUMO_SRC_PATH/3rd/curl/
sh $BUMO_SRC_PATH/3rd/curl/build_all.sh
echo "end make curl"

echo "start make openssl"
echo "cd $BUMO_SRC_PATH/3rd/openssl/"
cd $BUMO_SRC_PATH/3rd/openssl/ 
sh build-android-openssl.sh
echo "end make openssl"




