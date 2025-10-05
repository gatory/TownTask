@echo off
REM TaskTown Run Script for Windows
REM Builds and runs TaskTown

echo Starting TaskTown...

REM Build first
call build.bat
if %errorlevel% neq 0 (
    echo Build failed, cannot run application
    pause
    exit /b 1
)

REM Try to run the executable
if exist "build\Release\TaskTown.exe" (
    echo Running TaskTown (Visual Studio build)...
    build\Release\TaskTown.exe
) else if exist "build\TaskTown.exe" (
    echo Running TaskTown (MinGW build)...
    build\TaskTown.exe
) else (
    echo ERROR: TaskTown executable not found
    echo Expected locations:
    echo   build\Release\TaskTown.exe
    echo   build\TaskTown.exe
    pause
    exit /b 1
)

pause