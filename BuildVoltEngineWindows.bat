@echo off
setlocal

REM ================================
REM ===== USER CONFIGURATION =======
REM ================================

REM Set your Qt installation path (EDIT THIS)
set "QT_PATH=C:\Users\Aaron\Main Storage\Projects\VulkanProjects\ThirdPartyLibraries\Qt\6.10.0\msvc2022_64"

REM Project name (must match add_executable name)
set PROJECT_NAME=VoltEngine

set BUILD_DIR=VoltEngineBuild_Windows

REM Build type: Debug or Release
set BUILD_TYPE=Debug

REM ================================
REM ===== INTERNAL SETTINGS ========
REM ================================

set GENERATOR=Visual Studio 17 2022

echo.
echo ========================================
echo Configuring CMake...
echo ========================================
echo.

cmake -G "%GENERATOR%" ^
 -B "%BUILD_DIR%" ^
 -DCMAKE_PREFIX_PATH="%QT_PATH%" ^
 -DQt6_DIR="%QT_PATH%\lib\cmake\Qt6"

if %errorlevel% neq 0 (
    echo CMake configuration failed.
    exit /b %errorlevel%
)

echo.
echo ========================================
echo Building project (%BUILD_TYPE%)...
echo ========================================
echo.

cd "%BUILD_DIR%"

cmake --build . --config %BUILD_TYPE%

if %errorlevel% neq 0 (
    echo Build failed.
    exit /b %errorlevel%
)

echo.
echo ========================================
echo Running windeployqt...
echo ========================================
echo.

set EXE_PATH=%CD%\%BUILD_TYPE%\%PROJECT_NAME%.exe

if not exist "%EXE_PATH%" (
    echo Could not find built executable:
    echo %EXE_PATH%
    exit /b 1
)

"%QT_PATH%\bin\windeployqt.exe" --no-translations "%EXE_PATH%"

if %errorlevel% neq 0 (
    echo windeployqt failed.
    exit /b %errorlevel%
)

echo.
echo ========================================
echo DONE.
echo Executable located at:
echo %EXE_PATH%
echo ========================================
echo.

endlocal
pause