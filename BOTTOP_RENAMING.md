# BOTTOP Renaming Complete

This document summarizes the renaming from btop to **bottop**.

## What Changed

### 1. Binary Name

- **Old**: `btop`
- **New**: `bottop`
- Built binary: `build/bottop`
- Install location: `bin/bottop`

### 2. Configuration Directory

- **Old**: `~/.config/btop/`
- **New**: `~/.config/bottop/`
- Config file: `~/.config/bottop/bottop.conf`
- Themes: `~/.config/bottop/themes/`

### 3. System Theme Paths

- **Old**: `/usr/share/btop/themes`, `/usr/local/share/btop/themes`
- **New**: `/usr/share/bottop/themes`, `/usr/local/share/bottop/themes`

### 4. ASCII Banner

Changed from:

```
██████╗ ████████╗ ██████╗ ██████╗
██╔══██╗╚══██╔══╝██╔═══██╗██╔══██╗
██████╔╝   ██║   ██║   ██║██████╔╝
██╔══██╗   ██║   ██║   ██║██╔═══╝
██████╔╝   ██║   ╚██████╔╝██║
╚═════╝    ╚═╝    ╚═════╝ ╚═╝
```

To:

```
██████╗  ██████╗ ████████╗████████╗ ██████╗ ██████╗
██╔══██╗██╔═══██╗╚══██╔══╝╚══██╔══╝██╔═══██╗██╔══██╗
██████╔╝██║   ██║   ██║      ██║   ██║   ██║██████╔╝
██╔══██╗██║   ██║   ██║      ██║   ██║   ██║██╔═══╝
██████╔╝╚██████╔╝   ██║      ██║   ╚██████╔╝██║
╚═════╝  ╚═════╝    ╚═╝      ╚═╝    ╚═════╝ ╚═╝
```

### 5. User-Facing Strings

- Error messages now say "bottop" instead of "btop++"
- Log messages say "bottop" instead of "btop++"
- Theme descriptions reference "bottop" paths

### 6. Project Metadata

- CMake project name: `bottop`
- Description: "AzerothCore Bot Monitor - A fork of btop with WoW bot monitoring"

## Files Modified

### Source Files

- `src/btop.cpp` - Banner, error messages, theme paths
- `src/btop_config.cpp` - Config directory, theme path descriptions
- `src/btop_menu.cpp` - Theme path descriptions
- `src/btop_tools.cpp` - Log messages
- `src/btop_draw.hpp` - Comment about banner

### Build Files

- `CMakeLists.txt` - Project name and description

### Helper Scripts

- `install_binary.sh` - Updated to install as `bottop`

## How to Install

```bash
cd /home/havoc/bottop

# Make install script executable
chmod +x install_binary.sh

# Install the binary
./install_binary.sh
```

This will:

1. Create `bin/` directory if needed
2. Backup any existing `bin/bottop` to `bin/bottop.old`
3. Remove old `bin/btop` if present
4. Copy `build/bottop` to `bin/bottop`
5. Set executable permissions

## How to Run

```bash
# From the bottop directory
./bin/bottop

# Or add to PATH and run from anywhere
export PATH="/home/havoc/bottop/bin:$PATH"
bottop
```

## Configuration

On first run, bottop will create:

- `~/.config/bottop/` directory
- `~/.config/bottop/bottop.conf` config file
- `~/.config/bottop/themes/` themes directory (if custom themes added)

### AzerothCore Configuration

Edit `~/.config/bottop/bottop.conf` and set:

```conf
#* AzerothCore Bot Monitoring
azerothcore_enabled = True
azerothcore_ssh_host = "user@hostname"
azerothcore_db_host = "database-container-name"
azerothcore_db_user = "root"
azerothcore_db_pass = "password"
azerothcore_db_name = "acore_characters"
azerothcore_container = "worldserver-container-name"
azerothcore_config_path = "/path/to/worldserver.conf"
```

## Error Handling

The application now has robust error handling:

- SSH connection failures are logged and displayed, not crash the app
- Database query errors are logged and displayed
- AzerothCore collection errors are logged and displayed
- Drawing errors are caught and skipped for that cycle

Errors will appear in the AzerothCore section instead of crashing the entire application.

## What's Different from btop?

**bottop** is a fork of btop++ with these key differences:

1. **AzerothCore Bot Monitoring**: Integrated monitoring for AzerothCore WoW bot servers
2. **Split Screen Layout**: 40% performance metrics, 60% zone details
3. **Real-Time Bot Tracking**: Shows online bots, distribution by continent/faction/level, zone alignment
4. **Server Performance**: Tracks worldserver uptime and update times
5. **Ollama Integration**: Optional tracking of mod-ollama-chat statistics
6. **Independent Config**: Uses `~/.config/bottop/` instead of `~/.config/btop/`

## Original btop++ Credits

bottop is based on btop++ by aristocratos:

- GitHub: https://github.com/aristocratos/btop
- License: Apache 2.0

## Support

For issues specific to bottop (AzerothCore features), please report them separately from the upstream btop++ project.
