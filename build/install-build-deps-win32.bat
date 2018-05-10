echo "install-build-deps-win32..........."


cd win32

rd /s /Q ".\win\"

.\wget bumo.chinacloudapp.cn:36002/v8_target/win.zip

certutil -hashfile .\win.zip MD5


.\unzip.exe -u win.zip

mkdir bin
mkdir dbin

copy ".\win\*" ".\dbin\"
copy ".\win\*" ".\bin\"

rd /s /Q ".\win\"

del /s /Q "win.zip"

cd ..

