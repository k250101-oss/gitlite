@echo off
echo ========================================================
echo           Building GitLite Project (C++17)
echo ========================================================

where g++ >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] g++ compiler not found in PATH!
    exit /b 1
)

echo [1/2] Compiling main application (gitlite.exe)...
g++ -std=c++17 -O2 -Wall -Wextra -Iinclude src/main.cpp -o gitlite.exe
if %errorlevel% neq 0 (
    echo [ERROR] Failed to compile gitlite.exe!
    exit /b 1
)
echo [SUCCESS] Built gitlite.exe

echo [2/2] Compiling unit test runner (run_tests.exe)...
g++ -std=c++17 -O2 -Wall -Wextra -Iinclude tests/test_runner.cpp -o run_tests.exe
if %errorlevel% neq 0 (
    echo [ERROR] Failed to compile run_tests.exe!
    exit /b 1
)
echo [SUCCESS] Built run_tests.exe

echo ========================================================
echo Build complete! Run '.\gitlite.exe help' or '.\run_tests.exe'
echo ========================================================
