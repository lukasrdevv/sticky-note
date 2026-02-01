@echo off
echo Building Sticky Note App...
g++ main.cpp -o sticky_note.exe -lgdi32 -luser32 -lshell32 -mwindows -O2 -s
if %errorlevel% neq 0 (
    echo Build Failed!
    pause
) else (
    echo Build Successful!
    echo Run sticky_note.exe to test.
)
