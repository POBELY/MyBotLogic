
@echo off

cls

set pauseOpt=nopause

@echo off
call CheckInstall.bat CMAKE

if ErrorLevel == 1 (
    goto fail
)


SET bDirVar=build-solution-x64

echo Removing Old Build Directory

rd %bDirVar% /Q /S

mkdir %bDirVar%

cd %bDirVar%

if exist "%CMAKE_PATH%/cmake.exe" (
"%CMAKE_PATH%/cmake" .. -G "Visual Studio 15 2017 Win64"
goto generated
)

cmake .. -G "Visual Studio 15 2017 Win64"

:generated

cd ..

if NOT %pauseOpt% == nopause (
    pause
)

goto end:

:fail
    echo.
    echo Rebuild FAILED
    goto end

:end

:RetrievePath
    IF EXIST "TeamInfo.team" (
        for /f "delims== tokens=1,2" %%G in (TeamInfo.team) do set %%G=%%H
    )
    exit /B 0