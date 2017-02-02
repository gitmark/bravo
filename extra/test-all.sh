#!/bin/sh

#!/bin/sh

if ! [ -d math_app_demo ]; then
echo "error, math_app_demo not found"
exit 1
fi

app_versions="1.0.0 1.1.0 1.2.0 2.0.0"
lib_versions="1.0.0 1.0.1 1.1.0 1.2.0 2.0.0"

test_app_lib (){
app_version=$1
lib_version=$2
echo "[ app: $app_version, lib: $lib_version ]"
export DYLD_LIBRARY_PATH="math_lib_demo/$lib_version/install/lib"
export LD_LIBRARY_PATH=$DYLD_LIBRARY_PATH
math_app_demo/$app_version/install/bin/math_app_demo
echo ""
echo "with static linking:"
math_app_demo/$app_version/install/bin/math_app_demo_static
echo ""
}

test_app (){
app_version=$1

for lib_version in $lib_versions; do
	test_app_lib $app_version $lib_version
done
echo ""
echo "-----------------------------"
echo ""
}

for app_version in $app_versions; do
	test_app $app_version 
done




