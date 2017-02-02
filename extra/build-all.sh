#!/bin/sh

if ! [ -d math_app_demo ]; then
echo "error, math_app_demo not found"
exit 1
fi

build_it (){

cd $1
if [ -d build ]; then
rm -rf build
fi

if [ -d install ]; then
rm -rf install
fi

mkdir build
cd build

cmake -DCMAKE_INSTALL_PREFIX=../install -DCMAKE_BUILD_TYPE="Release" ..
make install

cd ../../..

}

build_it math_lib_demo/1.0.0
build_it math_lib_demo/1.0.1
build_it math_lib_demo/1.1.0
build_it math_lib_demo/1.2.0
build_it math_lib_demo/2.0.0

build_it math_app_demo/1.0.0
build_it math_app_demo/1.1.0
build_it math_app_demo/1.2.0
build_it math_app_demo/2.0.0



