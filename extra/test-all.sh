#!/bin/sh

if ! [ -d math_app_demo ]; then
echo "error, math_app_demo not found"
exit 1
fi

app_versions="1.0.0 1.1.0 1.2.0 2.0.0"
lib_versions="1.0.1 1.1.0 1.2.0 2.0.0"

test_app_lib ()
{
app_version=$1
lib_version=$2
echo "[ app: $app_version, lib: $lib_version ]"
export DYLD_LIBRARY_PATH="math_lib_demo/$lib_version/install/lib"
export LD_LIBRARY_PATH=$DYLD_LIBRARY_PATH
echo "" > app_results.txt
math_app_demo/$app_version/install/bin/math_app_demo  > app_results.txt 2>&1
cat app_results.txt
echo ""
echo "with static linking:"
math_app_demo/$app_version/install/bin/math_app_demo_static > static_app_results.txt 2>&1
cat static_app_results.txt
echo ""

pass_count=`grep -e 'passed: [0-9]*, .*' app_results.txt | sed  -e 's/passed: \([0-9]*\).*$/\1/'`
fail_count=`grep -e 'passed: [0-9]*, failed: [0-9]*.*' app_results.txt | sed  -e 's/passed: \([0-9]*\), failed: \([0-9]*\).*$/\2/'`

echo "pass count: $pass_count"
echo "fail count: $fail_count"
echo ""

if [ "$fail_count" = "" ]; then
fail_count="1"
fi


  
}

test_app (){
app_version=$1
line=""
for lib_version in $lib_versions; do
	test_app_lib $app_version $lib_version
line="${line} $fail_count"
done
echo ""
echo "-----------------------------"
echo ""
echo $line >> matrix.txt
}

echo "" > matrix.txt
for app_version in $app_versions; do
	test_app $app_version
done




