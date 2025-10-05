#!/bin/bash

# TaskTown Final Packaging Script
# Creates distributable packages for different platforms

set -e

echo "TaskTown Final Packaging Script"
echo "==============================="

# Detect platform
PLATFORM="unknown"
if [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macOS"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="Linux"
fi

echo "Platform: $PLATFORM"

# Version info
VERSION="1.0.0"
BUILD_DATE=$(date +"%Y-%m-%d")
PACKAGE_NAME="TaskTown-v${VERSION}-${PLATFORM}"

echo "Version: $VERSION"
echo "Build Date: $BUILD_DATE"

# Build optimized version
echo ""
echo "Building optimized version..."
./build_optimized.sh

# Create package directory
PACKAGE_DIR="packages/${PACKAGE_NAME}"
echo ""
echo "Creating package: $PACKAGE_DIR"
rm -rf "packages"
mkdir -p "$PACKAGE_DIR"

# Copy executable and assets
if [[ "$PLATFORM" == "macOS" ]]; then
    if [ -d "build/TaskTown.app" ]; then
        echo "Copying macOS app bundle..."
        cp -r "build/TaskTown.app" "$PACKAGE_DIR/"
        
        # Create launcher script
        cat > "$PACKAGE_DIR/TaskTown.command" << 'EOF'
#!/bin/bash
cd "$(dirname "$0")"
./TaskTown.app/Contents/MacOS/TaskTown
EOF
        chmod +x "$PACKAGE_DIR/TaskTown.command"
    fi
elif [[ "$PLATFORM" == "Linux" ]]; then
    if [ -f "build/TaskTown" ]; then
        echo "Copying Linux executable..."
        cp "build/TaskTown" "$PACKAGE_DIR/"
        chmod +x "$PACKAGE_DIR/TaskTown"
    fi
fi

# Copy assets
if [ -d "assets" ]; then
    echo "Copying assets..."
    cp -r "assets" "$PACKAGE_DIR/"
fi

# Copy documentation
echo "Copying documentation..."
cp README.md "$PACKAGE_DIR/" 2>/dev/null || echo "README.md not found"
cp QUICK_START_ASSETS.md "$PACKAGE_DIR/" 2>/dev/null || echo "QUICK_START_ASSETS.md not found"
cp CAT_PROCESSING_GUIDE.md "$PACKAGE_DIR/" 2>/dev/null || echo "CAT_PROCESSING_GUIDE.md not found"

if [[ "$PLATFORM" == "Linux" ]]; then
    cp WINDOWS_SETUP.md "$PACKAGE_DIR/" 2>/dev/null || echo "WINDOWS_SETUP.md not found"
fi

# Copy image processing tools
if [ -f "process_cat.py" ]; then
    echo "Copying image processing tools..."
    cp process_cat.py "$PACKAGE_DIR/"
    mkdir -p "$PACKAGE_DIR/tools"
    cp tools/image_processor.py "$PACKAGE_DIR/tools/" 2>/dev/null || echo "image_processor.py not found"
fi

# Create version info file
cat > "$PACKAGE_DIR/VERSION.txt" << EOF
TaskTown Version Information
===========================

Version: $VERSION
Build Date: $BUILD_DATE
Platform: $PLATFORM
Build Type: Release (Optimized)

Performance Optimizations:
- Native CPU instruction optimizations
- Link-time optimization (LTO)
- Dead code elimination
- Aggressive compiler optimizations (-O3)

Features:
- Gamified productivity application
- Character-controlled interface
- Pomodoro timer integration
- Task management system
- Note-taking and organization
- Habit tracking
- Real-time performance monitoring
- Cross-platform compatibility
- Asset management system
- Comprehensive error handling

Controls:
- WASD/Arrow Keys: Move character
- E: Enter buildings
- ESC: Exit buildings
- F1: Toggle debug mode
- F5: Save game
- F9: Load game
- F11: Toggle performance monitor
- F12: Test error dialog

System Requirements:
- OpenGL 3.3+ compatible graphics
- 100MB available disk space
- 4GB RAM recommended
- 60Hz display recommended for optimal experience

For support and updates, visit: https://github.com/your-repo/TaskTown
EOF

# Create installation instructions
if [[ "$PLATFORM" == "macOS" ]]; then
    cat > "$PACKAGE_DIR/INSTALL.txt" << 'EOF'
TaskTown Installation Instructions (macOS)
=========================================

1. Extract this package to your desired location
2. Double-click TaskTown.app to run the application
   OR
   Double-click TaskTown.command for terminal output

If you get a security warning:
1. Right-click TaskTown.app and select "Open"
2. Click "Open" in the security dialog
3. The app will be trusted for future launches

Troubleshooting:
- If the app won't open, try running TaskTown.command
- Check Console.app for error messages
- Ensure you have macOS 10.15 or later

Image Processing:
- Use process_cat.py to process your cat sprite images
- Requires Python 3 with Pillow: pip3 install Pillow
EOF

elif [[ "$PLATFORM" == "Linux" ]]; then
    cat > "$PACKAGE_DIR/INSTALL.txt" << 'EOF'
TaskTown Installation Instructions (Linux)
=========================================

1. Extract this package to your desired location
2. Open a terminal in the extracted directory
3. Run: ./TaskTown

Dependencies:
The application should run on most modern Linux distributions.
If you encounter issues, ensure you have:
- OpenGL 3.3+ drivers
- X11 or Wayland display server
- Audio system (ALSA/PulseAudio)

Troubleshooting:
- If you get permission errors: chmod +x TaskTown
- For missing libraries, install: libgl1-mesa-glx libasound2-dev
- On Ubuntu/Debian: sudo apt install libgl1-mesa-glx libasound2-dev
- On CentOS/RHEL: sudo yum install mesa-libGL alsa-lib-devel

Image Processing:
- Use process_cat.py to process your cat sprite images
- Install Python dependencies: pip3 install Pillow
EOF
fi

# Create archive
echo ""
echo "Creating archive..."
cd packages
if command -v zip &> /dev/null; then
    zip -r "${PACKAGE_NAME}.zip" "${PACKAGE_NAME}"
    echo "Created: packages/${PACKAGE_NAME}.zip"
elif command -v tar &> /dev/null; then
    tar -czf "${PACKAGE_NAME}.tar.gz" "${PACKAGE_NAME}"
    echo "Created: packages/${PACKAGE_NAME}.tar.gz"
fi
cd ..

# Package summary
echo ""
echo "Package Summary:"
echo "================"
echo "Package: $PACKAGE_NAME"
echo "Location: packages/$PACKAGE_NAME"
if [ -f "packages/${PACKAGE_NAME}.zip" ]; then
    echo "Archive: packages/${PACKAGE_NAME}.zip"
    echo "Size: $(du -h "packages/${PACKAGE_NAME}.zip" | cut -f1)"
elif [ -f "packages/${PACKAGE_NAME}.tar.gz" ]; then
    echo "Archive: packages/${PACKAGE_NAME}.tar.gz"
    echo "Size: $(du -h "packages/${PACKAGE_NAME}.tar.gz" | cut -f1)"
fi

echo ""
echo "Contents:"
ls -la "packages/${PACKAGE_NAME}/"

echo ""
echo "Packaging complete!"
echo "Ready for distribution on $PLATFORM"