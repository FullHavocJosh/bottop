# bottop Cleanup Summary

This document describes the cleanup performed to transform btop++ into bottop - a specialized Linux-only AzerothCore bot monitor.

## What Was Removed

### Platform-Specific Code

- ❌ **src/freebsd/** - FreeBSD system monitoring
- ❌ **src/netbsd/** - NetBSD system monitoring
- ❌ **src/openbsd/** - OpenBSD system monitoring
- ❌ **src/osx/** - macOS system monitoring and SMC sensors

### GPU Monitoring

- ❌ **src/linux/intel_gpu_top/** - Intel integrated GPU monitoring
- ❌ ROCm SMI library support for AMD GPUs
- ❌ NVIDIA GPU support via nvidia-ml
- ❌ All GPU-related CMake configuration

### Build System Files

- ❌ **cmake/Finddevstat.cmake** - FreeBSD devstat library
- ❌ **cmake/Findelf.cmake** - ELF library for FreeBSD
- ❌ **cmake/Findkvm.cmake** - KVM library for BSD systems
- ❌ **cmake/Findproplib.cmake** - NetBSD proplib library

### CI/CD Workflows

- ❌ **.github/workflows/cmake-freebsd.yml**
- ❌ **.github/workflows/cmake-macos.yml**
- ❌ **.github/workflows/cmake-netbsd.yml**
- ❌ **.github/workflows/cmake-openbsd.yml**
- ❌ **.github/workflows/continuous-build-freebsd.yml**
- ❌ **.github/workflows/continuous-build-macos.yml**
- ❌ **.github/workflows/continuous-build-netbsd.yml**
- ❌ **.github/workflows/continuous-build-openbsd.yml**
- ❌ **.github/workflows/continuous-build-gpu.yml**
- ❌ **.github/workflows/test-snap-can-build.yml**

### Packaging

- ❌ **snap/** directory - Snap package configuration
- ❌ Snap-related build configurations

### Themes (Reduced)

Removed 30+ theme files, keeping only 5 essential themes:

- ✅ nord.theme
- ✅ dracula.theme
- ✅ gruvbox_dark_v2.theme
- ✅ tokyo-night.theme
- ✅ onedark.theme

### Legacy Scripts

- ❌ **rename_wowmon_to_azerothcore.sh** - Old renaming script
- ❌ Old btop-specific documentation files

## What Was Kept

### Core System Monitoring (from btop++)

- ✅ **src/btop.cpp** - Main application logic
- ✅ **src/btop_cli.cpp** - Command-line interface
- ✅ **src/btop_config.cpp** - Configuration system
- ✅ **src/btop_draw.cpp** - TUI rendering engine
- ✅ **src/btop_input.cpp** - Input handling
- ✅ **src/btop_menu.cpp** - Menu system
- ✅ **src/btop_shared.cpp** - Shared utilities
- ✅ **src/btop_theme.cpp** - Theme engine
- ✅ **src/btop_tools.cpp** - Helper tools
- ✅ **src/linux/btop_collect.cpp** - Linux system data collection

### Linux System Monitoring

- ✅ CPU monitoring (utilization, frequency, temperature)
- ✅ Memory monitoring (RAM, swap)
- ✅ Network monitoring (bandwidth, connections)
- ✅ Process monitoring (list, filtering, sorting)

### AzerothCore Features (Added)

- ✅ **src/btop_azerothcore.cpp** - AzerothCore bot monitoring
- ✅ **src/btop_azerothcore.hpp** - AzerothCore headers
- ✅ SSH-based remote server monitoring
- ✅ Database query support for bot statistics
- ✅ Bot distribution tracking
- ✅ Zone health analysis

### Build System

- ✅ **CMakeLists.txt** - Simplified for Linux-only
- ✅ **src/main.cpp** - Application entry point
- ✅ **include/** - Third-party headers (fmt, widechar_width)
- ✅ **tests/** - Unit test framework

### Documentation

- ✅ **README.md** - Completely rewritten for bottop
- ✅ **ATTRIBUTION.md** - Credits to btop++ (NEW)
- ✅ **LICENSE** - Apache 2.0 (maintained)
- ✅ **AZEROTHCORE_INTEGRATION.md** - Integration documentation
- ✅ **BOTTOP_RENAMING.md** - Renaming documentation
- ✅ **CHANGELOG.md** - Change history

### Resources

- ✅ **Img/** - Icons and logos
- ✅ **themes/** - 5 essential themes (reduced from 35+)
- ✅ **btop.desktop** - Desktop entry (updated to bottop)

## Files Created

1. **ATTRIBUTION.md** - Comprehensive attribution to btop++
2. **cleanup_btop_legacy.sh** - Script to execute the cleanup
3. **CLEANUP_SUMMARY.md** - This file
4. Updated **README.md** - bottop-specific documentation
5. Updated **CMakeLists.txt** - Linux-only build configuration
6. Updated **btop.desktop** - bottop desktop integration

## Running the Cleanup

Execute the cleanup script to remove all the files listed above:

```bash
chmod +x cleanup_btop_legacy.sh
./cleanup_btop_legacy.sh
```

This will:

1. Remove all platform-specific code
2. Remove GPU monitoring support
3. Remove non-Linux CI workflows
4. Remove snap packaging
5. Clean up themes directory
6. Remove legacy scripts

## After Cleanup

### Build the Project

```bash
cd build
cmake ..
make -j$(nproc)
```

### Verify Everything Works

1. Check that bottop builds successfully
2. Run bottop and verify:
    - CPU/Memory/Network monitoring works
    - AzerothCore monitoring works (if configured)
    - Themes load correctly
    - Configuration system works

### Size Reduction

Expected reduction in repository size:

- **Before**: ~15-20 MB (with all platforms + GPU code)
- **After**: ~5-8 MB (Linux-only, essential features)

## What bottop Is Now

**bottop** is now a lean, focused tool for monitoring AzerothCore bot servers on Linux:

- **Single Platform**: Linux only (x86_64)
- **Single Purpose**: AzerothCore bot server monitoring
- **Clean Codebase**: ~60% smaller than original btop++
- **Clear Attribution**: Proper credits to btop++ foundation

## Maintenance Going Forward

### What to Update

- AzerothCore monitoring features
- Linux system monitoring (via btop++ updates)
- bottop-specific documentation
- Linux build configuration

### What NOT to Add Back

- Platform support for BSD/macOS/Windows
- GPU monitoring
- Disk I/O monitoring (removed for simplicity)
- Battery stats (server focus)
- Snap packaging

## Credits

This cleanup maintains full respect for the original btop++ project while creating a specialized tool for a specific use case. All core monitoring capabilities come from btop++, and we are grateful for that excellent foundation.

See **ATTRIBUTION.md** for full credits to the btop++ team.
