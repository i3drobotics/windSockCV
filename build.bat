@echo off

SETLOCAL
:: set working directory to script directory
SET initcwd=%cd%
SET scriptpath=%~dp0
cd %scriptpath:~0,-1%

SET build_path=%cd%\build
set "build_path_slash=%build_path:\=/%"

:: make build & install folders
mkdir %build_path%
mkdir install
:: build library
cd build
cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="../install" -DOpenCV_DIR="%build_path_slash%/../3rdparty/opencv-3.4.10/opencv/build" ..
:: install library
cmake --build . --config Release --target install

:: reset working directory
cd %initcwd%

:: complete message
echo Build complete.