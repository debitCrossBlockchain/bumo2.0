::git fetch --all
::git reset --hard origin/release/1.0.0.0
::git reset --hard origin/develop

set ORIGINAL_DATE=%date% 

set YEAR=%date:~0,4%
set MONTH=%date:~5,2%
set DAY=%date:~8,2%
set CURRENT_DATE=%YEAR%-%MONTH%-%DAY%


set HOUR=%time:~0,2%
set MINUTE=%time:~3,2%
set SECOND=%time:~6,2%


set TMP_HOUR=%time:~1,1%
set NINE=9
set ZERO=0

set CURRENT_DATE_TIME_STAMP=%YEAR%_%MONTH%%DAY%_%HOUR%%MINUTE%
echo %CURRENT_DATE_TIME_STAMP%

cd ../../
	

cd build/win32
rd /s /Q ".\pack\"
del output.log

"C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE\devenv.exe" /Clean "Release|Win32" Bumo.vs12.sln

"C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE\devenv.exe" Bumo.vs12.sln  /rebuild RELEASE /out output.log

mkdir pack
cd pack
mkdir buchain
mkdir buchain\bin
mkdir buchain\config
mkdir buchain\data
mkdir buchain\jslib
mkdir buchain\log

copy ..\bin\Bumo.exe buchain\bin
copy ..\bin\*.bin buchain\bin
copy ..\bin\*.dat buchain\bin
copy ..\bin\*.dll buchain\bin
copy ..\config\bumo-mainnet.json buchain\config\
copy ..\config\bumo-testnet.json buchain\config\
copy ..\config\bumo-single.json  buchain\config\
copy ..\config\ReadMe.txt  buchain\config\
copy ..\jslib\jslint.js buchain\jslib\

..\zip.exe -r buchain-win-%CURRENT_DATE_TIME_STAMP%.zip buchain

rd /s /Q ".\buchain\"

cd ../
	
