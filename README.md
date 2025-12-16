# bottop - AzerothCore Bot Monitor

![Linux](https://img.shields.io/badge/-Linux-grey?logo=linux)
![c++23](https://img.shields.io/badge/cpp-c%2B%2B23-green)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

**bottop** is a specialized TUI monitor for [AzerothCore](https://www.azerothcore.org/) WoW bot servers. Built on the foundation of [btop++](https://github.com/aristocratos/btop), it provides real-time monitoring of bot distributions, zone health, and server performance via SSH.

## What bottop Monitors

### Remote AzerothCore Server (via SSH)

- **Real-time bot statistics**: Total bots, online players, server capacity
- **Distribution tracking**: Bot spread across continents, factions, and level brackets
- **Zone health monitoring**: Identify zones with bot concentration issues
- **Server performance**: Worldserver uptime and update times
- **Database integration**: Real-time MySQL queries via SSH connection

**Note**: bottop focuses **exclusively** on remote AzerothCore monitoring. It does not monitor local system resources (CPU, Memory, Network).

## Screenshots

_Coming soon_

## Installation

### Prerequisites

- **Linux** (x86_64) - Currently Linux-only
- **GCC 14+** or **Clang 19+**
- **CMake 3.25+**
- **libssh2** - For SSH connectivity to AzerothCore servers

#### Install Dependencies (Ubuntu/Debian)

```bash
sudo apt install build-essential cmake git libssh2-1-dev lowdown
```

#### Install Dependencies (Fedora/RHEL)

```bash
sudo dnf install gcc-c++ cmake git libssh2-devel lowdown
```

### Compilation

```bash
git clone https://github.com/yourusername/bottop.git
cd bottop
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Installation

```bash
sudo make install
```

This installs:

- Binary: `/usr/local/bin/bottop`
- Themes: `/usr/local/share/bottop/themes/`
- Desktop entry: `/usr/local/share/applications/`
- Icons: `/usr/local/share/icons/hicolor/`

## Configuration

On first run, bottop creates `~/.config/bottop/bottop.conf`

### AzerothCore Setup

Edit `~/.config/bottop/bottop.conf`:

```ini
#* Enable AzerothCore bot monitoring
azerothcore_enabled = True

#* SSH connection (format: user@hostname or user@hostname:port)
azerothcore_ssh_host = "root@your-server.example.com"

#* Database configuration
azerothcore_db_host = "database-container-name"
azerothcore_db_user = "root"
azerothcore_db_pass = "password"
azerothcore_db_name = "acore_characters"

#* Docker container name for worldserver
azerothcore_container = "ac-worldserver"
```

### SSH Key Authentication

bottop requires SSH key-based authentication:

```bash
# Generate SSH key if you don't have one
ssh-keygen -t ed25519 -C "bottop-monitor"

# Copy to your server
ssh-copy-id user@your-server.example.com
```

## Usage

```bash
bottop
```

### Command Line Options

```
Usage: bottop [OPTIONS]

Options:
  -c, --config <file>     Path to a config file
  -d, --debug             Start in debug mode with additional logs
  -h, --help              Show help message and exit
  -V, --version           Show version and exit
```

### Keybindings

- `q` - Quit
- `Esc` - Close menus
- `m` - Main menu
- `o` - Options menu
- `h` - Help menu
- `F2` - Show/hide menu

## What Makes bottop Different from btop++

bottop is **not** a general-purpose system monitor. It's a specialized tool for monitoring AzerothCore bot servers remotely via SSH.

| Feature                       | bottop                        | btop++                                 |
| ----------------------------- | ----------------------------- | -------------------------------------- |
| **Primary Purpose**           | Remote AzerothCore monitoring | Local system monitoring                |
| **Platform Support**          | Linux only                    | Linux, macOS, FreeBSD, NetBSD, OpenBSD |
| **Remote Monitoring**         | ✅ Yes (SSH-based)            | ❌ No                                  |
| **AzerothCore Bot Tracking**  | ✅ Yes                        | ❌ No                                  |
| **Bot Distribution Analysis** | ✅ Yes                        | ❌ No                                  |
| **Zone Health Monitoring**    | ✅ Yes                        | ❌ No                                  |
| **Local CPU/Memory/Network**  | ❌ No                         | ✅ Yes                                 |
| **GPU Monitoring**            | ❌ No                         | ✅ Yes                                 |
| **Disk I/O**                  | ❌ No                         | ✅ Yes                                 |

## Monitoring Architecture

```
┌─────────────────┐         SSH          ┌──────────────────────┐
│   Local Machine │ ◄─────────────────► │  AzerothCore Server  │
│   (runs bottop) │                      │                      │
│                 │                      │  ┌────────────────┐  │
│  ┌───────────┐  │    MySQL Queries    │  │   Database     │  │
│  │  bottop   │──┼─────────────────────┼─►│ (characters)   │  │
│  │    TUI    │  │                      │  └────────────────┘  │
│  └───────────┘  │                      │                      │
│                 │                      │  ┌────────────────┐  │
│  Displays:      │                      │  │  Worldserver   │  │
│  • Bot stats    │                      │  │   (running)    │  │
│  • Zone health  │                      │  └────────────────┘  │
│  • Distribution │                      │                      │
└─────────────────┘                      └──────────────────────┘
```

## Features in Detail

### Bot Statistics

- Total bot count
- Online vs offline bots
- Server player capacity
- Worldserver uptime

### Distribution Tracking

- Bot spread across continents (Eastern Kingdoms, Kalimdor, Outland, Northrend)
- Faction distribution (Alliance, Horde, Neutral)
- Level bracket analysis (1-10, 11-20, 21-30, etc.)

### Zone Health Monitoring

- Identify zones with excessive bot concentrations
- Track bot level alignment with zone level ranges
- Highlight unhealthy zones needing rebalancing
- Top 15 zones by bot population

### Server Performance

- Worldserver uptime tracking
- Average update time monitoring
- Update time differential analysis

## Project Status

bottop is in active development. It is a specialized fork focused exclusively on remote AzerothCore server monitoring.

**Current Status**: Stable, production-ready
**Focus**: Remote bot monitoring only
**Future Plans**:

- Multi-server monitoring support
- Enhanced zone analysis and recommendations
- Alert system for unhealthy zones
- Historical data tracking

For general local system monitoring, we recommend using the original [btop++](https://github.com/aristocratos/btop).

## Attribution

bottop is built upon [btop++](https://github.com/aristocratos/btop) by aristocratos (Jakob Arndt). While bottop uses btop++'s TUI framework and architecture, all system monitoring has been removed to focus purely on AzerothCore bot monitoring.

See [ATTRIBUTION.md](ATTRIBUTION.md) for detailed credits.

## License

Apache License 2.0 - See [LICENSE](LICENSE) file for details.

Same license as the original btop++ project.

## Links

- **btop++ (upstream)**: https://github.com/aristocratos/btop
- **AzerothCore**: https://www.azerothcore.org/
- **Issues**: Report bottop-specific issues on our repository
- **For btop++ issues**: Report to https://github.com/aristocratos/btop/issues

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

For major changes, please open an issue first to discuss what you would like to change.

## Support

If you find bottop useful for managing your AzerothCore server, consider:

- ⭐ Starring the repository
- 🐛 Reporting bugs and issues
- 💡 Suggesting new features
- 📖 Improving documentation

## Acknowledgments

- **aristocratos** - Creator of btop++, which provided the TUI foundation
- **AzerothCore Team** - For the amazing WoW server emulator
- All contributors to both projects

---

**Note**: bottop is a specialized tool for AzerothCore monitoring. For general-purpose system monitoring, use the original [btop++](https://github.com/aristocratos/btop)!
