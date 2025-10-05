#!/bin/bash
# Setting up for Mac environment credited to Claude.ai

echo "🎮 Setting up Chaotic-Order Development Environment"
echo "=================================================="

# Detect platform
if [[ -f /etc/lsb-release ]]; then
    PLATFORM="Ubuntu/Debian"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macOS"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="Linux"
else
    PLATFORM="Windows/Other"
fi

echo "📋 Detected platform: $PLATFORM"

# Platform-specific dependency installation
case $PLATFORM in
    "Ubuntu/Debian")
        echo "📦 Installing dependencies for Ubuntu/Debian..."
        sudo apt update
        sudo apt install -y build-essential git cmake pkg-config
        sudo apt install -y libasound2-dev mesa-common-dev libx11-dev libxrandr-dev libxi-dev xorg-dev libgl1-mesa-dev libglu1-mesa-dev
        
        # Install raylib
        if ! pkg-config --exists raylib; then
            echo "📚 Installing raylib from source..."
            git clone --depth 1 https://github.com/raysan5/raylib.git /tmp/raylib
            cd /tmp/raylib/src
            make PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=SHARED
            sudo make install RAYLIB_LIBTYPE=SHARED
            cd -
            rm -rf /tmp/raylib
            sudo ldconfig
            echo "✅ Raylib installed successfully!"
        else
            echo "✅ Raylib is already installed!"
        fi
        ;;
        
    "macOS")
        echo "📦 Installing dependencies for macOS..."
        
        # Check if Homebrew is installed
        if ! command -v brew &> /dev/null; then
            echo "❌ Homebrew not found. Please install it first:"
            echo "   /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
            exit 1
        fi
        
        # Install dependencies
        brew install raylib pkg-config
        echo "✅ Dependencies installed via Homebrew!"
        ;;
        
    "Linux")
        echo "📦 Detected Linux (non-Ubuntu). Please install raylib manually:"
        echo ""
        echo "For Arch Linux:"
        echo "   sudo pacman -S raylib"
        echo ""
        echo "For Fedora/CentOS:"
        echo "   sudo dnf install raylib-devel"
        echo "   # or: sudo yum install raylib-devel"
        echo ""
        echo "For other distros, build from source:"
        echo "   git clone https://github.com/raysan5/raylib.git"
        echo "   cd raylib/src && make && sudo make install"
        echo ""
        ;;
        
    "Windows/Other")
        echo "📦 Windows/Other platform detected."
        echo ""
        echo "For Windows with MSYS2/MinGW64:"
        echo "   pacman -S mingw-w64-x86_64-raylib"
        echo ""
        echo "For Windows with vcpkg:"
        echo "   vcpkg install raylib"
        echo ""
        echo "For other setups, please install raylib manually:"
        echo "   https://github.com/raysan5/raylib/wiki/Working-on-Windows"
        echo ""
        echo "⚠️  Skipping automatic setup for this platform."
        echo "    You may need to manually install raylib and run 'make' to build."
        exit 0
        ;;
esac

# Try to build the game
echo "🔨 Building the game..."
make clean
make

if [[ $? -eq 0 ]]; then
    echo ""
    echo "🎉 Setup complete! Your game is ready!"
    echo "   Run './build/game' or 'make run' to play!"
    echo ""
else
    echo ""
    echo "❌ Build failed. This might mean:"
    echo "   • Raylib is not properly installed"
    echo "   • Missing development tools (gcc, make, etc.)"
    echo "   • Platform-specific issues"
    echo ""
    echo "💡 Try building manually with 'make' to see detailed errors."
    exit 1
fi
