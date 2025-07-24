#!/bin/bash
# Install script for Sig language compiler

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}📦 Installing Sig Language Compiler...${NC}"

# Check if sig executable exists
if [ ! -f "./sig" ]; then
    echo -e "${RED}❌ Error: sig executable not found. Please build first:${NC}"
    echo "   ./build.sh"
    exit 1
fi

# Determine install directory
if [ -d "$HOME/.local/bin" ]; then
    INSTALL_DIR="$HOME/.local/bin"
elif [ -d "$HOME/bin" ]; then
    INSTALL_DIR="$HOME/bin"
else
    echo -e "${YELLOW}📁 Creating user bin directory...${NC}"
    mkdir -p "$HOME/.local/bin"
    INSTALL_DIR="$HOME/.local/bin"
fi

echo -e "${BLUE}📍 Installing to: ${INSTALL_DIR}${NC}"

# Copy the executable
cp ./sig "$INSTALL_DIR/sig"
chmod +x "$INSTALL_DIR/sig"

echo -e "${GREEN}✅ Sig compiler installed successfully!${NC}"

# Check if install directory is in PATH
if [[ ":$PATH:" == *":$INSTALL_DIR:"* ]]; then
    echo -e "${GREEN}✅ $INSTALL_DIR is already in your PATH${NC}"
else
    echo -e "${YELLOW}⚠️  $INSTALL_DIR is not in your PATH${NC}"
    echo -e "${BLUE}📝 To use 'sig' from anywhere, add this to your shell profile:${NC}"
    
    # Detect shell and provide appropriate instructions
    if [ -n "$ZSH_VERSION" ]; then
        SHELL_CONFIG="~/.zshrc"
    elif [ -n "$BASH_VERSION" ]; then
        SHELL_CONFIG="~/.bashrc"
    else
        SHELL_CONFIG="~/.profile"
    fi
    
    echo ""
    echo -e "${BLUE}   echo 'export PATH=\"$INSTALL_DIR:\$PATH\"' >> $SHELL_CONFIG${NC}"
    echo -e "${BLUE}   source $SHELL_CONFIG${NC}"
    echo ""
    echo -e "${BLUE}Or restart your terminal.${NC}"
fi

echo ""
echo -e "${GREEN}🚀 Try it out:${NC}"
if [[ ":$PATH:" == *":$INSTALL_DIR:"* ]]; then
    echo -e "${GREEN}   sig examples/hello_world.sg${NC}"
    echo -e "${GREEN}   sig examples/variables.sg --ir${NC}"
    echo -e "${GREEN}   sig --help${NC}"
else
    echo -e "${GREEN}   $INSTALL_DIR/sig examples/hello_world.sg${NC}"
    echo -e "${GREEN}   $INSTALL_DIR/sig examples/variables.sg --ir${NC}"
fi

echo ""
echo -e "${BLUE}📚 Documentation: https://github.com/GhostedGaming/sig-language${NC}"
