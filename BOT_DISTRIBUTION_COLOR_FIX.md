# Bot Distribution Color Consistency Fix

## Issue

In the bot distribution pane, some level brackets (1-9, 50-59, 60, 70, 71-79) were displayed in a slightly lighter grey than others, creating visual inconsistency.

## Root Cause

The code in `btop_draw.cpp:1825-1831` was implementing **deviation-based coloring**:

- Brackets that deviated >3% from expected distribution → `Theme::c("graph_text")` (lighter warning color)
- Brackets within tolerance → `Theme::c("inactive_fg")` (normal grey)

This was intentionally highlighting brackets that weren't matching the expected bot distribution configured on the server, but it created visual inconsistency.

### Why Those Specific Brackets?

The server's expected distribution (from `mod_player_bot_level_brackets.conf`):

- 1-9: 5% expected
- 50-59: 6% expected
- 60: 12% expected
- 70: 10% expected
- 71-79: 16% expected

These brackets were showing in lighter grey because the actual bot distribution deviated more than 3% from these expected values.

## Solution

Removed the deviation-based coloring logic in `btop_draw.cpp:1825-1830` and replaced it with consistent coloring:

```cpp
// Use consistent color for all brackets
string color = Theme::c("inactive_fg");
```

**Before:**

```cpp
// Color based on deviation from expected (±3% tolerance)
string color;
if (expected_percent > 0 and abs(percent - expected_percent) > 3.0) {
    color = Theme::c("graph_text");  // Warning color for deviation
} else {
    color = Theme::c("inactive_fg");  // Normal color
}
```

**After:**

```cpp
// Use consistent color for all brackets
string color = Theme::c("inactive_fg");
```

## Result

✅ All level brackets now display in consistent grey color
✅ No visual distraction from color variations
✅ Cleaner, more uniform appearance

## Trade-off Note

The previous behavior was actually a feature that highlighted distribution issues. If you wanted to monitor whether bots were being created according to the configured level distribution, the color coding provided a quick visual indicator.

However, for a cleaner UI without monitoring alerts, consistent coloring is better.

## Files Modified

- **`src/btop_draw.cpp`** (lines 1825-1830) - Removed deviation-based coloring

## Build Status

✅ Successfully compiled (Dec 15, 2025)

- Build time: 1m 12s
- Binary: `/home/havoc/bottop/bin/bottop`
