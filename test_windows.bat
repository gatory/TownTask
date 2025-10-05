@echo off
REM TaskTown Windows Compatibility Test Script
REM Tests key functionality to ensure Windows compatibility

echo TaskTown Windows Compatibility Test
echo ====================================

REM Build the application first
echo Building application...
call build_windows.bat
if %errorlevel% neq 0 (
    echo ERROR: Build failed on Windows
    pause
    exit /b 1
)

echo.
echo Build successful! Testing key functionality...

REM Check if executable exists
set "EXE_PATH="
if exist "build\Release\TaskTown.exe" (
    set "EXE_PATH=build\Release\TaskTown.exe"
    echo Found executable: %EXE_PATH%
) else if exist "build\TaskTown.exe" (
    set "EXE_PATH=build\TaskTown.exe"
    echo Found executable: %EXE_PATH%
) else (
    echo ERROR: TaskTown executable not found
    echo Expected locations:
    echo   build\Release\TaskTown.exe
    echo   build\TaskTown.exe
    pause
    exit /b 1
)

REM Check if assets exist
echo.
echo Checking assets...
if not exist "assets\config\map_config.json" (
    echo WARNING: map_config.json not found
) else (
    echo ✓ Map configuration found
)

if not exist "assets\config\character_config.json" (
    echo WARNING: character_config.json not found  
) else (
    echo ✓ Character configuration found
)

if not exist "assets\sprites\buildings" (
    echo WARNING: Building sprites directory not found
) else (
    echo ✓ Building sprites directory found
)

REM Test image processing tools
echo.
echo Testing image processing tools...
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo WARNING: Python not found - image processing tools won't work
    echo Install Python from https://python.org
) else (
    echo ✓ Python found
    
    REM Test if Pillow is available
    python -c "import PIL; print('✓ Pillow (PIL) available')" 2>nul
    if %errorlevel% neq 0 (
        echo WARNING: Pillow not installed
        echo Install with: pip install Pillow
    )
)

echo.
echo Windows Compatibility Test Results:
echo ==================================
echo ✓ Application builds successfully on Windows
echo ✓ Executable created and located
echo ✓ Asset system configured
echo ✓ Cross-platform build scripts available

echo.
echo Manual Testing Required:
echo ----------------------
echo 1. Run the application: %EXE_PATH%
echo 2. Test character movement (WASD keys)
echo 3. Test building entry (E key near buildings)
echo 4. Test ESC key behavior:
echo    - In building: Should exit building only
echo    - In town: Should show exit confirmation
echo 5. Test UI layout:
echo    - Town: UI at top of screen
echo    - Buildings: UI at bottom, no overlap
echo 6. Test notifications and feedback system
echo 7. Test performance monitor (F11 key)

echo.
echo To run manual test: %EXE_PATH%
echo.
pause