# AzerothCore Monitor Integration for btop

This document describes the integration of AzerothCore bot monitoring functionality from the `azerothcore` project into `btop`.

## Overview

The AzerothCore Monitor (azerothcore) functionality has been integrated into btop as an optional feature. When enabled, it allows monitoring of AzerothCore bots running on an AzerothCore server via SSH connection to a remote database.

## Features Integrated

- **SSH Client**: Connects to remote server using libssh2 with key-based authentication
- **Database Queries**: Executes MySQL queries via SSH to fetch bot statistics
- **Real-time Monitoring**: Displays bot statistics including:
    - Total bot count
    - Server load (CPU usage)
    - Bot distribution by continent (Eastern Kingdoms, Kalimdor, Outland, Northrend)
    - Zone health (top 15 zones with bot concentrations)
    - Level distribution (bots grouped in 10-level brackets)

## New Files Added

1. **src/btop_azerothcore.hpp**: Header file defining AzerothCore namespace, data structures, and function prototypes
2. **src/btop_azerothcore.cpp**: Implementation of SSH client, query handler, and monitoring functions

## Modified Files

1. **CMakeLists.txt**:
    - Added `src/btop_azerothcore.cpp` to libbtop sources
    - Added libssh2 detection and linking
    - Defined `AZEROTHCORE_SUPPORT` when libssh2 is available

2. **src/btop_config.cpp**:
    - Added configuration descriptions for AzerothCore monitoring
    - Added default configuration values:
        - `azerothcore_enabled`: Enable/disable AzerothCore monitoring (default: false)
        - `azerothcore_ssh_host`: SSH connection string (format: user@hostname[:port])
        - `azerothcore_db_host`: Database hostname
        - `azerothcore_db_user`: Database username
        - `azerothcore_db_pass`: Database password
        - `azerothcore_db_name`: Database name
        - `azerothcore_container`: Docker container name

3. **src/btop_draw.cpp**:
    - Added `AzerothCore` namespace with draw functions
    - Implements box rendering for AzerothCore monitor display

4. **src/btop.cpp**:
    - Added `#include "btop_azerothcore.hpp"` when AZEROTHCORE_SUPPORT is defined
    - Initialized AzerothCore monitoring in btop_main after Shared::init()
    - Added AzerothCore data collection and drawing in the runner thread loop

## Configuration

To enable AzerothCore monitoring, edit your btop configuration file (`~/.config/btop/btop.conf`) and add:

```ini
#* Enable AzerothCore bot monitoring
azerothcore_enabled = True

#* SSH connection string (format: user@hostname or user@hostname:port)
azerothcore_ssh_host = "root@testing-azerothcore.rollet.family"

#* Database configuration
azerothcore_db_host = "testing-ac-database"
azerothcore_db_user = "root"
azerothcore_db_pass = "password"
azerothcore_db_name = "acore_characters"

#* Docker container name for AzerothCore server
azerothcore_container = "testing-ac-worldserver"
```

## Build Requirements

- **libssh2**: Required for SSH connectivity

    ```bash
    # Debian/Ubuntu
    sudo apt-get install libssh2-1-dev

    # Fedora/RHEL
    sudo dnf install libssh2-devel

    # macOS
    brew install libssh2
    ```

- **SSH Key Authentication**: The monitoring requires SSH key-based authentication to be configured for the target server

## Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

If libssh2 is not found, btop will build without AzerothCore monitoring support.

## Architecture

### AzerothCore Namespace (::AzerothCore)

Main namespace containing:

- `ServerConfig`: Configuration structure for server connection
- `BotStats`: Bot count and server load statistics
- `Continent`: Bot distribution by continent
- `Zone`: Zone health with bot concentrations
- `LevelBracket`: Level distribution statistics
- `ServerData`: Complete data snapshot
- `SSHClient`: SSH connection handler
- `Query`: Database query executor

### Draw::AzerothCore Namespace

Display namespace for rendering the AzerothCore monitor box in btop's TUI.

## Data Flow

1. **Initialization** (btop_main):
    - Read configuration from btop.conf
    - Create SSHClient with configured host
    - Establish SSH connection
    - Create Query handler

2. **Collection** (Runner thread):
    - Execute database queries via SSH
    - Parse results into data structures
    - Update current_data snapshot

3. **Drawing** (Runner thread):
    - Format data for display
    - Render AzerothCore monitor box
    - Update screen

## Integration Approach

The integration maintains maximum compatibility with existing btop code by:

- Using conditional compilation (`#ifdef AZEROTHCORE_SUPPORT`)
- Adding functionality without modifying core btop features
- Following btop's namespace and code organization patterns
- Using btop's existing configuration system
- Integrating into btop's runner thread architecture

## Future Enhancements

Potential improvements:

- Add hotkey to toggle AzerothCore monitor display
- Add AzerothCore monitor to btop boxes system (like cpu, mem, net, proc)
- Add graphical representation of bot distributions
- Add alerts for unhealthy zones
- Add support for multiple servers
- Add bot rebalancing triggers

## Original Project

This integration is based on the standalone `azerothcore` project located at:
`~/Downloads/azerothcore`

The original project was a lightweight ncurses-based monitor. This integration brings its functionality into btop while maintaining btop's architecture and style.
