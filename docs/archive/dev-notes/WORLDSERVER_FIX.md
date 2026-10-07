# WorldServer Performance Display Fix

**Date:** December 13, 2025  
**Status:** ✅ Fixed and Rebuilt

---

## Problem

The WorldServer performance data from `server info` command was not displaying in Bottop. Instead, only "Database Query Time" was shown.

## Root Cause

The issue was in the command used to fetch server info from the worldserver:

**Incorrect Command (before):**

```bash
docker exec testing-ac-worldserver worldserver-console 'server info'
```

**Correct Command (after):**

```bash
docker exec -i testing-ac-worldserver /bin/bash -c 'echo "server info" | socat - UNIX-CONNECT:/azerothcore/data/worldserver.sock'
```

### Why the Fix Works

1. **Uses socat** - Properly connects to the UNIX socket at `/azerothcore/data/worldserver.sock`
2. **Adds -i flag** - Makes docker exec interactive so it can read from stdin
3. **Pipes command** - Sends "server info" command through the socket
4. **Returns full output** - Gets the complete response with all metrics

---

## Changes Made

### File: `src/btop_azerothcore.cpp`

#### Change 1: Updated Command (Lines 385-388)

```cpp
// Before:
cmd << "docker exec " << config_.container
    << " worldserver-console 'server info' 2>/dev/null";

// After:
cmd << "docker exec -i " << config_.container
    << " /bin/bash -c 'echo \"server info\" | socat - UNIX-CONNECT:/azerothcore/data/worldserver.sock' 2>/dev/null";
```

#### Change 2: Added AC> Prefix Stripping (Lines 412-420)

```cpp
while (std::getline(stream, line)) {
    // Strip "AC>" prefix if present
    if (line.find("AC>") == 0) {
        line = line.substr(3);
        // Trim leading whitespace
        line.erase(0, line.find_first_not_of(" \t"));
    }

    // Skip empty lines
    if (line.empty()) continue;

    // ... rest of parsing logic
}
```

#### Change 3: Added Debug Logging (Lines 390-391)

```cpp
Logger::debug("fetch_server_performance: Executing: " + cmd.str());
std::string result = ssh_.execute(cmd.str());
Logger::debug("fetch_server_performance: Result length: " + std::to_string(result.length()));
```

---

## Expected Output

After the fix, when you run Bottop, you should see:

```
┌─ AzerothCore Server ─────────────────────────┐
│ Status: ONLINE                                │
│ Server Uptime: 4 minute(s) 56 second(s)      │
│                                               │
│ WorldServer Performance                       │
│   Rev: ece1060fa05d+ (Testing-Playerbot)     │
│        [RelWithDebInfo]                       │
│   Players: 0  Characters: 2842  Peak: 0      │
│   Uptime: 4 minute(s) 56 second(s)           │
│   Current: 174ms     ← Yellow (100-149ms)     │
│   Mean: 96ms  Median: 77ms                   │
│   P95: 186ms  P99: 198ms  Max: 227ms         │
│                                               │
│   [Graph showing real-time update times]      │
└───────────────────────────────────────────────┘
```

### Instead of (old behavior):

```
┌─ AzerothCore Server ─────────────────────────┐
│ Status: ONLINE                                │
│ Server Uptime: 4 minute(s) 56 second(s)      │
│                                               │
│ Database Query Time: 45ms                     │
│                                               │
│   [Graph showing database query times]        │
└───────────────────────────────────────────────┘
```

---

## Verification Steps

### 1. Verify Config

Check that your configuration has the correct values:

```bash
cat ~/.config/bottop/bottop.conf | grep azerothcore
```

Expected output:

```
azerothcore_ssh_host = "root@testing-azerothcore.rollet.family"
azerothcore_container = "testing-ac-worldserver"
azerothcore_db_host = "testing-ac-database"
azerothcore_db_user = "root"
azerothcore_db_pass = "password"
azerothcore_db_name = "acore_characters"
```

### 2. Test Command Manually

Test if the command works from your shell:

```bash
# Set variables (adjust if different)
SSH_HOST="root@testing-azerothcore.rollet.family"
CONTAINER="testing-ac-worldserver"

# Test the command
ssh ${SSH_HOST} "docker exec -i ${CONTAINER} /bin/bash -c 'echo \"server info\" | socat - UNIX-CONNECT:/azerothcore/data/worldserver.sock'"
```

Expected output:

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

### 3. Run Bottop

```bash
/home/havoc/bottop/build/bottop
```

Look for:

- ✅ "WorldServer Performance" section (not "Database Query Time")
- ✅ Server revision and branch displayed
- ✅ Player counts (connected, characters, peak)
- ✅ Server uptime
- ✅ All 6 performance metrics (Current, Mean, Median, P95, P99, Max)
- ✅ Color-coded current update time
- ✅ Real-time graph updating

### 4. Check Debug Logs (Optional)

If you want to see debug output:

```bash
# Run with debug flag (if supported)
/home/havoc/bottop/build/bottop --debug
```

Look for lines like:

```
[DEBUG] fetch_server_performance: Executing: docker exec -i testing-ac-worldserver /bin/bash -c 'echo "server info" | socat - UNIX-CONNECT:/azerothcore/data/worldserver.sock'
[DEBUG] fetch_server_performance: Result length: 423
[DEBUG] fetch_server_performance: diff=174ms, mean=96ms, median=77ms
```

---

## Troubleshooting

### Issue: Still seeing "Database Query Time"

**Possible causes:**

1. **WorldServer is offline**
    - Check: `ssh ${SSH_HOST} "docker ps | grep worldserver"`
    - Expected: Container should be running

2. **Socket doesn't exist**
    - Check: `ssh ${SSH_HOST} "docker exec -i ${CONTAINER} ls -la /azerothcore/data/worldserver.sock"`
    - Expected: Socket file should exist

3. **socat not installed**
    - Check: `ssh ${SSH_HOST} "docker exec -i ${CONTAINER} which socat"`
    - Expected: Path to socat binary (e.g., `/usr/bin/socat`)
    - Fix: Install socat in container

4. **Socket path is different**
    - Check worldserver config for socket location
    - May need to adjust path in code

5. **Permissions issue**
    - Socket may not be readable
    - Check: `ssh ${SSH_HOST} "docker exec -i ${CONTAINER} stat /azerothcore/data/worldserver.sock"`

### Issue: Command returns empty

**Debug steps:**

1. Test without stderr redirection:

    ```bash
    ssh ${SSH_HOST} "docker exec -i ${CONTAINER} /bin/bash -c 'echo \"server info\" | socat - UNIX-CONNECT:/azerothcore/data/worldserver.sock'"
    ```

2. Check if socket is listening:

    ```bash
    ssh ${SSH_HOST} "docker exec -i ${CONTAINER} netstat -anp | grep worldserver.sock"
    ```

3. Try connecting manually:
    ```bash
    ssh ${SSH_HOST} "docker exec -it ${CONTAINER} /bin/bash"
    # Then inside container:
    echo "server info" | socat - UNIX-CONNECT:/azerothcore/data/worldserver.sock
    ```

### Issue: Parsing errors

**Symptoms:** Data is fetched but not displayed correctly

**Debug:**

1. Check debug logs for parsing errors
2. Verify output format matches expected:
    - First line: `AC> server info` (or just command echo)
    - Second line: `AC> AzerothCore rev. ...`
    - Third line: `Connected players: ...`
    - etc.

3. If format is different, may need to adjust parsing in `btop_azerothcore.cpp`

---

## Technical Details

### Socket Communication

The worldserver exposes a UNIX domain socket for console access:

- **Location:** `/azerothcore/data/worldserver.sock`
- **Type:** UNIX stream socket
- **Protocol:** Line-based text commands
- **Prompt:** `AC>` prefix on responses

### Command Flow

1. **SSH** to remote host
2. **Docker exec** into worldserver container
3. **Bash** interprets the echo command
4. **Echo** outputs "server info"
5. **Pipe** connects echo to socat
6. **Socat** sends to UNIX socket
7. **WorldServer** processes command
8. **Response** comes back through socket
9. **Socat** outputs to stdout
10. **Bottop** parses the output

### Parsing Logic

The parser handles:

- ✅ `AC>` prefix stripping
- ✅ Empty line skipping
- ✅ Revision extraction (git hash + dirty flag)
- ✅ Branch name extraction
- ✅ Build type extraction (RelWithDebInfo, Debug, Release)
- ✅ Build date/time
- ✅ Player metrics (connected, characters in world, peak)
- ✅ Server uptime (human readable + seconds calculation)
- ✅ Current update time (ms)
- ✅ Statistical metrics (mean, median, P95, P99, max)

### Data Structure

```cpp
struct ServerPerformance {
    std::string revision;           // "ece1060fa05d+"
    std::string branch;             // "Testing-Playerbot"
    std::string build_date;         // "2025-12-12 19:37:01 +0000"
    std::string build_type;         // "RelWithDebInfo"

    int connected_players;          // 0
    int characters_in_world;        // 2842
    int connection_peak;            // 0

    std::string uptime;             // "4 minute(s) 56 second(s)"
    long long uptime_seconds;       // 296

    long long update_time_diff;     // 174ms (current)
    long long mean;                 // 96ms
    long long median;               // 77ms
    long long p95;                  // 186ms
    long long p99;                  // 198ms
    long long max;                  // 227ms

    bool available;                 // true if data parsed
};
```

---

## Build Information

**Build Date:** December 13, 2025  
**Binary:** `/home/havoc/bottop/build/bottop`  
**Version:** 1.4.5+871c1db  
**Size:** 2.2M  
**Build Status:** ✅ Success (no errors)

---

## Related Files

- `src/btop_azerothcore.cpp` - Collection and parsing logic
- `src/btop_azerothcore.hpp` - Data structures
- `src/btop_draw.cpp` - Display rendering
- `WORLDSERVER_PERFORMANCE_COMPLETE.md` - Complete feature documentation
- `BUILD_STATUS.md` - Build and warning details

---

## Summary

The fix involved:

1. ✅ Correcting the command to use socat + UNIX socket
2. ✅ Adding AC> prefix handling in parser
3. ✅ Adding debug logging for troubleshooting
4. ✅ Successfully rebuilt with no errors

The WorldServer performance monitoring should now work correctly and display all metrics from the `server info` command in real-time.

**Status:** ✅ **READY TO TEST**

Run `/home/havoc/bottop/build/bottop` to verify the fix!
