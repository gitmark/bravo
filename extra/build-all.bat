call ..\scripts\vcvars.bat


if NOT EXIST math_app_demo (
echo "error, math_app_demo not found"
exit 1
)
goto :main
:build_it 

cd %1
if EXIST build rmdir /s /q build

if EXIST install rmdir /s /q install

mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX=..\install -DCMAKE_BUILD_TYPE="Release" ..
nmake install

cd ..\..\..
exit /B 0
}

:main
rem app_versions="1.0.0 1.1.0 1.2.0 2.0.0 3.0.0"
rem lib_versions="1.0.0 1.0.1 1.1.0 1.2.0 2.0.0 3.0.0 3.1.0"

rem app_versions="4.0.0"
rem lib_versions="4.0.0 4.1.0"

rem for lib_version in $lib_versions; do
rem 	build_it math_lib_demo/$lib_version
rem done

rem for app_version in $app_versions; do
rem 	build_it math_app_demo/$app_version
rem done

call :build_it math_lib_demo\1.0.0
call :build_it math_app_demo\1.0.0


pause

