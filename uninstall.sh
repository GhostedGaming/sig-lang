#!/bin/bash
# Uninstall script for Sig language compiler

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}Uninstalling Sig Language Compiler...${NC}"

# Check common installation directories
FOUND=false
INSTALL_DIRS=("$HOME/.local/bin" "$HOME/bin" "/usr/local/bin")

for dir in "${INSTALL_DIRS[@]}"; do
    if [ -f "$dir/sig" ]; then
        echo -e "${BLUE}Found sig compiler in: $dir${NC}"
        
        # Check if we have write permission
        if [ -w "$dir/sig" ]; then
            rm "$dir/sig"
            echo -e "${GREEN}Removed $dir/sig${NC}"
            FOUND=true
        else
            echo -e "${RED}No write permission for $dir${NC}"
            echo -e "${YELLOW}   Try: sudo rm $dir/sig${NC}"
        fi
    fi
done

if [ "$FOUND" = false ]; then
    echo -e "${YELLOW}No sig compiler installation found in common directories.${NC}"
    echo -e "${BLUE}   Checked:${NC}"
    for dir in "${INSTALL_DIRS[@]}"; do
        echo "   - $dir"
    done
    echo ""
    echo -e "${BLUE}   If you installed sig elsewhere, remove it manually:${NC}"
    echo -e "${BLUE}   which sig${NC}"
    echo -e "${BLUE}   rm \$(which sig)${NC}"
else
    echo ""
    echo -e "${GREEN}Sig compiler uninstalled successfully!${NC}"
fi

echo ""
echo -e "${BLUE}To reinstall, run: ./build.sh --install${NC}"
