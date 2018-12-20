echo "install-build-deps-win32..........."


cd win32

rd /s /Q ".\win\"

.\wget depends.bumocdn.com:36002/v8_target/win.zip


set check_md5_ok=0
for /f "delims=" %%t in ('certutil -hashfile .\win.zip MD5') do if "%%t" == "e99789b892405e041301c5d15c04b959" (
		echo "md5 check ok "
		echo %%t
		set check_md5_ok=1
		dfadsf
	) 
	
	
if %check_md5_ok% equ 1 (
		echo "md5 check ok "
	) else (
		echo "check md5 error!" %check_md5_ok%
		exit
	)



.\unzip.exe -u win.zip

mkdir bin
mkdir dbin

copy ".\win\*" ".\dbin\"
copy ".\win\*" ".\bin\"

rd /s /Q ".\win\"

del /s /Q "win.zip"

cd ..

