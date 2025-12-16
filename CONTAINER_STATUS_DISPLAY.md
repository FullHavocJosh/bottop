# Container Status Display Feature

**Date**: December 16, 2025  
**Status**: Implemented and Tested ✅

## Overview

Added container status display feature to show the state of AzerothCore Docker containers when the server is not in ONLINE status. This provides immediate diagnostic information about which containers are running or stopped.

## Implementation Details

### When Container Status is Displayed

Container statuses are shown in the Performance pane when:

- Server status is **OFFLINE** (container not running)
- Server status is **RESTARTING** (recovering from failures)
- Server status is **REBUILDING** (database rebuild in progress)

Container list is **hidden** when server status is **ONLINE** to keep the UI clean.

### Container Filtering

**Files Modified**: `src/btop_azerothcore.cpp` (lines 1436-1448)

To prevent clutter from initialization and helper containers, only **main service containers** are displayed:

**Shown**:

- `worldserver` - Main game server process
- `authserver` - Authentication server
- `database` - MySQL/MariaDB database

**Hidden** (filtered out):

- `db-import` - Database initialization helper
- `client-data-init` - Client data extraction helper
- `bot-relocate` - Bot relocation helper
- Any other non-service containers

**Filter Logic**:

```cpp
bool is_service_container = (
    container.short_name.find("worldserver") != std::string::npos ||
    container.short_name.find("authserver") != std::string::npos ||
    container.short_name.find("database") != std::string::npos
);
```

### Container Name Display

**Name Extraction**: Full container names like `testing-ac-worldserver` are shortened to display names like `worldserver`.

**Algorithm** (`src/btop_azerothcore.cpp:1428-1434`):

1. Find `"ac-"` in the container name
2. Extract everything after `"ac-"`
3. Fallback to full name if pattern not found

**Examples**:

- `testing-ac-worldserver` → `worldserver`
- `prod-ac-authserver` → `authserver`
- `my-ac-database` → `database`

### Display Format

**Layout** (in Performance pane):

```
Status: OFFLINE

  worldserver: exited     (red)
  authserver: running     (green)
  database: running       (green)
```

**Color Coding**:

- **Green**: Container is running (state = "running")
- **Yellow**: Container is restarting or paused
- **Red**: Container is exited, dead, or in error state

### Data Collection

**Method**: `Query::fetch_container_statuses()` (`src/btop_azerothcore.cpp:1379-1446`)

**Docker Command**:

```bash
docker ps -a --filter 'name=ac-' --format '{{.Names}}|{{.State}}|{{.Status}}'
```

**Output Format**:

```
testing-ac-worldserver|running|Up 2 hours
testing-ac-authserver|running|Up 2 hours
testing-ac-database|exited|Exited (0) 5 minutes ago
```

**Parsing**:

1. Split by `|` delimiter
2. Extract: name, state, status
3. Set `is_running` flag (true if state == "running")
4. Filter to keep only service containers
5. Extract short name for display

### Integration Points

**Collection** (`src/btop_azerothcore.cpp`):

- Line 1577: Called when server is **OFFLINE**
- Line 1596: Called when server is **REBUILDING**

**Display** (`src/btop_draw.cpp:1661-1685`):

```cpp
if (data.status != ServerStatus::ONLINE && !data.containers.empty()) {
    // Show container statuses with color coding
}
```

## Testing Performed

### Test 1: OFFLINE Status with Container Display

**Steps**:

1. Stopped worldserver container: `docker stop testing-ac-worldserver`
2. Launched bottop
3. **Expected**: Status shows OFFLINE with container list
4. **Result**: ✅ Displays 3 containers (worldserver=exited, authserver=running, database=running)

### Test 2: Container Filtering

**Steps**:

1. Listed all containers with `ac-` in name (6 total)
2. Verified only service containers are shown (3 filtered)
3. **Expected**: Only worldserver, authserver, database shown
4. **Result**: ✅ Helper containers (db-import, client-data-init, bot-relocate) correctly filtered out

### Test 3: Return to ONLINE

**Steps**:

1. Started worldserver: `docker start testing-ac-worldserver`
2. Waited for server to come online
3. **Expected**: Container list disappears, status shows ONLINE
4. **Result**: ✅ Clean transition back to ONLINE status

### Test 4: REBUILDING Status

**Steps**:

1. Created rebuild marker: `echo 50 > /tmp/azerothcore_rebuild_progress.txt`
2. Launched bottop
3. **Expected**: Status shows REBUILDING [50%] with container list
4. **Result**: ✅ Marker detected, container statuses shown during rebuild

### Test 5: Container Name Extraction

**Steps**:

1. Verified name parsing with various container names
2. **Tested**:
    - `testing-ac-worldserver` → `worldserver` ✅
    - `testing-ac-authserver` → `authserver` ✅
    - `testing-ac-database` → `database` ✅
3. **Result**: ✅ All names extracted correctly

## Usage for Server Administrators

### Normal Operation

When server is ONLINE, container statuses are hidden automatically. No action needed.

### Troubleshooting with Container Display

**Scenario 1: Server shows OFFLINE**

```
Status: OFFLINE

  worldserver: exited     ← Problem: worldserver crashed
  authserver: running
  database: running
```

**Action**: Check worldserver logs, restart container

**Scenario 2: Database Issue**

```
Status: OFFLINE

  worldserver: exited
  authserver: running
  database: exited        ← Problem: database stopped
```

**Action**: Check database logs, verify data volume, restart database

**Scenario 3: Full Outage**

```
Status: OFFLINE

  worldserver: exited
  authserver: exited
  database: exited
```

**Action**: Check Docker daemon, system resources, restart all services

### During Rebuilds

```
Status: REBUILDING [45%]

  worldserver: running
  authserver: running
  database: running
```

All containers should be running during rebuild. If any show "exited", the rebuild may have failed.

## Configuration Requirements

### Container Naming Convention

Containers **must** include `"ac-"` in their name to be detected:

- ✅ `testing-ac-worldserver`
- ✅ `prod-ac-authserver`
- ✅ `dev-ac-database`
- ❌ `worldserver-testing` (will not be detected)
- ❌ `my-worldserver` (will not be detected)

### Docker Access

SSH user must have permission to run `docker ps` command:

```bash
# Test access
ssh root@server "docker ps"
```

If permission denied, add user to docker group:

```bash
usermod -aG docker username
```

## Code Structure

### Data Structures

**ContainerStatus** (`src/btop_azerothcore.hpp:403-412`):

```cpp
struct ContainerStatus {
    std::string name;        // Full name: "testing-ac-worldserver"
    std::string short_name;  // Display name: "worldserver"
    std::string state;       // Docker state: "running", "exited", etc.
    std::string status;      // Status message: "Up 2 hours", "Exited (0)"
    bool is_running;         // Quick check: true if state == "running"
};
```

**ServerData** (`src/btop_azerothcore.hpp:491`):

```cpp
struct ServerData {
    // ... other fields ...
    std::vector<ContainerStatus> containers;  // List of container statuses
};
```

### Display Logic Flow

1. **Data Collection** (`btop_azerothcore.cpp`):
    - Check if server is OFFLINE/REBUILDING
    - If yes: Call `fetch_container_statuses()`
    - Store results in `current_data.containers`

2. **Rendering** (`btop_draw.cpp:1661-1685`):
    - Check if status != ONLINE and containers vector not empty
    - Add blank line for spacing
    - Loop through each container
    - Apply color based on state
    - Format: `"  <short_name>: <state>"`

3. **Color Selection**:
    - `is_running == true` → Green (proc_misc theme color)
    - `state == "restarting"` → Yellow (#93)
    - `state == "paused"` → Yellow (#93)
    - Otherwise → Red (#91)

## Performance Considerations

### Execution Frequency

Container statuses are fetched **every collection cycle** when server is not ONLINE:

- Typical interval: 1-2 seconds
- Network overhead: ~50-100ms per SSH command
- Parsing overhead: Negligible (<1ms)

### Optimization Opportunities

**Future Enhancement 1: Status Caching**

```cpp
// Cache container statuses for N seconds
static auto last_container_check = std::time(nullptr);
if (now - last_container_check < 5) {
    return cached_containers;
}
```

**Future Enhancement 2: Async Collection**

```cpp
// Fetch container statuses in background thread
// Update display when data arrives
```

### Current Impact

- **Minimal**: Only runs when server is not ONLINE
- **No impact during normal operation** (server ONLINE)
- **Acceptable latency**: <100ms additional delay when OFFLINE

## Known Limitations

1. **Container Naming Requirement**:
    - Containers must have "ac-" in name
    - Non-standard names will not be detected

2. **Service Container Hardcoding**:
    - Only recognizes worldserver, authserver, database
    - Custom containers (e.g., "gameserver") won't be shown
    - Solution: Add custom names to filter in `fetch_container_statuses()`

3. **No Container Health Checks**:
    - Shows Docker state only (running/exited)
    - Doesn't detect application-level health
    - Container may be "running" but process inside crashed

4. **No Multi-Host Support**:
    - Assumes all containers on same host as worldserver
    - Distributed setups not supported

5. **Docker Dependency**:
    - Requires Docker containers
    - Won't work with systemd services, bare metal, VMs

## Future Enhancements

### Possible Improvements

1. **Container Health Status**:
    - Parse Docker health check results
    - Display: `worldserver: running (healthy)` vs `running (unhealthy)`

2. **Container Uptime**:
    - Show how long container has been in current state
    - Display: `worldserver: exited (5m ago)`

3. **Interactive Control**:
    - Hotkeys to start/stop/restart containers
    - Example: Press 'w' to restart worldserver

4. **Container Logs**:
    - View recent log entries for failed containers
    - Press 'l' to show last 10 lines

5. **Configurable Filter**:
    - Add config option: `azerothcore_containers = "worldserver,authserver,database"`
    - Support custom container names

6. **Resource Usage**:
    - Show CPU/memory usage per container
    - Detect resource exhaustion issues

## Related Features

This feature complements:

- **REBUILDING Status** (`REBUILDING_STATUS_TRACKING.md`) - Shows container status during rebuilds
- **Server Status Consolidation** (`STATUS_CONSOLIDATION.md`) - Unified status display
- **Performance Pane Layout** (`PERFORMANCE_PANE_LAYOUT_FIX.md`) - Layout improvements for 80-char width

## Documentation

**Files Created/Updated**:

- `CONTAINER_STATUS_DISPLAY.md` (this file)
- `src/btop_azerothcore.cpp` - Data collection
- `src/btop_azerothcore.hpp` - Data structures
- `src/btop_draw.cpp` - Display rendering

## Summary

✅ **Feature Complete**: Container status display working as designed  
✅ **Tested**: All scenarios verified (OFFLINE, REBUILDING, ONLINE)  
✅ **Filtered**: Only shows service containers (worldserver, authserver, database)  
✅ **Color-Coded**: Easy visual identification of container states  
✅ **Non-Intrusive**: Hidden during normal ONLINE operation  
✅ **Production Ready**: Suitable for live server monitoring

**Next Steps**: Monitor in production, gather user feedback, consider enhancements
