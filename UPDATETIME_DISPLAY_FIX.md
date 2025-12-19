# WorldServer UpdateTime Display Fix

## Issue
The WorldServer UpdateTime millisecond value was not being displayed in the bottop interface, even though the graph was visible and fluctuating.

## Root Cause
The display code in `btop_draw.cpp` was only showing the **Mean** value (average of last 500 update cycles) but not the **Current** update time diff value, which is what users expect to see for real-time monitoring.

## Data Available
The AzerothCore monitoring collects two key metrics from "server info":
- `perf.update_time_diff` - Current update time (most recent cycle)
- `perf.mean` - Average of last 500 update cycles

## Solution
Modified the display in `src/btop_draw.cpp` (lines ~1760-1785) to show BOTH values:

**New Format:**
```
WorldServer UpdateTime: 120ms [Mean] (Current: 45ms)
```

**Color Coding:**
Both values are color-coded independently based on performance:
- Green (< 50ms) - Excellent
- Light Green (50-100ms) - Good  
- Yellow (100-150ms) - Acceptable
- Red (>= 150ms) - Poor

## Benefits
1. **Real-time visibility**: Current update time shows instant performance
2. **Trend awareness**: Mean shows overall server health
3. **Quick diagnosis**: Can immediately spot if current spike is anomaly vs sustained issue
4. **Color coded**: Visual feedback for performance levels

## Example Display Scenarios

**Good Performance:**
```
WorldServer UpdateTime: 45ms [Mean] (Current: 42ms)
                        ^^^ green      ^^^ green
```

**Spike Detection:**
```
WorldServer UpdateTime: 50ms [Mean] (Current: 180ms)
                        ^^^ green      ^^^ red (spike!)
```

**Degraded Performance:**
```
WorldServer UpdateTime: 155ms [Mean] (Current: 162ms)
                        ^^^ red        ^^^ red (sustained)
```

**Cached Data:**
```
WorldServer UpdateTime: 120ms [Cached Mean] (Current: 115ms)
```

## Files Modified
- `src/btop_draw.cpp` - Updated UpdateTime display logic
- Backup: `src/btop_draw.cpp.backup`

## Build Status
✅ Compiled successfully
✅ Ready to test: `/root/bottop/build/bottop`

## Testing
Run bottop and look at the AzerothCore monitoring panel. The WorldServer UpdateTime line should now show:
1. Mean value (colored)
2. [Mean] or [Cached Mean] label
3. (Current: XXms) value (colored) - **THIS IS THE FIX**

The current value will update with each refresh cycle, providing real-time feedback on server performance.
