# Bottop - Current Status

**Last Updated:** December 13, 2025, 19:01 EST  
**Binary Status:** ✅ **READY TO TEST**

---

## What Is Bottop?

Bottop is a fork of `btop` enhanced with **AzerothCore WorldServer monitoring** capabilities. It displays real-time performance metrics from your AzerothCore game server alongside traditional system monitoring.

---

## Current Implementation Status

### ✅ Complete and Working

1. **Configuration System**
    - Environment variable support (BOTTOP*AC*\*)
    - Config file: `~/.config/bottop/bottop.conf`
    - Priority: ENV → Config → Defaults

2. **SSH Connection**
    - Connects to remote server via SSH
    - Key-based authentication
    - Executes Docker commands remotely

3. **Database Monitoring**
    - Bot statistics (count, distribution)
    - Zone health metrics
    - Level brackets
    - Continental distribution

4. **WorldServer Performance Monitoring** (Code Complete)
    - ✅ Data structure defined (12 metrics)
    - ✅ Parsing logic implemented
    - ✅ Display code ready
    - ✅ Color-coded performance indicators
    - ✅ Real-time graph
    - ⚠️ **NEEDS TESTING** - Container not currently running

---

## WorldServer Performance Features

### Metrics Displayed

When WorldServer container is running, you'll see:

```
┌─ AzerothCore Server ─────────────────────────┐
│ Status: ONLINE                                │
│                                               │
│ WorldServer Performance                       │
│   Rev: ece1060fa05d+ (Testing-Playerbot)     │
│        [RelWithDebInfo]                       │
│   Players: 0  Characters: 2842  Peak: 0      │
│   Uptime: 4 minute(s) 56 second(s)           │
│   Current: 174ms     ← Color-coded            │
│   Mean: 96ms  Median: 77ms                   │
│   P95: 186ms  P99: 198ms  Max: 227ms         │
│                                               │
│   [Real-time graph showing update times]      │
└───────────────────────────────────────────────┘
```

### Color Coding

- **Green** (< 50ms) - Excellent performance
- **Light Green** (50-99ms) - Good performance
- **Yellow** (100-149ms) - Acceptable performance
- **Red** (≥ 150ms) - Poor performance

### Data Points

| Metric         | Description                         |
| -------------- | ----------------------------------- |
| **Revision**   | Git commit hash + date              |
| **Branch**     | Git branch name                     |
| **Build Type** | RelWithDebInfo/Debug/Release        |
| **Players**    | Currently connected players         |
| **Characters** | Total characters in database        |
| **Peak**       | Highest concurrent connection count |
| **Uptime**     | Server uptime string + seconds      |
| **Current**    | Current tick duration (ms)          |
| **Mean**       | Average of last 500 ticks           |
| **Median**     | Middle value of last 500 ticks      |
| **P95**        | 95th percentile (500 ticks)         |
| **P99**        | 99th percentile (500 ticks)         |
| **Max**        | Maximum tick time (500 ticks)       |

---

## How It Works

### Command Flow

```bash
# Local machine runs bottop
./build/bottop

# Bottop connects via SSH
ssh root@testing-azerothcore.rollet.family

# Executes Docker command on remote
docker exec -i testing-ac-worldserver \
  testing-ac-worldserver 'server info'

# WorldServer console responds
AC> server info
AC> AzerothCore rev. ece1060fa05d+ ...
Connected players: 0. Characters in world: 2842.
...
AC>

# Bottop parses output
# Strips AC> prefixes
# Extracts all metrics
# Displays with color coding
# Updates graph every second
```

### The Console Script

The WorldServer container includes a console script named after the container itself:

- Container name: `testing-ac-worldserver`
- Console script: `/usr/local/bin/testing-ac-worldserver` (or similar)
- Usage: `testing-ac-worldserver 'command'`

This is a **container-specific naming pattern** - your container name determines the console script name.

---

## Current Configuration

**Location:** `~/.config/bottop/bottop.conf`

```ini
#* Enable AzerothCore monitoring
azerothcore_enabled = True

#* SSH connection
azerothcore_ssh_host = "root@testing-azerothcore.rollet.family"

#* Docker container name (also used as console script name)
azerothcore_container = "testing-ac-worldserver"

#* Database credentials
azerothcore_db_host = "testing-ac-database"
azerothcore_db_user = "root"
azerothcore_db_pass = "password"
azerothcore_db_name = "acore_characters"
```

---

## Testing Instructions

### Prerequisites

1. **Start WorldServer container:**

    ```bash
    # On remote server (testing-azerothcore.rollet.family)
    docker start testing-ac-worldserver

    # Verify it's running
    docker ps | grep testing-ac-worldserver
    ```

2. **Test console command manually:**

    ```bash
    ssh root@testing-azerothcore.rollet.family \
      "docker exec -i testing-ac-worldserver testing-ac-worldserver 'server info'"
    ```

    You should see output like:

    ```
    AC> server info
    AC> AzerothCore rev. ece1060fa05d+ 2025-12-12 19:37:01 +0000 (Testing-Playerbot branch) (Unix, RelWithDebInfo, Static)
    Connected players: 0. Characters in world: 2842.
    Connection peak: 0.
    Server uptime: 4 minute(s) 56 second(s)
    Update time diff: 174ms. Last 500 diffs summary:
    |- Mean: 96ms
    |- Median: 77ms
    |- Percentiles (95, 99, max): 186ms, 198ms, 227ms
    AC>
    ```

### Run Bottop

```bash
/home/havoc/bottop/build/bottop
```

### What You Should See

1. **If WorldServer is running:**
    - "WorldServer Performance" section with all 12 metrics
    - Color-coded current update time
    - Real-time graph updating every second
    - All percentile statistics

2. **If WorldServer is not running:**
    - Fallback to "Database Query Time" display
    - Bot statistics from database
    - Zone and level distribution

### Debug Information

If you don't see performance data:

```bash
# Check debug log
cat /tmp/bottop_debug.txt

# You should see:
# - "fetch_server_performance: Executing: docker exec -i testing-ac-worldserver testing-ac-worldserver 'server info' 2>/dev/null"
# - "fetch_server_performance: Result length: NNN"
# - Raw output from server info command
```

---

## Code Architecture

### Files Modified

| File                       | Purpose               | Lines     |
| -------------------------- | --------------------- | --------- |
| `src/btop_azerothcore.cpp` | Main monitoring logic | 387-640   |
| `src/btop_azerothcore.hpp` | Data structures       | 50-75     |
| `src/btop_draw.cpp`        | Display rendering     | 1634-1696 |
| `src/btop_config.cpp`      | Configuration         | Multiple  |

### Key Functions

```cpp
// Fetch server performance data
ServerPerformance fetch_server_performance();

// Parse server info output (Lines 412-640)
// - Strips AC> prefixes
// - Extracts version info
// - Parses player counts
// - Parses uptime
// - Extracts all performance metrics

// Display (Lines 1634-1696 in btop_draw.cpp)
// - Shows all 12 metrics
// - Color codes current update time
// - Draws real-time graph
// - Falls back to database query time if unavailable
```

### Data Flow

```
1. Config loaded (ENV or file)
2. SSH connection established
3. Every 1 second:
   a. Execute: docker exec -i <container> <container> 'server info'
   b. Parse output
   c. Update data structures
   d. Render display
   e. Update graph
4. History: 300 samples (~5 minutes)
```

---

## Known Issues

### Container Not Running

**Symptom:** Display shows "Database Query Time" instead of "WorldServer Performance"

**Cause:** The `testing-ac-worldserver` container is not running

**Solution:**

```bash
ssh root@testing-azerothcore.rollet.family "docker start testing-ac-worldserver"
```

### Console Script Not Found

**Symptom:** No server info output, debug log shows empty result

**Possible causes:**

1. Container uses different console script name
2. Console script not in PATH
3. Console not enabled in worldserver.conf

**Debug:**

```bash
# Find console script
ssh root@testing-azerothcore.rollet.family \
  "docker exec testing-ac-worldserver find / -name '*console*' -o -name '*worldserver*' 2>/dev/null"

# Check if script is executable
ssh root@testing-azerothcore.rollet.family \
  "docker exec testing-ac-worldserver ls -la /usr/local/bin/"
```

### Permission Issues

**Symptom:** "Permission denied" errors

**Solution:** Ensure SSH key-based auth is configured for root@testing-azerothcore.rollet.family

---

## Build Information

**Binary:** `/home/havoc/bottop/build/bottop`  
**Size:** 2.3 MB  
**Built:** December 13, 2025, 19:01:51 EST  
**Compiler:** g++ (CMake)  
**Dependencies:** libssh2, ncurses, pthread  
**Warnings:** 0 (in our code)  
**Errors:** 0

### Rebuild Instructions

```bash
cd /home/havoc/bottop/build
make -j4
```

Build time: ~10 seconds on 4 cores

---

## Next Steps

### Immediate

1. **Start WorldServer container** on remote server
2. **Test console command** manually (see Testing Instructions above)
3. **Run bottop** and verify display
4. **Check debug log** if issues occur

### Future Enhancements

- [ ] Add hotkey to toggle WorldServer display
- [ ] Add alerts for poor performance (>150ms)
- [ ] Add historical performance tracking
- [ ] Support multiple servers
- [ ] Add RA (Remote Access) port support as fallback
- [ ] Add automatic reconnection on SSH failure

---

## Documentation Files

| File                                  | Purpose                        |
| ------------------------------------- | ------------------------------ |
| `CURRENT_STATUS.md`                   | This file - overall status     |
| `AZEROTHCORE_INTEGRATION.md`          | Original integration docs      |
| `WORLDSERVER_PERFORMANCE_COMPLETE.md` | Feature implementation details |
| `WORLDSERVER_FIX_FINAL.md`            | Console command fix docs       |
| `CONFIGURATION.md`                    | Configuration guide            |
| `TESTING_GUIDE.md`                    | Testing procedures             |

---

## Questions?

If you encounter issues:

1. Check the container is running: `docker ps | grep worldserver`
2. Test the command manually (see Testing Instructions)
3. Check debug log: `cat /tmp/bottop_debug.txt`
4. Review error output: Run command without `2>/dev/null`

---

**Status:** ✅ Code complete, ready for testing  
**Binary:** `/home/havoc/bottop/build/bottop`  
**Action Required:** Start WorldServer container and test
