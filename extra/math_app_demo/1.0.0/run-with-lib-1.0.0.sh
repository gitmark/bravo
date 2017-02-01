#/bin/sh
cd install/bin
export DYLD_LIBRARY_PATH="../../../../math_lib_demo/1.0.0/install/lib:$PATH"
./math_app_demo
cd ../..
