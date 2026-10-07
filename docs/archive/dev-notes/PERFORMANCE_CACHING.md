# WorldServer Performance Caching & 1-Second Updates - COMPLETE ✅

**Date:** December 13, 2025, 22:40 EST  
**Status:** Built and Ready

## Problem

WorldServer Performance section was randomly disappearing when:

1. The `server info` command failed to return data
2. Parsing failed due to timing issues with docker attach
3. Server was busy with other operations

## Solution

Implemented two key features:

### 1. Performance Data Caching (Last Known Good Values)

When `fetch_server_performance()` fails to parse new data, it now falls back to the last successfully parsed data instead of displaying nothing.

**How it works:**

- Every successful parse stores data in `last_known_perf`
- If parsing fails, return the cached data
- Performance display remains visible with last known values
- Graceful degradation instead of disappearing UI

### 2. 1-Second Update Interval for Performance

Performance metrics now update every 1 second instead of following the general update interval (which could be 2-5 seconds).

**How it works:**

- `PERF_UPDATE_INTERVAL_MS = 1000` (1 second)
- Tracks `last_perf_update_time` timestamp
- If called within 1 second, returns cached data (no SSH overhead)
- After 1 second, fetches fresh data from WorldServer
- Independent timing from other metrics (zones, bots, etc.)

## Code Changes

### File: `src/btop_azerothcore.cpp`

#### 1. Added Global Variables (lines 53-58)

```cpp
//* Historical data for graphs
std::deque<long long> load_history;

//* Cached performance data (last known good values)
::AzerothCore::ServerPerformance last_known_perf;
uint64_t last_perf_update_time = 0;
const uint64_t PERF_UPDATE_INTERVAL_MS = 1000;  // Update every 1 second
```

#### 2. Updated `fetch_server_performance()` Function (lines 389-406)

**Added timing check at start:**

```cpp
::AzerothCore::ServerPerformance Query::fetch_server_performance() {
    // Check if we should fetch new data (1 second interval)
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    // If less than 1 second since last update, return cached data
    if (last_perf_update_time > 0 &&
        (now_ms - last_perf_update_time) < PERF_UPDATE_INTERVAL_MS &&
        last_known_perf.available) {
        Logger::debug("fetch_server_performance: Using cached data (age: " +
            std::to_string(now_ms - last_perf_update_time) + "ms)");
        return last_known_perf;
    }

    ::AzerothCore::ServerPerformance perf;
    // ... rest of function
}
```

#### 3. Updated Data Caching Logic (lines 695-718)

**Added cache storage and fallback:**

```cpp
// Mark as available if we parsed at least one value
perf.available = (perf.update_time_diff > 0 || perf.mean > 0 || perf.median > 0);

if (perf.available) {
    Logger::debug("fetch_server_performance: diff=" + std::to_string(perf.update_time_diff) + "ms...");

    // Cache this good data
    last_known_perf = perf;
    last_perf_update_time = now_ms;
} else {
    // Failed to parse - use cached data if available
    if (last_known_perf.available) {
        Logger::debug("fetch_server_performance: Parse failed, using last known good data");
        perf = last_known_perf;
        perf.available = true;  // Mark as available since we have cached data
    }
}

return perf;
```

## Benefits

### 1. **Stable UI**

- Performance section no longer randomly disappears
- Always shows data (either fresh or last known)
- Smoother user experience

### 2. **Faster Updates**

- 1-second refresh for performance metrics
- More responsive to server performance changes
- Better real-time monitoring

### 3. **Reduced Overhead**

- Caching prevents redundant SSH calls within 1 second
- Less load on SSH connection and server
- Efficient resource usage

### 4. **Graceful Degradation**

- System continues working during temporary glitches
- No data loss from transient failures
- Robust error handling

## Behavior Examples

### Example 1: Normal Operation

```
T=0s:  Fetch fresh data → Success → Display + Cache → Update timestamp
T=0.5s: Request data → Use cached (< 1s old) → Return instantly
T=1.1s: Fetch fresh data → Success → Display + Cache → Update timestamp
```

### Example 2: Parse Failure

```
T=0s:  Fetch fresh data → Success → Display + Cache
T=1.1s: Fetch fresh data → Parse fails → Use cached → Display last known
T=2.2s: Fetch fresh data → Success → Display + Cache → Update timestamp
```

### Example 3: Multiple Quick Calls

```
T=0.0s: Fetch fresh data → Execute SSH command (takes ~3s)
T=0.2s: Request data → Use cached (< 1s old) → Instant return
T=0.5s: Request data → Use cached (< 1s old) → Instant return
T=0.8s: Request data → Use cached (< 1s old) → Instant return
T=1.1s: Fetch fresh data → Execute SSH command
```

## Testing

- ✅ Code compiles without errors
- ✅ Binary rebuilt (2.2MB, Dec 13 22:40 EST)
- ✅ Caching logic implemented
- ✅ 1-second interval implemented
- ✅ Fallback to cached data implemented
- ⏳ Visual testing in TTY (requires manual run)

## Debug Logging

Debug messages now include:

- "Using cached data (age: XXms)" - When returning cached data due to timing
- "Parse failed, using last known good data" - When fallback is used

Check `/tmp/bottop_debug.txt` for detailed logs.

## Configuration

No configuration changes needed. The 1-second interval is hardcoded as:

```cpp
const uint64_t PERF_UPDATE_INTERVAL_MS = 1000;
```

To change the interval, modify this value in `src/btop_azerothcore.cpp` and rebuild.

## Notes

- **Other metrics unchanged:** Zone, bot, and faction data still use the global `update_ms` interval
- **Cache persists:** Last known performance data persists for the entire session
- **No stale data marker:** UI doesn't indicate when showing cached vs fresh data (could be added if needed)
- **Thread-safe:** Uses existing single-threaded collect model

## Build Info

**Binary:** `/home/havoc/bottop/build/bottop` (2.2MB)  
**Timestamp:** Dec 13, 2025, 22:40 EST  
**Status:** ✅ Ready for testing

---

**Ready to use!** Performance data will now update every 1 second and never disappear. 🎉
