#!/bin/bash
# Script to install the newly built bottop binary

set -e

# Check if build/bottop exists
if [ ! -f "build/bottop" ]; then
    echo "ERROR: build/bottop not found. Please run 'cmake --build build' first"
    exit 1
fi

# Create bin directory if it doesn't exist
mkdir -p bin

# Backup old binary if it exists
if [ -f "bin/bottop" ]; then
    echo "Backing up old binary to bin/bottop.old"
    mv bin/bottop bin/bottop.old
fi

# Remove old btop binary if it exists (from before the rename)
if [ -f "bin/btop" ]; then
    echo "Removing old btop binary (now renamed to bottop)"
    rm bin/btop
fi

# Copy new binary
echo "Installing new binary to bin/bottop"
cat build/bottop > bin/bottop
chmod +x bin/bottop

echo "Done! New binary installed at bin/bottop"
ls -lh bin/bottop

echo ""
echo "To run: ./bin/bottop"
echo "Config will be stored in: ~/.config/bottop/"
