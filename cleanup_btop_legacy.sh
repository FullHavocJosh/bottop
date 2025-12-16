#!/bin/bash
# Cleanup script to remove unused btop code from bottop
# This makes bottop a Linux-only, AzerothCore-focused monitoring tool

set -e

echo "Cleaning up legacy btop code..."

# Remove platform-specific collectors
echo "Removing non-Linux platform support..."
rm -rf src/freebsd
rm -rf src/netbsd  
rm -rf src/openbsd
rm -rf src/osx

# Remove GPU monitoring
echo "Removing GPU monitoring support..."
rm -rf src/linux/intel_gpu_top

# Remove platform-specific CMake modules
echo "Removing platform-specific CMake modules..."
rm -f cmake/Finddevstat.cmake
rm -f cmake/Findelf.cmake
rm -f cmake/Findkvm.cmake
rm -f cmake/Findproplib.cmake

# Remove non-Linux GitHub workflows
echo "Removing non-Linux CI workflows..."
rm -f .github/workflows/cmake-freebsd.yml
rm -f .github/workflows/cmake-macos.yml
rm -f .github/workflows/cmake-netbsd.yml
rm -f .github/workflows/cmake-openbsd.yml
rm -f .github/workflows/continuous-build-freebsd.yml
rm -f .github/workflows/continuous-build-macos.yml
rm -f .github/workflows/continuous-build-netbsd.yml
rm -f .github/workflows/continuous-build-openbsd.yml
rm -f .github/workflows/continuous-build-gpu.yml
rm -f .github/workflows/test-snap-can-build.yml

# Remove snap packaging
echo "Removing snap packaging..."
rm -rf snap

# Remove old btop renaming scripts
echo "Removing old renaming scripts..."
rm -f rename_wowmon_to_azerothcore.sh

# Remove excessive themes (keep only 5 popular ones)
echo "Cleaning up themes directory..."
cd themes
# Keep these themes
KEEP_THEMES="nord.theme dracula.theme gruvbox_dark_v2.theme tokyo-night.theme onedark.theme"
for theme in *.theme; do
    if [[ ! " $KEEP_THEMES " =~ " $theme " ]]; then
        rm -f "$theme"
    fi
done
cd ..

echo ""
echo "Cleanup complete!"
echo ""
echo "Removed:"
echo "  - FreeBSD, NetBSD, OpenBSD, macOS platform support"
echo "  - GPU monitoring (Intel, AMD, NVIDIA)"
echo "  - Non-Linux CI workflows"
echo "  - Snap packaging"
echo "  - Excess themes (kept 5 popular ones)"
echo ""
echo "Next steps:"
echo "  1. Update CMakeLists.txt to remove platform checks"
echo "  2. Update README.md"
echo "  3. Test build with: cd build && cmake .. && make"
