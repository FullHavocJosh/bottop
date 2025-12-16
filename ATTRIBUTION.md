# Attribution

## btop++ - The Foundation

**bottop** is built upon the excellent [btop++](https://github.com/aristocratos/btop) project by aristocratos (Jakob Arndt).

### Original Project

- **Project**: btop++
- **Author**: aristocratos (Jakob Arndt)
- **Repository**: https://github.com/aristocratos/btop
- **License**: Apache License 2.0
- **Description**: Resource monitor that shows usage and stats for processor, memory, disks, network and processes.

### What bottop Inherits from btop++

bottop uses the following core components from btop++:

- **Terminal UI Framework**: The entire TUI rendering system, including the drawing engine, themes, and menu system
- **System Monitoring**: CPU, Memory, Network, and Process monitoring capabilities for Linux
- **Configuration System**: Config file handling and runtime options
- **Input Handling**: Keyboard and mouse input processing
- **Build System**: CMake build configuration and compilation setup

### What Makes bottop Different

bottop extends btop++ with:

- **AzerothCore Integration**: Real-time monitoring of AzerothCore WoW bot servers
- **Bot Statistics**: Tracking of bot distribution, levels, zones, and factions
- **Remote SSH Monitoring**: SSH-based database queries for remote server monitoring
- **Specialized UI**: Custom display boxes for AzerothCore-specific metrics

### License

Both btop++ and bottop are licensed under the Apache License 2.0.

See the [LICENSE](LICENSE) file for the full license text.

### Acknowledgments

We are deeply grateful to aristocratos and all contributors to the btop++ project for creating such a robust and well-designed monitoring tool. bottop would not exist without their excellent work.

If you're looking for a general-purpose system monitor, we highly recommend the original [btop++](https://github.com/aristocratos/btop).
