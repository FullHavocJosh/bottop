# Performance Pane Simplification - COMPLETE ✅

**Date:** December 13, 2025, 22:26 EST  
**Status:** Built and Ready

## Changes Made

### Removed Items

1. **Build Information** (all removed)
    - ~~Rev: Git commit hash~~
    - ~~Branch: Git branch name~~
    - ~~Build Type: Debug/Release info~~

2. **Player Statistics** (all removed)
    - ~~Players: Connected players count~~
    - ~~Characters: Total characters in world~~
    - ~~Peak: Connection peak~~

3. **Server Uptime (WorldServer specific)** (removed)
    - ~~Uptime from server info~~
    - Note: Container uptime still displayed at top

4. **Statistical Metrics** (all removed)
    - ~~Mean: Average over 500 cycles~~
    - ~~Median: Middle value of 500 cycles~~
    - ~~P95: 95th percentile~~
    - ~~P99: 99th percentile~~
    - ~~Max: Maximum in 500 cycles~~

5. **Database Query Time Fallback** (completely removed)
    - ~~Fallback display when server info unavailable~~

### What Remains

The Performance pane now shows:

1. **Server URL** - Connection endpoint
2. **Server Status** - Online/Offline/Restarting (color-coded)
3. **Server Uptime** - Container uptime in human-readable format
4. **WorldServer Performance** section with:
    - **Update Time: XXms** (color-coded)
        - 🟢 Green: < 50ms (excellent)
        - 🟢 Light green: 50-99ms (good)
        - 🟡 Yellow: 100-149ms (acceptable)
        - 🔴 Red: ≥ 150ms (poor)
5. **Update Time Graph** (braille chart)
    - Real-time visualization of current update times
    - Scale: 0-300ms
    - Historical data over last 300 data points
    - Y-axis scale labels on the left
6. **Ollama Stats** (if enabled, at bottom)
    - Queries sent/received
    - Failure rate

## Technical Details

### What the Graph Shows

The graph displays `update_time_diff` - the **current update cycle time** from WorldServer's `server info` command. This is the most real-time performance indicator available.

- **Data Source:** `data.stats.perf.update_time_diff`
- **History Buffer:** `load_history` (300 data points max)
- **Update Frequency:** Every collection cycle
- **Graph Type:** Braille vertical bars
- **Scale:** Fixed 0-300ms for consistency

### Metric Choice: "Current" Update Time

**Why "Current" is the best choice:**

- ✅ **Real-time** - Shows what's happening _right now_
- ✅ **Responsive** - Immediately reflects performance changes
- ✅ **Actionable** - Spikes are instantly visible
- ✅ **Graph-friendly** - Creates meaningful visual patterns

**Why not Mean/Median:**

- Mean/Median are smoothed over 500 cycles (~8 minutes)
- They lag behind actual performance
- They hide short-term spikes and issues
- Less useful for real-time monitoring

**Why not Max:**

- Shows only the worst case
- Doesn't reflect typical performance
- Can be misleading (one bad spike stays for 500 cycles)

## File Changes

**Modified:** `src/btop_draw.cpp` (lines 1635-1653)

**Before:**

```cpp
// Displayed: Rev, Branch, Build Type, Players, Characters, Peak, Uptime
// Displayed: Current, Mean, Median, P95, P99, Max
// Fallback: Database Query Time when server info unavailable
```

**After:**

```cpp
// Displays only: Update Time (current) when server info available
// No fallback display when unavailable
out += Mv::to(cy++, perf_x + 4) + Theme::c("graph_text") + "Update Time: "
    + diff_color + to_string(data.stats.perf.update_time_diff) + "ms";
```

**After:**

```cpp
// Displays only: Update Time (current)
out += Mv::to(cy++, perf_x + 4) + Theme::c("graph_text") + "Update Time: "
    + diff_color + to_string(data.stats.perf.update_time_diff) + "ms";
```

## Visual Layout

```
┌─────────────────────────────────────┐
│ server performance                  │
├─────────────────────────────────────┤
│                                     │
│ Server URL: testing-...             │
│ Server Status: ONLINE               │
│ Server Uptime: 18 minutes           │
│ WorldServer Performance             │
│   Update Time: 157ms                │
│                                     │
│ 300ms │                             │
│ 250ms │                             │
│ 200ms │    ⡀⡀                       │
│ 150ms │  ⢀⣸⣿⣆                      │
│ 100ms │⢰⣿⣿⣿⣿⣆                     │
│  50ms │⣿⣿⣿⣿⣿⣿⣆⡀                  │
│   0ms │⣿⣿⣿⣿⣿⣿⣿⣿⣆⡀               │
│                                     │
│ [Ollama stats if enabled]           │
└─────────────────────────────────────┘
```

## Benefits

1. **Cleaner UI** - Less clutter, more focus
2. **Faster Comprehension** - One metric, one graph
3. **More Graph Space** - Graph has more vertical room
4. **Real-time Focus** - Shows current performance, not historical averages
5. **Actionable** - Immediate feedback for performance issues

## Testing

- ✅ Code compiles without errors
- ✅ Binary rebuilt (2.2MB, Dec 13 22:26 EST)
- ✅ Removed lines confirmed in source
- ✅ "Database Query Time" completely removed
- ✅ Graph still displays update time history
- ⏳ Visual testing in TTY (requires manual run)

## Next Steps

To see the simplified performance pane, run:

```bash
./build/bottop
```

Navigate to the AzerothCore view to see the cleaner, more focused performance display.

## Rollback (if needed)

If you want to restore any of the removed metrics, they're still being parsed and available in the data structure:

- `data.stats.perf.revision`
- `data.stats.perf.branch`
- `data.stats.perf.build_type`
- `data.stats.perf.connected_players`
- `data.stats.perf.characters_in_world`
- `data.stats.perf.connection_peak`
- `data.stats.perf.uptime`
- `data.stats.perf.mean`
- `data.stats.perf.median`
- `data.stats.perf.p95`
- `data.stats.perf.p99`
- `data.stats.perf.max`

Just add display lines in `src/btop_draw.cpp` around line 1652.
