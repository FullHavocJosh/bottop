# WorldServer Performance Only When ONLINE - COMPLETE ✅

**Date:** December 14, 2025, 15:34 EST  
**Status:** Built and Ready

## Overview

Modified the system to only collect and display WorldServer performance data when the server status is ONLINE. When the server is OFFLINE, RESTARTING, or in ERROR state, the performance data is cleared and not displayed.

## The Problem

Previously, WorldServer performance data would persist and show cached values even when the server was not online. This could be misleading, as users might see performance metrics for a server that's actually down.

## The Solution

Added logic to clear performance data (`perf.available = false`, `perf.is_cached = false`) when:

1. Server status is OFFLINE or RESTARTING
2. An error occurs during data collection

## Code Changes

### File: `src/btop_azerothcore.cpp` (lines 1193-1248)

**Added performance data clearing in two places:**

#### 1. When Server is Not Online (OFFLINE/RESTARTING)

```cpp
if (!server_online) {
    current_data.status = ServerStatus::OFFLINE;
    current_data.consecutive_failures++;

    // After 3 failures, assume restarting
    if (current_data.consecutive_failures >= 3) {
        current_data.status = ServerStatus::RESTARTING;
    }

    current_data.error = "Server container is not running";

    // Clear performance data when server is not online
    current_data.stats.perf.available = false;
    current_data.stats.perf.is_cached = false;

    return;
}
```

#### 2. When Error Occurs

```cpp
} catch (const std::exception& e) {
    current_data.error = std::string("Collect error: ") + e.what();
    current_data.status = ServerStatus::ERROR;
    current_data.consecutive_failures++;

    // Clear performance data on error
    current_data.stats.perf.available = false;
    current_data.stats.perf.is_cached = false;
}
```

## Behavior

### When Server is ONLINE ✅

```
Server Status: 🟢 ONLINE
Server Uptime: 2h
WorldServer UpdateTime: 141ms [Median]
<graph>
```

Performance data is collected and displayed normally.

### When Server is OFFLINE 🔴

```
Server Status: 🔴 OFFLINE
Server Uptime: 2h
<graph with last known values>
```

- **WorldServer UpdateTime line is NOT displayed**
- Performance data is cleared (`available = false`)
- No cached data is shown
- Graph may show previous history but won't update

### When Server is RESTARTING 🟡

```
Server Status: 🟡 RESTARTING
Server Uptime: 2h
<graph>
```

- **WorldServer UpdateTime line is NOT displayed**
- Performance data is cleared
- Appears after 3 consecutive failures to detect online status

### When Server has ERROR ⚠️

```
Server Status: ⚠️ ERROR
Server Uptime: 2h
<graph>
```

- **WorldServer UpdateTime line is NOT displayed**
- Performance data is cleared
- Error message may be shown in the pane

## Logic Flow

```
┌─────────────────────────────┐
│  Check Server Online?       │
└──────────┬──────────────────┘
           │
           ├─ YES → Fetch all data (including performance)
           │        └─ Status: ONLINE
           │           Display: WorldServer UpdateTime shown ✅
           │
           └─ NO  → Clear performance data
                    └─ Status: OFFLINE or RESTARTING
                       Display: WorldServer UpdateTime hidden ❌
```

## What Gets Cleared

When server is not ONLINE, these flags are reset:

```cpp
current_data.stats.perf.available = false;   // Marks data as unavailable
current_data.stats.perf.is_cached = false;   // Clears cache flag
```

This causes the display code to skip rendering the WorldServer UpdateTime line entirely (since `data.stats.perf.available` is checked before display).

## Display Logic (Unchanged)

The display code already checks `perf.available`:

```cpp
if (data.stats.perf.available) {
    // Display WorldServer UpdateTime
    out += Mv::to(cy++, perf_x + 2) + title + "WorldServer UpdateTime: "
        + diff_color + to_string(data.stats.perf.median) + "ms "
        + Theme::c("graph_text") + metric_type;
}
```

When `available = false`, this entire block is skipped, so the line isn't rendered.

## Benefits

✅ **Accuracy** - No misleading performance data when server is down  
✅ **Clarity** - Users immediately see that performance isn't being collected  
✅ **Clean Display** - No stale or cached data shown for offline servers  
✅ **Consistency** - Performance data state matches server state

## Example Scenarios

### Scenario 1: Server Goes Offline

```
Time    Status        Display
-----   -------       ----------------------------------
10:00   🟢 ONLINE      WorldServer UpdateTime: 141ms [Median]
10:01   🟢 ONLINE      WorldServer UpdateTime: 143ms [Median]
10:02   🔴 OFFLINE     (no WorldServer UpdateTime line)
10:03   🔴 OFFLINE     (no WorldServer UpdateTime line)
10:04   🟡 RESTARTING  (no WorldServer UpdateTime line)
10:05   🟢 ONLINE      WorldServer UpdateTime: 145ms [Median]
```

### Scenario 2: Collection Error

```
Time    Status        Display
-----   -------       ----------------------------------
10:00   🟢 ONLINE      WorldServer UpdateTime: 141ms [Median]
10:01   ⚠️ ERROR       (no WorldServer UpdateTime line)
10:02   🟢 ONLINE      WorldServer UpdateTime: 142ms [Median]
```

### Scenario 3: Normal Operation

```
Time    Status        Display
-----   -------       ----------------------------------
10:00   🟢 ONLINE      WorldServer UpdateTime: 141ms [Median]
10:01   🟢 ONLINE      WorldServer UpdateTime: 143ms [Median]
10:02   🟢 ONLINE      WorldServer UpdateTime: 141ms [Cached Median]  ← docker attach failed
10:03   🟢 ONLINE      WorldServer UpdateTime: 142ms [Median]         ← recovered
```

Notice in Scenario 3: Even if docker attach fails temporarily, the server is still ONLINE, so cached performance data can be shown. The key is that when the _server itself_ is not ONLINE, we clear everything.

## Status Hierarchy

**When performance data is collected:**

- ✅ ServerStatus::ONLINE

**When performance data is cleared:**

- ❌ ServerStatus::OFFLINE
- ❌ ServerStatus::RESTARTING
- ❌ ServerStatus::ERROR

## Cache Behavior

### Normal Cache (Server ONLINE, docker attach fails):

- Server is ONLINE
- Docker attach or parsing fails temporarily
- Shows: `WorldServer UpdateTime: 141ms [Cached Median]`
- Keeps last known good data

### No Cache (Server NOT ONLINE):

- Server is OFFLINE/RESTARTING/ERROR
- All performance data cleared
- Shows: (nothing - line is hidden)
- No cached data displayed

## Build Info

**Binary:** `/home/havoc/bottop/build/bottop` (2.2MB)  
**Build Date:** Dec 14, 2025, 15:34 EST  
**Status:** ✅ Compiled successfully  
**Change:** Performance data cleared when server is not ONLINE

## Testing

To verify the fix:

1. **Normal operation:**

    ```bash
    ./build/bottop
    # Server ONLINE → WorldServer UpdateTime line should be visible
    ```

2. **Simulate server offline:**

    ```bash
    # Stop the worldserver container
    docker stop worldserver

    # Watch bottop - WorldServer UpdateTime line should disappear
    # Status should show OFFLINE then RESTARTING
    ```

3. **Restart server:**

    ```bash
    docker start worldserver

    # WorldServer UpdateTime line should reappear when ONLINE
    ```

## Related Changes

This fix complements:

1. **CACHED_DATA_INDICATOR.md** - Cache status indicator
2. **CONSOLIDATED_PERFORMANCE_LINE.md** - Single-line display format
3. **PERFORMANCE_CACHING.md** - 1-second caching system

---

**Fixed!** WorldServer performance data is now only collected and displayed when the server status is ONLINE. 🎉
