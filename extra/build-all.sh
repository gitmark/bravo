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

#app_versions="1.0.0 1.1.0 1.2.0 2.0.0 3.0.0"
#lib_versions="1.0.0 1.0.1 1.1.0 1.2.0 2.0.0 3.0.0 3.1.0"

app_versions="4.0.0"
lib_versions="4.0.0 4.1.0"

for lib_version in $lib_versions; do
	build_it math_lib_demo/$lib_version
done

for app_version in $app_versions; do
	build_it math_app_demo/$app_version
done


