#!/bin/bash
#
# PCIe Simulator - Dependency Installation Script
#
# Copyright (c) 2025 Karan Mamaniya <kmamaniya@gmail.com>
# Licensed under the MIT License
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

echo "PCIe Simulator - Installing Dependencies"
echo "========================================"

# Function to detect OS
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if command -v apt-get >/dev/null 2>&1; then
            echo "ubuntu"
        elif command -v yum >/dev/null 2>&1; then
            echo "rhel"
        elif command -v dnf >/dev/null 2>&1; then
            echo "fedora"
        elif command -v pacman >/dev/null 2>&1; then
            echo "arch"
        elif command -v apk >/dev/null 2>&1; then
            echo "alpine"
        else
            echo "linux-unknown"
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "win32" ]]; then
        echo "windows"
    else
        echo "unknown"
    fi
}

# Function to install dependencies on Ubuntu/Debian
install_ubuntu() {
    echo "Detected Ubuntu/Debian system"

    echo "Updating package lists..."
    sudo apt-get update

    echo "Installing build essentials..."
    sudo apt-get install -y \
        build-essential \
        gcc \
        g++ \
        make \
        libc6-dev

    echo "Installing kernel development headers..."
    KERNEL_VERSION=$(uname -r)
    sudo apt-get install -y linux-headers-${KERNEL_VERSION} || \
        sudo apt-get install -y linux-headers-generic

    echo "Installing additional development tools..."
    sudo apt-get install -y \
        pkg-config \
        git \
        tar \
        gzip

    echo "Dependencies installed successfully for Ubuntu/Debian"
}

# Function to install dependencies on RHEL/CentOS
install_rhel() {
    echo "Detected RHEL/CentOS system"

    echo "Installing development tools..."
    sudo yum groupinstall -y "Development Tools"
    sudo yum install -y \
        gcc \
        gcc-c++ \
        make \
        glibc-devel

    echo "Installing kernel development headers..."
    sudo yum install -y kernel-devel kernel-headers

    echo "Installing additional tools..."
    sudo yum install -y \
        pkgconfig \
        git \
        tar \
        gzip

    echo "Dependencies installed successfully for RHEL/CentOS"
}

# Function to install dependencies on Fedora
install_fedora() {
    echo "Detected Fedora system"

    echo "Installing development tools..."
    sudo dnf groupinstall -y "Development Tools" "C Development Tools and Libraries"
    sudo dnf install -y \
        gcc \
        gcc-c++ \
        make \
        glibc-devel

    echo "Installing kernel development headers..."
    sudo dnf install -y kernel-devel kernel-headers

    echo "Installing additional tools..."
    sudo dnf install -y \
        pkgconfig \
        git \
        tar \
        gzip

    echo "Dependencies installed successfully for Fedora"
}

# Function to install dependencies on Arch Linux
install_arch() {
    echo "Detected Arch Linux system"

    echo "Updating package database..."
    sudo pacman -Sy

    echo "Installing development tools..."
    sudo pacman -S --needed base-devel gcc make

    echo "Installing kernel headers..."
    sudo pacman -S --needed linux-headers

    echo "Installing additional tools..."
    sudo pacman -S --needed \
        pkgconf \
        git \
        tar \
        gzip

    echo "Dependencies installed successfully for Arch Linux"
}

# Function to install dependencies on Alpine Linux
install_alpine() {
    echo "Detected Alpine Linux system"

    echo "Updating package index..."
    sudo apk update

    echo "Installing development tools..."
    sudo apk add \
        build-base \
        gcc \
        g++ \
        make \
        musl-dev

    echo "Installing kernel headers..."
    sudo apk add linux-headers

    echo "Installing additional tools..."
    sudo apk add \
        pkgconfig \
        git \
        tar \
        gzip

    echo "Dependencies installed successfully for Alpine Linux"
}

# Function to install dependencies on macOS
install_macos() {
    echo "Detected macOS system"

    # Check if Xcode command line tools are installed
    if ! command -v gcc >/dev/null 2>&1; then
        echo "Installing Xcode command line tools..."
        xcode-select --install
        echo "Please complete the Xcode installation dialog and re-run this script"
        exit 1
    fi

    # Check if Homebrew is installed
    if ! command -v brew >/dev/null 2>&1; then
        echo "Installing Homebrew..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    fi

    echo "Installing development tools via Homebrew..."
    brew install \
        gcc \
        make \
        pkg-config \
        git

    echo "Note: Kernel module development is not supported on macOS"
    echo "Only userspace library and examples will be available"
    echo "Dependencies installed successfully for macOS"
}

# Function to install dependencies on Windows
install_windows() {
    echo "Detected Windows system (MSYS2/MinGW)"

    # Check if we're in MSYS2 environment
    if [[ -z "$MSYSTEM" ]]; then
        echo "❌ Error: This script requires MSYS2 environment"
        echo ""
        echo "Please install MSYS2 first:"
        echo "1. Download MSYS2 from https://www.msys2.org/"
        echo "2. Install MSYS2"
        echo "3. Open MSYS2 terminal"
        echo "4. Run this script again"
        echo ""
        echo "Alternative: Use Visual Studio with vcpkg (see README.md)"
        exit 1
    fi

    echo "MSYS2 environment detected: $MSYSTEM"

    # Determine the correct package prefix based on MSYSTEM
    case "$MSYSTEM" in
        MINGW64)
            PKG_PREFIX="mingw-w64-x86_64"
            ;;
        MINGW32)
            PKG_PREFIX="mingw-w64-i686"
            ;;
        UCRT64)
            PKG_PREFIX="mingw-w64-ucrt-x86_64"
            ;;
        CLANG64)
            PKG_PREFIX="mingw-w64-clang-x86_64"
            ;;
        *)
            echo "❌ Unsupported MSYSTEM: $MSYSTEM"
            echo "Please use MINGW64, MINGW32, UCRT64, or CLANG64"
            exit 1
            ;;
    esac

    echo "Updating MSYS2 package database..."
    pacman -Sy

    echo "Installing build tools..."
    pacman -S --needed \
        $PKG_PREFIX-gcc \
        $PKG_PREFIX-make \
        $PKG_PREFIX-pkg-config \
        make \
        git

    echo "Installing additional development tools..."
    pacman -S --needed \
        $PKG_PREFIX-cmake \
        $PKG_PREFIX-ninja \
        tar \
        gzip

    echo "Dependencies installed successfully for Windows (MSYS2)"
    echo ""
    echo "Build commands for Windows:"
    echo "  make -f lib/Makefile.win all          # Build library"
    echo "  make -f examples/Makefile.win all     # Build examples"
    echo ""
    echo "Note: Windows version uses simulation backend (no kernel module)"
}

# Function to check if dependencies are already installed
check_dependencies() {
    echo "Checking existing dependencies..."

    local missing=0

    # Check essential build tools
    for tool in gcc g++ make; do
        if ! command -v $tool >/dev/null 2>&1; then
            echo "  ❌ Missing: $tool"
            missing=1
        else
            echo "  ✅ Found: $tool ($(command -v $tool))"
        fi
    done

    # Check kernel headers (Linux only)
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        KERNEL_VERSION=$(uname -r)
        HEADER_PATHS=(
            "/lib/modules/$KERNEL_VERSION/build"
            "/usr/src/linux-headers-$KERNEL_VERSION"
            "/usr/src/kernels/$KERNEL_VERSION"
        )

        local headers_found=0
        for path in "${HEADER_PATHS[@]}"; do
            if [[ -d "$path" ]]; then
                echo "  ✅ Found kernel headers: $path"
                headers_found=1
                break
            fi
        done

        if [[ $headers_found -eq 0 ]]; then
            echo "  ❌ Missing: kernel headers for $KERNEL_VERSION"
            missing=1
        fi
    elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "win32" ]]; then
        echo "  ℹ️  Windows: Kernel module not applicable (userspace only)"
    fi

    if [[ $missing -eq 0 ]]; then
        echo "All dependencies are already installed!"
        return 0
    else
        echo "Some dependencies are missing"
        return 1
    fi
}

# Main installation logic
main() {
    echo "Project directory: $PROJECT_DIR"
    echo "Kernel version: $(uname -r)"
    echo "Architecture: $(uname -m)"
    echo ""

    # Check if dependencies are already installed
    if check_dependencies; then
        echo ""
        echo "✅ System is ready for building!"
        echo ""
        echo "Next steps:"
        echo "  cd $PROJECT_DIR"
        echo "  make all          # Build userspace components"
        echo "  make kernel       # Build kernel module"
        exit 0
    fi

    echo ""
    echo "Installing missing dependencies..."
    echo ""

    # Detect OS and install accordingly
    OS=$(detect_os)
    case "$OS" in
        ubuntu)
            install_ubuntu
            ;;
        rhel)
            install_rhel
            ;;
        fedora)
            install_fedora
            ;;
        arch)
            install_arch
            ;;
        alpine)
            install_alpine
            ;;
        macos)
            install_macos
            ;;
        windows)
            install_windows
            ;;
        *)
            echo "❌ Unsupported operating system: $OSTYPE"
            echo ""
            echo "Please install the following manually:"
            echo "  - GCC compiler"
            echo "  - G++ compiler"
            echo "  - Make build tool"
            echo "  - Kernel development headers (Linux only)"
            echo "  - pkg-config"
            echo "  - Git"
            echo ""
            echo "For Windows: Install MSYS2 from https://www.msys2.org/"
            exit 1
            ;;
    esac

    echo ""
    echo "✅ Dependencies installed successfully!"
    echo ""
    echo "Next steps:"
    echo "  cd $PROJECT_DIR"
    echo "  make all          # Build userspace components"
    echo "  make kernel       # Build kernel module (Linux only)"
    echo "  make run-examples # Run example programs (requires sudo)"
}

# Handle command line arguments
case "${1:-}" in
    --check|-c)
        check_dependencies
        exit $?
        ;;
    --help|-h)
        echo "Usage: $0 [OPTIONS]"
        echo ""
        echo "Install build dependencies for PCIe Simulator"
        echo ""
        echo "Options:"
        echo "  --check, -c    Check if dependencies are installed"
        echo "  --help, -h     Show this help message"
        echo ""
        echo "Supported platforms:"
        echo "  - Ubuntu/Debian (apt)"
        echo "  - RHEL/CentOS (yum)"
        echo "  - Fedora (dnf)"
        echo "  - Arch Linux (pacman)"
        echo "  - Alpine Linux (apk)"
        echo "  - macOS (homebrew)"
        echo "  - Windows (MSYS2)"
        exit 0
        ;;
    "")
        main
        ;;
    *)
        echo "Unknown option: $1"
        echo "Use --help for usage information"
        exit 1
        ;;
esac