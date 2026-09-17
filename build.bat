@echo off
echo Building TrafficRule Compiler...
gcc -std=c99 -Wall -Wextra src/*.c -o trafficrule.exe
if %ERRORLEVEL% equ 0 (
    echo [SUCCESS] Build succeeded: trafficrule.exe
) else (
    echo [ERROR] Build failed!
)
