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

set CURRENT_DATE_TIME_STAMP=%YEAR%%MONTH%%DAY%_%HOUR%%MINUTE%
echo %CURRENT_DATE_TIME_STAMP%

cd ../../
if "%1"=="git" (
		::echo "git fetch..."
		::git fetch --all
		::git reset --hard origin/release/1.0.0.0
		::git reset --hard origin/develop
		exit
		echo "$git error"
	) else echo NO 


cd build/win32

del output.log

"C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE\devenv.exe" /Clean "Release|Win32" Bumo.vs12.sln

"C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE\devenv.exe" Bumo.vs12.sln  /rebuild RELEASE /out output.log

cd bin
mkdir bumochain
mkdir bumochain\bin
mkdir bumochain\config
mkdir bumochain\data
mkdir bumochain\jslib
mkdir bumochain\log

copy Bumo.exe bumochain\bin
copy *.bin bumochain\bin
copy *.dat bumochain\bin
copy *.dll bumochain\bin
copy ..\config\* bumochain\config\
copy ..\jslib\jslint.js bumochain\jslib\

..\zip.exe -r bumochain-%CURRENT_DATE_TIME_STAMP%.zip bumochain

rd /s /Q ".\bumochain\"

cd ../
	
::"C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE\devenv.exe" /Clean "Release|Win32" Bumo.vs12.sln
::"C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE\devenv.exe" Bumo.vs12.sln /rebuild "Release|Win32" 