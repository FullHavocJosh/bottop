# Display Mean Instead of Median - COMPLETE ✅

**Date:** December 14, 2025, 15:40 EST  
**Status:** Built and Ready

## Overview

Changed the WorldServer UpdateTime display and graph from using Median to using Mean (average) of the last 500 update cycles.

## The Change

### Before:

```
WorldServer UpdateTime: 141ms [Median]
```

Graph showed: Median values

### After:

```
WorldServer UpdateTime: 145ms [Mean]
```

Graph shows: Mean (average) values

## Why Mean?

**Mean (Average):**

- Average of all 500 update cycles
- Shows overall average performance
- More familiar metric for most users
- Includes all data points in calculation

**Median (Middle value):**

- Middle value when sorted
- More resistant to outliers
- Can hide occasional spikes/dips

## Code Changes

### File: `src/btop_draw.cpp` (lines 1635-1655)

**Display logic:**

Before:

```cpp
// Median update time with color coding
if (data.stats.perf.median < 50) {
    diff_color = Theme::c("proc_misc");
} else if (data.stats.perf.median < 100) {
    diff_color = "\x1b[92m";
} // ... etc

string metric_type = data.stats.perf.is_cached ? "[Cached Median]" : "[Median]";
out += ... + to_string(data.stats.perf.median) + "ms " + metric_type;
```

After:

```cpp
// Mean update time with color coding
if (data.stats.perf.mean < 50) {
    diff_color = Theme::c("proc_misc");
} else if (data.stats.perf.mean < 100) {
    diff_color = "\x1b[92m";
} // ... etc

string metric_type = data.stats.perf.is_cached ? "[Cached Mean]" : "[Mean]";
out += ... + to_string(data.stats.perf.mean) + "ms " + metric_type;
```

### File: `src/btop_azerothcore.cpp` (lines 1226-1232)

**Graph data source:**

Before:

```cpp
// Track median server update time (more stable than current, resistant to spikes)
if (current_data.stats.perf.available) {
    load_history.push_back(current_data.stats.perf.median);
}
```

After:

```cpp
// Track mean server update time (average of last 500 cycles)
if (current_data.stats.perf.available) {
    load_history.push_back(current_data.stats.perf.mean);
}
```

## Display Format

**Format:** `WorldServer UpdateTime: <VALUE>ms <TYPE>`

**Types:**

- `[Mean]` - Fresh data (average of last 500 cycles from WorldServer)
- `[Cached Mean]` - Last known good data (docker attach/parsing failed)

**Color Coding (unchanged):**

- 🟢 Green: < 50ms (excellent)
- 🟢 Light green: 50-99ms (good)
- 🟡 Yellow: 100-149ms (acceptable)
- 🔴 Red: ≥ 150ms (poor)

## Visual Examples

### Fresh Data (Normal):

```
Server Status: 🟢 ONLINE
Server Uptime: 2h
WorldServer UpdateTime: 145ms [Mean]
<graph>
```

### Cached Data:

```
Server Status: 🟢 ONLINE
Server Uptime: 2h
WorldServer UpdateTime: 145ms [Cached Mean]
<graph>
```

### Server Offline:

```
Server Status: 🔴 OFFLINE
Server Uptime: 2h
<graph>
```

_(WorldServer UpdateTime line is hidden)_

## Data Source

All performance metrics come from the WorldServer `server info` command:

```
Update time diff: 157 (min: 30 max: 243)
Average update time: 141
Median update time: 139
95-th percentile update time: 155
99-th percentile update time: 165
Max update time: 243
```

**What we display:** The "Average update time" (mean) value

**What we used before:** The "Median update time" value

## Performance Metrics Available

From `server info`, these metrics are parsed:

| Metric           | Description                | Now Used?                        |
| ---------------- | -------------------------- | -------------------------------- |
| update_time_diff | Current single cycle       | ❌ No                            |
| **mean**         | Average of 500 cycles      | ✅ **Yes** (displayed & graphed) |
| median           | Middle value of 500 cycles | ❌ No                            |
| p95              | 95th percentile            | ❌ No                            |
| p99              | 99th percentile            | ❌ No                            |
| max              | Maximum in 500 cycles      | ❌ No                            |

## Graph Behavior

The performance graph now shows:

- **Y-axis:** Mean update time in milliseconds (0-300ms range)
- **X-axis:** Time (last 300 samples)
- **Data:** Historical mean values collected every 1 second

## Comparison: Mean vs Median

**Example update times (last 500 cycles):**

```
Most values: 140-145ms
Occasional spikes: 200-250ms
```

**Mean:** 148ms (includes spikes in calculation, pulls average up)  
**Median:** 142ms (middle value, spikes don't affect it)

**Result:** Mean will typically be slightly higher than median when there are occasional spikes, giving a more complete picture of overall performance.

## Build Info

**Binary:** `/home/havoc/bottop/build/bottop` (2.2MB)  
**Build Date:** Dec 14, 2025, 15:40 EST  
**Status:** ✅ Compiled successfully  
**Changes:**

- Display changed from median to mean
- Graph changed from median to mean
- Labels updated to `[Mean]` / `[Cached Mean]`

## Testing

To verify:

```bash
./build/bottop

# Check that the display shows:
# "WorldServer UpdateTime: XXXms [Mean]"
# (not "[Median]")
```

## Related Changes

This builds on:

1. **CONSOLIDATED_PERFORMANCE_LINE.md** - Single-line format
2. **CACHED_DATA_INDICATOR.md** - Cache status indicator
3. **PERFORMANCE_ONLY_WHEN_ONLINE.md** - Only show when ONLINE
4. **MEDIAN_AND_10S_INTERVAL.md** - Originally changed to median (now mean)

---

**Updated!** WorldServer UpdateTime now displays and graphs the Mean (average) instead of Median. 🎉
