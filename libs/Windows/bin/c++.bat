@echo off >nul 2>&1
rem() { "$@"; }
rem test -f nul && rm nul
rem false --help > /dev/null 2>&1 && false /mingw64/bin/c++ "$@" || /mingw64/bin/c++ "$@"
rem exit $?
 D:/mas/msys64/mingw64/bin/c++.exe %*
