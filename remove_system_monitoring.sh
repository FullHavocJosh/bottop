#!/bin/bash
# Script to remove all stock btop monitoring code from bottop
# This will keep only AzerothCore monitoring functionality

set -e

BOTTOP_DIR="/home/havoc/bottop"
cd "$BOTTOP_DIR"

echo "================================================"
echo "Removing Stock btop Monitoring Code"
echo "================================================"
echo ""

# Backup current state
echo "[1/5] Creating backup..."
BACKUP_DIR="${BOTTOP_DIR}_backup_$(date +%Y%m%d_%H%M%S)"
cp -r "$BOTTOP_DIR" "$BACKUP_DIR"
echo "  ✓ Backup created at: $BACKUP_DIR"
echo ""

# Remove the linux-specific system monitoring collection
echo "[2/5] Removing system monitoring collection..."
if [ -f "src/linux/btop_collect.cpp" ]; then
    rm -f "src/linux/btop_collect.cpp"
    echo "  ✓ Removed src/linux/btop_collect.cpp"
fi
if [ -d "src/linux" ] && [ -z "$(ls -A src/linux)" ]; then
    rmdir "src/linux"
    echo "  ✓ Removed empty src/linux directory"
fi
echo ""

echo "[3/5] Current source files to review:"
ls -1 src/*.cpp src/*.hpp 2>/dev/null | grep -v azerothcore | sed 's/^/  - /'
echo ""

echo "[4/5] Files to keep (core functionality):"
echo "  - main.cpp (entry point)"
echo "  - btop.cpp (main loop - will be simplified)"
echo "  - btop.hpp (main header)"
echo "  - btop_cli.cpp/hpp (command line args)"
echo "  - btop_config.cpp/hpp (configuration)"
echo "  - btop_draw.cpp/hpp (rendering - will be simplified)" 
echo "  - btop_input.cpp/hpp (input handling - will be simplified)"
echo "  - btop_menu.cpp/hpp (menu system)"
echo "  - btop_shared.cpp/hpp (shared utilities)"
echo "  - btop_theme.cpp/hpp (theming)"
echo "  - btop_tools.cpp/hpp (utility functions)"
echo "  - btop_azerothcore.cpp/hpp (AzerothCore monitoring)"
echo "  - btop_collect_stub.cpp (empty stub for removed collection)"
echo "  - config.h.in (CMake config)"
echo ""

echo "[5/5] Next steps:"
echo "  1. Edit btop_draw.cpp to remove Cpu, Mem, Net, Proc, Gpu namespaces"
echo "  2. Edit btop_input.cpp to remove all stock box input handlers"
echo "  3. Edit btop.cpp to remove stock box initialization/collection"
echo "  4. Update CMakeLists.txt to remove linux/btop_collect.cpp reference"
echo "  5. Rebuild and test"
echo ""
echo "Run ./remove_system_monitoring.sh to execute these changes"
echo ""
