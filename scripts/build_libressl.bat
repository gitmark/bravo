call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

"C:\Program Files (x86)\CMake\bin\cmake" verbose=1 -G "NMake Makefiles" -DCMAKE_C_FLAGS="-DPATH_MAX=260" -DCMAKE_C_FLAGS="/MD" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=install %1

nmake install

pause



