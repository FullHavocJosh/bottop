# Collection Interval & Median Performance Display - COMPLETE ✅

**Date:** December 13, 2025, 22:49 EST  
**Status:** Built and Ready

## Changes Made

### 1. Collection Interval: 2s → 10s

Changed the default update interval from 2 seconds to 10 seconds for all data collection (zones, bots, factions, etc.).

**Why 10 seconds?**

- Reduces SSH connection overhead
- Less load on database queries
- Data doesn't change that rapidly anyway
- Still responsive enough for monitoring

**Note:** WorldServer performance metrics still update every 1 second (independent timing from the caching system).

### 2. Performance Metric: Current → Median

Changed the displayed and graphed performance metric from "Current" (single update time) to "Median" (middle value of last 500 cycles).

**Why Median over Current?**

- ✅ **More stable** - Not affected by single spikes
- ✅ **More representative** - Shows typical performance, not momentary
- ✅ **Resistant to outliers** - One bad tick doesn't skew the display
- ✅ **Better for monitoring** - Shows sustained performance level

**Why Median over Mean (Average)?**

- Median is the middle value when sorted
- Mean can be pulled up/down by extreme outliers
- Median shows "typical" performance more accurately

**Example:**

```
Update times: [100, 105, 110, 115, 500]
Mean:    186ms (pulled up by the 500ms outlier)
Median:  110ms (the actual middle value - more representative)
```

## Code Changes

### File: `src/btop_config.cpp` (line 291)

**Changed default update interval:**

```cpp
// Before:
{"update_ms", 2000},  // 2 seconds

// After:
{"update_ms", 10000},  // 10 seconds
```

### File: `src/btop_azerothcore.cpp` (lines 1219-1225)

**Changed graph data source:**

```cpp
// Before:
// Track current server update time
if (current_data.stats.perf.available) {
    load_history.push_back(current_data.stats.perf.update_time_diff);
}

// After:
// Track median server update time (more stable, resistant to spikes)
if (current_data.stats.perf.available) {
    load_history.push_back(current_data.stats.perf.median);
}
```

### File: `src/btop_draw.cpp` (lines 1635-1652)

**Changed display metric and label:**

```cpp
// Before:
string diff_color;
if (data.stats.perf.update_time_diff < 50) {
    diff_color = Theme::c("proc_misc");  // Green
} // ... color coding

out += "Update Time: " + diff_color + to_string(data.stats.perf.update_time_diff) + "ms";

// After:
string diff_color;
if (data.stats.perf.median < 50) {
    diff_color = Theme::c("proc_misc");  // Green
} // ... color coding

out += "Update Time (Median): " + diff_color + to_string(data.stats.perf.median) + "ms";
```

## What This Means

### Performance Display Now Shows:

**Label:** "Update Time (Median): XXms"

**Value:** Median of the last 500 update cycles from WorldServer

- Represents typical sustained performance
- Not affected by momentary spikes or dips
- More stable and reliable indicator

**Graph:** Historical median values over time

- Smoother line (less spiky)
- Shows sustained performance trends
- Easier to spot actual performance degradation vs. temporary blips

### Collection Behavior:

**General Data (zones, bots, factions):** Every 10 seconds

- Reduced database load
- Less SSH overhead
- Still plenty responsive

**Performance Metrics:** Every 1 second (unchanged)

- Fast updates for real-time monitoring
- Uses caching to avoid overhead
- Independent of general collection interval

## Visual Comparison

### Before (Current):

```
Update Time: 157ms  (single tick, could be spike)
Graph: Spiky ↗↘↗↘  (volatile)
```

### After (Median):

```
Update Time (Median): 141ms  (typical of 500 ticks)
Graph: Smooth ~~~  (stable trend)
```

## Performance Metrics Context

For reference, the server collects these from `server info`:

| Metric      | Description           | Use Case                                           |
| ----------- | --------------------- | -------------------------------------------------- |
| **Current** | Latest single update  | Real-time instant value                            |
| **Mean**    | Average of 500 cycles | Overall average (affected by outliers)             |
| **Median**  | Middle of 500 cycles  | **Typical sustained performance** ← Now using this |
| **P95**     | 95th percentile       | 5% of cycles were slower                           |
| **P99**     | 99th percentile       | 1% of cycles were slower                           |
| **Max**     | Worst in 500 cycles   | Worst case scenario                                |

## Benefits

### Median Display:

✅ **Stability** - Less visual noise from spikes  
✅ **Accuracy** - Represents typical performance  
✅ **Reliability** - Not fooled by outliers  
✅ **Monitoring** - Easier to spot real issues

### 10-Second Collection:

✅ **Efficiency** - Less database queries  
✅ **Lower overhead** - Fewer SSH connections  
✅ **Adequate** - Data doesn't change that fast  
✅ **Scalability** - Better for long-running sessions

## User Configuration

If you want to change the collection interval back, edit your config file:

**Location:** `~/.config/bottop/bottop.conf`

```ini
# Change this value (in milliseconds)
update_ms=10000

# Examples:
# update_ms=2000   # 2 seconds (old default)
# update_ms=5000   # 5 seconds
# update_ms=10000  # 10 seconds (new default)
# update_ms=30000  # 30 seconds
```

Or use the command line:

```bash
./bottop --update 5000  # 5 seconds
```

## Build Info

**Binary:** `/home/havoc/bottop/build/bottop` (2.2MB)  
**Timestamp:** Dec 13, 2025, 22:49 EST  
**Status:** ✅ Ready for testing

## Testing Checklist

- ✅ Code compiles without errors
- ✅ Default update interval changed to 10s
- ✅ Performance display uses median
- ✅ Graph uses median values
- ✅ Label updated to "Update Time (Median)"
- ⏳ Visual testing in TTY (requires manual run)

---

**Ready to use!** The performance display is now more stable and meaningful, showing median (typical) performance instead of momentary values. Collection runs every 10 seconds to reduce overhead. 🎉
