# WorldServer Performance Monitoring

This document describes the real-time WorldServer performance monitoring feature using the `server info` console command.

## Overview

Bottop now collects **real WorldServer tick performance metrics** directly from the server using the `server info` console command. This provides much more accurate performance data than timing database queries over SSH.

---

## What Changed

### Before

- **"Database Query Time"** displayed SSH + Docker + MySQL round-trip time (50-300ms)
- Not a true measure of WorldServer performance
- Only showed one value (average query time)
- Graph tracked query latency, not server performance

### After

- **"WorldServer Performance"** displays actual server tick metrics
- Data comes from WorldServer's internal performance tracking
- Shows comprehensive statistics:
    - **Current** - Current update time diff
    - **Mean** - Average of last 500 ticks
    - **Median** - Median of last 500 ticks
    - **P95** - 95th percentile
    - **P99** - 99th percentile
    - **Max** - Maximum tick time
- Graph tracks mean tick time over last 5 minutes
- Falls back to "Database Query Time" if `server info` unavailable

---

## How It Works

### Data Collection

Every update interval (default 5 seconds), Bottop executes:

```bash
docker exec testing-ac-worldserver worldserver-console 'server info'
```

This returns output like:

```
AC> AzerothCore rev. ece1060fa05d+ 2025-12-12 19:37:01 +0000 (Testing-Playerbot branch) (Unix, RelWithDebInfo, Static)
Connected players: 1. Characters in world: 3063.
Connection peak: 1.
Server uptime: 8 hour(s) 56 minute(s) 25 second(s)
Update time diff: 41ms. Last 500 diffs summary:
|- Mean: 120ms
|- Median: 106ms
|- Percentiles (95, 99, max): 243ms, 278ms, 314ms
AC>
```

### Parsing

The parser extracts these metrics:

- **update_time_diff**: Current tick time (41ms in example)
- **mean**: Average of last 500 ticks (120ms)
- **median**: Median of last 500 ticks (106ms)
- **p95**: 95th percentile (243ms)
- **p99**: 99th percentile (278ms)
- **max**: Maximum tick time (314ms)

### Display

**Performance Pane shows:**

```
WorldServer Performance
  Current: 41ms    (color-coded: green <50, light green <100, yellow <150, red >=150)
  Mean: 120ms  Median: 106ms
  P95: 243ms  P99: 278ms  Max: 314ms
```

**Graph shows:**

- Mean tick time over last 5 minutes (300 samples)
- Fixed 300ms ceiling for consistent scale
- Color gradient from green (good) to red (poor)

---

## Implementation Details

### Data Structures

**Added to `btop_azerothcore.hpp`:**

```cpp
//* WorldServer performance metrics from "server info" command
struct ServerPerformance {
    long long update_time_diff = 0;    // Current update time diff in ms
    long long mean = 0;                // Mean of last 500 diffs in ms
    long long median = 0;              // Median of last 500 diffs in ms
    long long p95 = 0;                 // 95th percentile in ms
    long long p99 = 0;                 // 99th percentile in ms
    long long max = 0;                 // Maximum in ms
    bool available = false;            // Whether server info data is available
};

//* Bot statistics and server performance
struct BotStats {
    // ... existing fields ...
    ServerPerformance perf;         // Real server performance metrics
};
```

### Fetching Performance Data

**Added to `Query` class:**

```cpp
ServerPerformance fetch_server_performance();  // Fetch from "server info"
```

**Implementation** (`btop_azerothcore.cpp` lines 382-546):

- Executes `worldserver-console 'server info'` via docker exec
- Parses output line by line using string operations
- Extracts all 6 metrics
- Returns `ServerPerformance` struct
- Sets `available = true` if at least one metric parsed successfully

### Display Logic

**Updated** (`btop_draw.cpp` lines 1634-1685):

- Checks if `data.stats.perf.available`
- If available: Shows "WorldServer Performance" with all 6 metrics
- If unavailable: Falls back to "Database Query Time"
- Uses color coding for current tick time:
    - Green: < 50ms (excellent)
    - Light green: < 100ms (good)
    - Yellow: < 150ms (acceptable)
    - Red: >= 150ms (poor)

### Graph Tracking

**Updated** (`btop_azerothcore.cpp` lines 1007-1015):

- Prefers `perf.mean` for graph history
- Falls back to query time if server info unavailable
- Maintains 300-sample history (~5 minutes at 1-second intervals)

---

## Benefits

### Accuracy

- **True server performance** - Direct from WorldServer's internal tracking
- **No network overhead** - Not affected by SSH/Docker latency
- **Comprehensive metrics** - 6 different performance indicators

### Visibility

- **Current vs. Average** - See if server is currently struggling vs. average load
- **Percentiles** - Understand performance distribution (P95, P99)
- **Max spike detection** - Identify worst-case lag spikes

### Diagnostics

- **Identify performance issues** - Spot degradation trends
- **Capacity planning** - Know when server is reaching limits
- **Bot load correlation** - Compare bot count to server performance

---

## Understanding the Metrics

### Update Time Diff (Current)

- **What**: Time taken for the most recent server tick
- **Good**: < 50ms
- **Acceptable**: 50-100ms
- **Concerning**: 100-150ms
- **Poor**: > 150ms
- **Note**: Can spike briefly, watch for sustained high values

### Mean

- **What**: Average tick time over last 500 ticks
- **Good**: < 100ms
- **Acceptable**: 100-150ms
- **Concerning**: 150-200ms
- **Poor**: > 200ms
- **Use**: Best indicator of overall server health

### Median

- **What**: Middle value of last 500 ticks (50th percentile)
- **Use**: More resistant to outliers than mean
- **Compare**: If median << mean, server has occasional spikes

### P95 (95th Percentile)

- **What**: 95% of ticks are faster than this value
- **Good**: < 200ms
- **Acceptable**: 200-250ms
- **Poor**: > 250ms
- **Use**: Represents "typical worst case" lag players experience

### P99 (99th Percentile)

- **What**: 99% of ticks are faster than this value
- **Good**: < 250ms
- **Acceptable**: 250-300ms
- **Poor**: > 300ms
- **Use**: Occasional lag spikes, rare but noticeable

### Max

- **What**: Worst tick in last 500
- **Good**: < 300ms
- **Acceptable**: 300-500ms
- **Poor**: > 500ms
- **Use**: Identify severe lag spikes (freezes)

---

## Example Scenarios

### Healthy Server

```
WorldServer Performance
  Current: 41ms     (green)
  Mean: 85ms  Median: 80ms
  P95: 150ms  P99: 180ms  Max: 220ms
```

**Analysis:**

- Current and mean both low - server running smoothly
- Median close to mean - consistent performance
- P95 and P99 reasonable - occasional minor spikes
- Max under control - no severe freezes

### Server Under Load

```
WorldServer Performance
  Current: 135ms    (yellow)
  Mean: 120ms  Median: 106ms
  P95: 243ms  P99: 278ms  Max: 314ms
```

**Analysis:**

- Mean creeping up - server experiencing sustained load
- Median lower than mean - some ticks much slower
- P95 approaching 250ms - 5% of players see lag
- Max over 300ms - occasional brief freezes

### Overloaded Server

```
WorldServer Performance
  Current: 187ms    (red)
  Mean: 165ms  Median: 155ms
  P95: 285ms  P99: 320ms  Max: 450ms
```

**Analysis:**

- Current high and red - server struggling right now
- Mean and median both high - consistently slow
- P95/P99 very high - frequent lag for many players
- Max 450ms - regular severe freezes
- **Action needed**: Reduce bot count or optimize server

---

## Troubleshooting

### "Database Query Time" Shown Instead of "WorldServer Performance"

**Possible causes:**

1. **worldserver-console not available**
    - Check if command exists: `docker exec worldserver which worldserver-console`
    - Some AzerothCore builds don't include console tool

2. **Docker exec failing**
    - Test manually: `docker exec testing-ac-worldserver worldserver-console 'server info'`
    - Check container name in config matches actual container

3. **Server not responding to console**
    - Server might be frozen or crashed
    - Check server logs: `docker logs worldserver`

4. **Parsing failed**
    - Different AzerothCore version might have different output format
    - Check debug logs for parse errors
    - Create GitHub issue with your `server info` output

### Metrics Show 0 or Wrong Values

**Check:**

- Look at debug logs: `Logger::debug` messages show parsed values
- Manually run `server info` and compare output format
- Different AC builds might format output differently

### High Performance Numbers

**Remember:**

- 3000+ bots is a HUGE load
- Mean of 120ms is actually reasonable for that many bots
- Real players typically see much better performance
- Consider your server hardware specs

---

## Files Modified

1. **`src/btop_azerothcore.hpp`** (lines 344-363)
    - Added `ServerPerformance` struct
    - Added `perf` member to `BotStats`
    - Added `fetch_server_performance()` declaration

2. **`src/btop_azerothcore.cpp`** (lines 376-546)
    - Implemented `fetch_server_performance()` method
    - Added console command execution
    - Implemented parsing logic
    - Updated `fetch_bot_stats()` to call `fetch_server_performance()`
    - Updated graph history to use mean tick time

3. **`src/btop_draw.cpp`** (lines 1634-1685)
    - Updated performance pane display
    - Shows WorldServer Performance when available
    - Falls back to Database Query Time when not available
    - Added color coding for current tick time

---

## Future Enhancements

### Potential Improvements

1. **Alert Thresholds**
    - Visual/audio alerts when metrics exceed thresholds
    - Configurable warning levels

2. **Performance Trends**
    - Track mean/p95/p99 over longer periods (hours/days)
    - Detect degradation trends

3. **Correlation Analysis**
    - Overlay bot count on performance graph
    - Show which zones contribute most to lag

4. **Historical Data**
    - Log performance metrics to file
    - Generate daily/weekly performance reports

5. **Multiple Servers**
    - Compare performance across multiple servers
    - Aggregate metrics for cluster view

---

## Testing

```bash
# Build
cd /home/havoc/bottop/build
make -j4

# Run
./bottop

# What to look for:
# 1. "WorldServer Performance" title (not "Database Query Time")
# 2. All 6 metrics displayed with reasonable values
# 3. Current value color-coded (green/yellow/red)
# 4. Graph showing historical mean tick time
```

**Manual testing:**

```bash
# Test server info command manually
docker exec testing-ac-worldserver worldserver-console 'server info'

# Should output something like:
# AC> AzerothCore rev. ...
# Update time diff: 41ms. Last 500 diffs summary:
# |- Mean: 120ms
# |- Median: 106ms
# |- Percentiles (95, 99, max): 243ms, 278ms, 314ms
```

---

## Conclusion

The WorldServer Performance feature provides **accurate, real-time insight into server health**. By monitoring all 6 metrics, you can:

- **Diagnose lag issues** quickly
- **Optimize bot distribution** to reduce load
- **Plan capacity** for bot growth
- **Identify performance regressions** after config changes

This is a significant improvement over the previous "Database Query Time" metric, which measured network latency rather than actual server performance.
