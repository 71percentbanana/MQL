@echo off
echo ===================================================
echo   Building SQL2Mongo Compiler and Execution Engine
echo ===================================================
gcc -std=c99 -Wall -Wextra src/*.c -o sql2mongo.exe
if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] sql2mongo.exe built successfully.
) else (
    echo [FAILED] Compilation errors encountered.
)
