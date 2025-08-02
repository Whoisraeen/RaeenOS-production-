#!/bin/bash

# RaeCompat Setup Script
# Installs and configures the Windows compatibility layer for RaeenOS

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
RAECOMPAT_DIR="/usr/lib/raecompat"
WINE_VERSION="8.21"
DXVK_VERSION="2.3.1"
VKD3D_VERSION="2.10"
PROTON_GE_VERSION="GE-Proton8-26"

# Directories
USER_HOME="${HOME:-/home/$USER}"
RAECOMPAT_USER_DIR="$USER_HOME/.raecompat"
PREFIX_DIR="$RAECOMPAT_USER_DIR/prefixes"
DOWNLOADS_DIR="$RAECOMPAT_USER_DIR/downloads"
LOGS_DIR="$RAECOMPAT_USER_DIR/logs"

echo -e "${BLUE}=== RaeCompat Windows Compatibility Layer Setup ===${NC}"
echo "Setting up Wine, DXVK, VKD3D, and related components..."

# Check system architecture
check_architecture() {
    echo -e "${YELLOW}Checking system architecture...${NC}"
    ARCH=$(uname -m)
    if [ "$ARCH" != "x86_64" ]; then
        echo -e "${RED}Error: RaeCompat requires x86_64 architecture. Found: $ARCH${NC}"
        exit 1
    fi
    echo -e "${GREEN}✓ Architecture check passed: $ARCH${NC}"
}

# Check for required dependencies
check_dependencies() {
    echo -e "${YELLOW}Checking dependencies...${NC}"
    
    REQUIRED_PACKAGES=(
        "build-essential"
        "gcc-multilib"
        "g++-multilib"
        "libc6-dev"
        "libc6-dev-i386"
        "libvulkan1"
        "libvulkan-dev"
        "vulkan-tools"
        "mesa-vulkan-drivers"
        "libgl1-mesa-dev"
        "libglu1-mesa-dev"
        "libasound2-dev"
        "libpulse-dev"
        "libx11-dev"
        "libxext-dev"
        "libxrandr-dev"
        "libxi-dev"
        "libxxf86vm-dev"
        "libxinerama-dev"
        "libxcursor-dev"
        "libfreetype6-dev"
        "libfontconfig1-dev"
        "libjson-c-dev"
        "curl"
        "wget"
        "git"
        "unzip"
        "cabextract"
        "p7zip-full"
    )
    
    MISSING_PACKAGES=()
    
    for package in "${REQUIRED_PACKAGES[@]}"; do
        if ! dpkg -l | grep -q "^ii  $package "; then
            MISSING_PACKAGES+=("$package")
        fi
    done
    
    if [ ${#MISSING_PACKAGES[@]} -ne 0 ]; then
        echo -e "${YELLOW}Installing missing packages: ${MISSING_PACKAGES[*]}${NC}"
        sudo apt update
        sudo apt install -y "${MISSING_PACKAGES[@]}"
    fi
    
    echo -e "${GREEN}✓ Dependencies check completed${NC}"
}

# Check Vulkan support
check_vulkan() {
    echo -e "${YELLOW}Checking Vulkan support...${NC}"
    
    if ! command -v vulkaninfo &> /dev/null; then
        echo -e "${RED}Error: vulkaninfo not found. Install vulkan-tools.${NC}"
        exit 1
    fi
    
    if ! vulkaninfo --summary | grep -q "Vulkan Instance"; then
        echo -e "${RED}Error: No Vulkan support detected. Check your GPU drivers.${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}✓ Vulkan support detected${NC}"
}

# Create directory structure
setup_directories() {
    echo -e "${YELLOW}Setting up directory structure...${NC}"
    
    # System directories
    sudo mkdir -p "$RAECOMPAT_DIR"
    sudo mkdir -p "$RAECOMPAT_DIR/wine"
    sudo mkdir -p "$RAECOMPAT_DIR/dxvk"
    sudo mkdir -p "$RAECOMPAT_DIR/vkd3d"
    sudo mkdir -p "$RAECOMPAT_DIR/proton"
    sudo mkdir -p "$RAECOMPAT_DIR/winetricks"
    
    # User directories
    mkdir -p "$RAECOMPAT_USER_DIR"
    mkdir -p "$PREFIX_DIR"
    mkdir -p "$DOWNLOADS_DIR"
    mkdir -p "$LOGS_DIR"
    
    echo -e "${GREEN}✓ Directory structure created${NC}"
}

# Download and install Wine
install_wine() {
    echo -e "${YELLOW}Installing Wine $WINE_VERSION...${NC}"
    
    WINE_DIR="$RAECOMPAT_DIR/wine"
    
    if [ -f "$WINE_DIR/bin/wine" ]; then
        echo -e "${GREEN}✓ Wine already installed${NC}"
        return
    fi
    
    # Download Wine staging
    WINE_URL="https://github.com/wine-mirror/wine/archive/refs/tags/wine-$WINE_VERSION.tar.gz"
    WINE_ARCHIVE="$DOWNLOADS_DIR/wine-$WINE_VERSION.tar.gz"
    
    echo "Downloading Wine source..."
    wget -O "$WINE_ARCHIVE" "$WINE_URL"
    
    # Extract and build
    cd "$DOWNLOADS_DIR"
    tar -xzf "wine-$WINE_VERSION.tar.gz"
    cd "wine-wine-$WINE_VERSION"
    
    echo "Configuring Wine build..."
    ./configure --prefix="$WINE_DIR" --enable-win64 --with-vulkan
    
    echo "Building Wine (this may take a while)..."
    make -j$(nproc)
    
    echo "Installing Wine..."
    sudo make install
    
    # Clean up
    cd "$DOWNLOADS_DIR"
    rm -rf "wine-wine-$WINE_VERSION"
    rm -f "$WINE_ARCHIVE"
    
    echo -e "${GREEN}✓ Wine $WINE_VERSION installed${NC}"
}

# Download and install DXVK
install_dxvk() {
    echo -e "${YELLOW}Installing DXVK $DXVK_VERSION...${NC}"
    
    DXVK_DIR="$RAECOMPAT_DIR/dxvk"
    
    if [ -f "$DXVK_DIR/x64/d3d11.dll" ]; then
        echo -e "${GREEN}✓ DXVK already installed${NC}"
        return
    fi
    
    # Download DXVK
    DXVK_URL="https://github.com/doitsujin/dxvk/releases/download/v$DXVK_VERSION/dxvk-$DXVK_VERSION.tar.gz"
    DXVK_ARCHIVE="$DOWNLOADS_DIR/dxvk-$DXVK_VERSION.tar.gz"
    
    echo "Downloading DXVK..."
    wget -O "$DXVK_ARCHIVE" "$DXVK_URL"
    
    # Extract
    cd "$DOWNLOADS_DIR"
    tar -xzf "dxvk-$DXVK_VERSION.tar.gz"
    
    # Install
    sudo cp -r "dxvk-$DXVK_VERSION"/* "$DXVK_DIR/"
    
    # Clean up
    rm -rf "dxvk-$DXVK_VERSION"
    rm -f "$DXVK_ARCHIVE"
    
    echo -e "${GREEN}✓ DXVK $DXVK_VERSION installed${NC}"
}

# Download and install VKD3D-Proton
install_vkd3d() {
    echo -e "${YELLOW}Installing VKD3D-Proton $VKD3D_VERSION...${NC}"
    
    VKD3D_DIR="$RAECOMPAT_DIR/vkd3d"
    
    if [ -f "$VKD3D_DIR/x64/d3d12.dll" ]; then
        echo -e "${GREEN}✓ VKD3D-Proton already installed${NC}"
        return
    fi
    
    # Download VKD3D-Proton
    VKD3D_URL="https://github.com/HansKristian-Work/vkd3d-proton/releases/download/v$VKD3D_VERSION/vkd3d-proton-$VKD3D_VERSION.tar.xz"
    VKD3D_ARCHIVE="$DOWNLOADS_DIR/vkd3d-proton-$VKD3D_VERSION.tar.xz"
    
    echo "Downloading VKD3D-Proton..."
    wget -O "$VKD3D_ARCHIVE" "$VKD3D_URL"
    
    # Extract
    cd "$DOWNLOADS_DIR"
    tar -xf "vkd3d-proton-$VKD3D_VERSION.tar.xz"
    
    # Install
    sudo cp -r "vkd3d-proton-$VKD3D_VERSION"/* "$VKD3D_DIR/"
    
    # Clean up
    rm -rf "vkd3d-proton-$VKD3D_VERSION"
    rm -f "$VKD3D_ARCHIVE"
    
    echo -e "${GREEN}✓ VKD3D-Proton $VKD3D_VERSION installed${NC}"
}

# Install Proton-GE
install_proton_ge() {
    echo -e "${YELLOW}Installing Proton-GE $PROTON_GE_VERSION...${NC}"
    
    PROTON_DIR="$RAECOMPAT_DIR/proton"
    
    if [ -d "$PROTON_DIR/$PROTON_GE_VERSION" ]; then
        echo -e "${GREEN}✓ Proton-GE already installed${NC}"
        return
    fi
    
    # Download Proton-GE
    PROTON_URL="https://github.com/GloriousEggroll/proton-ge-custom/releases/download/$PROTON_GE_VERSION/$PROTON_GE_VERSION.tar.gz"
    PROTON_ARCHIVE="$DOWNLOADS_DIR/$PROTON_GE_VERSION.tar.gz"
    
    echo "Downloading Proton-GE..."
    wget -O "$PROTON_ARCHIVE" "$PROTON_URL"
    
    # Extract
    cd "$DOWNLOADS_DIR"
    tar -xzf "$PROTON_GE_VERSION.tar.gz"
    
    # Install
    sudo mv "$PROTON_GE_VERSION" "$PROTON_DIR/"
    
    # Clean up
    rm -f "$PROTON_ARCHIVE"
    
    echo -e "${GREEN}✓ Proton-GE $PROTON_GE_VERSION installed${NC}"
}

# Install Winetricks
install_winetricks() {
    echo -e "${YELLOW}Installing Winetricks...${NC}"
    
    WINETRICKS_DIR="$RAECOMPAT_DIR/winetricks"
    
    if [ -f "$WINETRICKS_DIR/winetricks" ]; then
        echo -e "${GREEN}✓ Winetricks already installed${NC}"
        return
    fi
    
    # Download latest winetricks
    echo "Downloading Winetricks..."
    sudo wget -O "$WINETRICKS_DIR/winetricks" "https://raw.githubusercontent.com/Winetricks/winetricks/master/src/winetricks"
    sudo chmod +x "$WINETRICKS_DIR/winetricks"
    
    echo -e "${GREEN}✓ Winetricks installed${NC}"
}

# Install MangoHUD (optional performance overlay)
install_mangohud() {
    echo -e "${YELLOW}Installing MangoHUD...${NC}"
    
    if command -v mangohud &> /dev/null; then
        echo -e "${GREEN}✓ MangoHUD already installed${NC}"
        return
    fi
    
    # Install MangoHUD from package manager or build from source
    if apt list --installed 2>/dev/null | grep -q mangohud; then
        echo -e "${GREEN}✓ MangoHUD available from package manager${NC}"
        sudo apt install -y mangohud
    else
        echo "Building MangoHUD from source..."
        cd "$DOWNLOADS_DIR"
        git clone --recurse-submodules https://github.com/flightlessmango/MangoHud.git
        cd MangoHud
        ./build.sh build
        ./build.sh install
        cd "$DOWNLOADS_DIR"
        rm -rf MangoHud
    fi
    
    echo -e "${GREEN}✓ MangoHUD installed${NC}"
}

# Create system integration
setup_system_integration() {
    echo -e "${YELLOW}Setting up system integration...${NC}"
    
    # Create desktop entry for RaeenGameManager
    cat > "$USER_HOME/.local/share/applications/raeen-game-manager.desktop" << EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=RaeenOS Game Manager
Comment=Launch and manage Windows games on RaeenOS
Exec=/usr/bin/raeen-game-manager
Icon=/usr/share/raeenos/icons/game-manager.png
Terminal=false
Categories=Game;
EOF
    
    # Create system service for RaeCompat daemon (optional)
    sudo tee /etc/systemd/system/raecompat.service > /dev/null << EOF
[Unit]
Description=RaeCompat Windows Compatibility Service
After=graphical-session.target

[Service]
Type=notify
ExecStart=/usr/bin/raecompat-daemon
Restart=on-failure
User=root

[Install]
WantedBy=multi-user.target
EOF
    
    # Create environment setup script
    cat > "$RAECOMPAT_USER_DIR/raecompat-env.sh" << EOF
#!/bin/bash
# RaeCompat Environment Setup

export RAECOMPAT_DIR="$RAECOMPAT_DIR"
export WINEPREFIX="\$HOME/.raecompat/prefixes/default"
export WINEARCH=win64
export PATH="$RAECOMPAT_DIR/wine/bin:\$PATH"
export LD_LIBRARY_PATH="$RAECOMPAT_DIR/wine/lib:\$LD_LIBRARY_PATH"

# DXVK/VKD3D paths
export DXVK_PATH="$RAECOMPAT_DIR/dxvk"
export VKD3D_PATH="$RAECOMPAT_DIR/vkd3d"

# Performance optimizations
export WINE_CPU_TOPOLOGY=4:2
export DXVK_HUD=fps,memory,gpuload
export __GL_SHADER_DISK_CACHE=1
export __GL_SHADER_DISK_CACHE_PATH="\$HOME/.cache/mesa_shader_cache"

# Enable esync/fsync if available
export WINEESYNC=1
export WINEFSYNC=1

echo "RaeCompat environment loaded"
EOF
    
    chmod +x "$RAECOMPAT_USER_DIR/raecompat-env.sh"
    
    echo -e "${GREEN}✓ System integration completed${NC}"
}

# Create default Wine prefix
create_default_prefix() {
    echo -e "${YELLOW}Creating default Wine prefix...${NC}"
    
    DEFAULT_PREFIX="$PREFIX_DIR/default"
    
    if [ -d "$DEFAULT_PREFIX" ]; then
        echo -e "${GREEN}✓ Default prefix already exists${NC}"
        return
    fi
    
    # Source environment
    source "$RAECOMPAT_USER_DIR/raecompat-env.sh"
    
    # Create prefix
    export WINEPREFIX="$DEFAULT_PREFIX"
    "$RAECOMPAT_DIR/wine/bin/wineboot" --init
    
    # Install basic dependencies
    "$RAECOMPAT_DIR/winetricks/winetricks" -q corefonts vcrun2019 dotnet48
    
    echo -e "${GREEN}✓ Default Wine prefix created${NC}"
}

# Verify installation
verify_installation() {
    echo -e "${YELLOW}Verifying installation...${NC}"
    
    # Check Wine
    if [ -f "$RAECOMPAT_DIR/wine/bin/wine" ]; then
        WINE_VERSION_OUTPUT=$("$RAECOMPAT_DIR/wine/bin/wine" --version)
        echo -e "${GREEN}✓ Wine: $WINE_VERSION_OUTPUT${NC}"
    else
        echo -e "${RED}✗ Wine installation failed${NC}"
        return 1
    fi
    
    # Check DXVK
    if [ -f "$RAECOMPAT_DIR/dxvk/x64/d3d11.dll" ]; then
        echo -e "${GREEN}✓ DXVK: Installed${NC}"
    else
        echo -e "${RED}✗ DXVK installation failed${NC}"
        return 1
    fi
    
    # Check VKD3D
    if [ -f "$RAECOMPAT_DIR/vkd3d/x64/d3d12.dll" ]; then
        echo -e "${GREEN}✓ VKD3D-Proton: Installed${NC}"
    else
        echo -e "${RED}✗ VKD3D-Proton installation failed${NC}"
        return 1
    fi
    
    # Check Proton-GE
    if [ -d "$RAECOMPAT_DIR/proton/$PROTON_GE_VERSION" ]; then
        echo -e "${GREEN}✓ Proton-GE: $PROTON_GE_VERSION${NC}"
    else
        echo -e "${RED}✗ Proton-GE installation failed${NC}"
        return 1
    fi
    
    # Check Winetricks
    if [ -f "$RAECOMPAT_DIR/winetricks/winetricks" ]; then
        echo -e "${GREEN}✓ Winetricks: Installed${NC}"
    else
        echo -e "${RED}✗ Winetricks installation failed${NC}"
        return 1
    fi
    
    echo -e "${GREEN}✓ Installation verification completed${NC}"
}

# Main installation process
main() {
    echo -e "${BLUE}Starting RaeCompat installation...${NC}"
    
    check_architecture
    check_dependencies
    check_vulkan
    setup_directories
    install_wine
    install_dxvk
    install_vkd3d
    install_proton_ge
    install_winetricks
    install_mangohud
    setup_system_integration
    create_default_prefix
    verify_installation
    
    echo -e "${GREEN}"
    echo "=================================================="
    echo "RaeCompat installation completed successfully!"
    echo "=================================================="
    echo -e "${NC}"
    echo
    echo "Next steps:"
    echo "1. Source the environment: source ~/.raecompat/raecompat-env.sh"
    echo "2. Launch RaeenGameManager: raeen-game-manager"
    echo "3. Add your game directories in the settings"
    echo "4. Enjoy gaming on RaeenOS!"
    echo
    echo "For troubleshooting, check logs in: $LOGS_DIR"
}

# Run main function
main "$@"
