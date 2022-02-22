@echo off >nul 2>&1
rem() { "$@"; }
rem test -f nul && rm nul
rem false --help > /dev/null 2>&1 && false /mingw64/bin/clang "$@" || /mingw64/bin/clang "$@"
rem exit $?
 D:/mas/msys64/mingw64/bin/clang.exe %*
