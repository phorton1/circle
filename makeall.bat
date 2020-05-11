rem prh prh prh
rem
rem This is makeall.bat for building circle on windows.
rem
rem I had to download a newer version of make (my minGw had 3.81)
rem from ftp://ftp.equation.com/make/64/make.exe

set VHIQ_SOUND=0


cd lib
make %1 %2
if %errorlevel% neq 0 exit /b %errorlevel%

    cd usb
    make %1 %2
    if %errorlevel% neq 0 exit /b %errorlevel%
    cd ..
    
    cd input
    make %1 %2
    if %errorlevel% neq 0 exit /b %errorlevel%
    cd ..
    
    cd fs
    make %1 %2
    if %errorlevel% neq 0 exit /b %errorlevel%
    
        cd fat
        make %1 %2
        if %errorlevel% neq 0 exit /b %errorlevel%
        cd ..

    cd ..

    cd sched
    make %1 %2
    if %errorlevel% neq 0 exit /b %errorlevel%
    cd ..

    cd net
    make %1 %2
    if %errorlevel% neq 0 exit /b %errorlevel%
    cd ..

    cd bt
    make %1 %2
    if %errorlevel% neq 0 exit /b %errorlevel%
    cd ..

cd ..

cd addon

    cd fatfs
    make %1 %2
    if %errorlevel% neq 0 exit /b %errorlevel%
    cd ..

    cd SDCard
    make %1 %2
    if %errorlevel% neq 0 exit /b %errorlevel%
    cd ..

    rem cd ugui
    rem make %1 %2
    rem if %errorlevel% neq 0 exit /b %errorlevel%
    rem cd ..
    rem 
    rem cd littlevgl
    rem make %1 %2
    rem if %errorlevel% neq 0 exit /b %errorlevel%
    rem cd ..
    
    if %VHIQ_SOUND%==0 goto NO_VHIQ_SOUND
    
    cd linux
    make %1 %2
    if %errorlevel% neq 0 exit /b %errorlevel%
    cd ..
    
    cd vc4

        cd vchiq
        make %1 %2
        if %errorlevel% neq 0 exit /b %errorlevel%
        cd ..
        
        cd sound
        make %1 %2
        if %errorlevel% neq 0 exit /b %errorlevel%
        cd ..

    cd ..
    
:NO_VHIQ_SOUND    
    
cd ..


cd _prh

    cd utils
    make %1 %2
    if %errorlevel% neq 0 cd ../..; exit /b %errorlevel%
    cd ..

    cd bootloader
    make %1 %2
    if %errorlevel% neq 0 cd ../..; exit /b %errorlevel%
    cd ..

cd ..


    