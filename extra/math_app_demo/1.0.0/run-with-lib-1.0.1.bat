@echo off
cd install\bin
set PATH=..\..\..\..\math_lib_demo\1.0.1\install\bin;%PATH%
math_app_demo.exe
cd ..\..
