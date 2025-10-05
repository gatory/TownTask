#!/usr/bin/env fish
# Setting up for Windows environment credited to Claude.ai

echo "🎮 Setting up Chaotic-Order Development Environment"
echo "=================================================="

# Detect platform
set platform_name ""
if test -f /etc/lsb-release
    set platform_name "Ubuntu/Debian"
else if test (uname -s) = "Darwin"
    set platform_name "macOS" 
else if test (uname -s) = "Linux"
    set platform_name "Linux"
else
    set platform_name "Windows/Other"
end

echo "📋 Detected platform: $platform_name"

# Platform-specific dependency installation
switch $platform_name
    case "Ubuntu/Debian"
        echo "📦 Installing dependencies for Ubuntu/Debian..."
        sudo apt update
        sudo apt install -y build-essential git cmake pkg-config
        sudo apt install -y libasound2-dev mesa-common-dev libx11-dev libxrandr-dev libxi-dev xorg-dev libgl1-mesa-dev libglu1-mesa-dev
        
        # Install raylib
        if not pkg-config --exists raylib
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
        end
        
    case "macOS"
        echo "📦 Installing dependencies for macOS..."
        
        # Check if Homebrew is installed
        if not command -v brew > /dev/null
            echo "❌ Homebrew not found. Please install it first:"
            echo "   /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
            exit 1
        end
        
        # Install dependencies
        brew install raylib pkg-config
        echo "✅ Dependencies installed via Homebrew!"
        
    case "Linux"
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
        
    case "Windows/Other"
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
end

# Try to build the game
echo "🔨 Building the game..."
make clean
make

if test $status -eq 0
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
end
