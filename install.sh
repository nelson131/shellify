#!/usr/bin/env bash

set -e

GREEN='\033[0;32m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0;0m'

echo -e "${CYAN}installing the shellify${NC}"

if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}error: failed to find the cmakelist.txt${NC}"
    exit 1
fi

if ! command -v cmake &> /dev/null; then
    echo -e "${RED}error: failed to find cmake${NC}"
    exit 1
fi

echo -e "\n${YELLOW}[1/3] starting to build${NC}"
mkdir -p build
cd build

cmake -DCMAKE_BUILD_TYPE=Release ..

echo -e "\n${YELLOW}[2/3] compiling${NC}"
cmake --build . --parallel "$(nproc 2>/dev/null || echo 2)"

echo -e "${GREEN}complied successfully${NC}"

echo -e "\n${YELLOW}[3/3] installing as a package${NC}"

sudo cmake --install .

sudo update-desktop-database /usr/local/share/applications 2>/dev/null || true

echo -e "\n${YELLOW}Testing the installation${NC}"
if command -v shellify &> /dev/null; then
    echo -e "${GREEN}success${NC}"
else
    echo -e "${RED}error${NC}"
fi

echo -e "\n${GREEN}shellify has been installed successfully.${NC}\n"
