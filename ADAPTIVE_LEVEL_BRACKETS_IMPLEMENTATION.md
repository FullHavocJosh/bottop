# 3-Tier Color System for Level Distribution Monitoring

## Feature Implementation

Implemented a color-coded system for level bracket distribution that provides at-a-glance monitoring of how well actual bot distribution matches the expected configuration.

## Color System

The entire bracket line (name + percentage) is colored based on deviation from expected values:

### Tier 1: Green (On Target)

**Deviation: ±0-3%**

- Color: `Theme::c("available")` (Green)
- Meaning: Distribution is within acceptable range
- Example: Expected 10%, Actual 8-13% → Green

### Tier 2: Yellow (Warning)

**Deviation: ±3-6%**

- Color: `Theme::c("graph_text")` (Yellow/Orange)
- Meaning: Noticeable deviation, should be monitored
- Example: Expected 10%, Actual 5-8% or 13-16% → Yellow

### Tier 3: Red (Critical)

**Deviation: ±6-9%**

- Color: `Theme::c("title")` (Red/Pink)
- Meaning: Significant deviation, action recommended
- Example: Expected 10%, Actual 2-5% or 16-19% → Red

### Tier 4: Bright/White (Severe)

**Deviation: >±9%**

- Color: `Theme::c("hi_fg")` (Bright/White)
- Meaning: Severe deviation, requires immediate attention
- Example: Expected 10%, Actual <2% or >19% → Bright

## Display Example

```
Levels:
1-9            5%   (Green - on target)
10-19          9%   (Yellow - slight deviation)
20-29         17%   (Red - significant deviation)
30-39          2%   (Bright - severe deviation)
40-49          8%   (Green - on target)
...
```

## Why You Might Not See Colors

If all brackets appear in default grey, it could be because:

1. **Expected values not loaded yet**
    - The expected distribution config needs to load first
    - Check `/tmp/bottop_init_debug.log` to verify brackets were loaded
    - Should show: "Final bracket count: 11"

2. **All brackets within ±3% tolerance**
    - If bot distribution is nearly perfect, all show green
    - Green may look like grey depending on terminal theme

3. **Expected percentages not in config**
    - If server config doesn't have percentage values
    - Code falls back to default grey

## Interpretation Guide

### Healthy Distribution

- Most brackets: Green or Yellow
- Few: Red
- None: Bright

### Bot Creation Issues

- Many low-level brackets: Red/Bright
- High-level brackets: Green
- → Not enough new bot creation

### Leveling Issues

- Low-level brackets: Green
- High-level brackets: Red/Bright
- → Bots leveling up too fast or max bot count too low

### Configuration Mismatch

- Random mix of all colors across ranges
- → Expected percentages don't match current bot behavior
- → Consider adjusting mod_player_bot_level_brackets.conf

## Implementation Details

**File:** `src/btop_draw.cpp` (lines 1811-1851)

**Key Logic:**

```cpp
double deviation = abs(actual_percent - expected_percent);

if (deviation <= 3.0)      → Green  (on target)
else if (deviation <= 6.0) → Yellow (warning)
else if (deviation <= 9.0) → Red    (critical)
else                       → Bright (severe)
```

**Color Choices:**

- Green: `available` - Typically green in most themes
- Yellow: `graph_text` - Graph warning color (yellow/orange)
- Red: `title` - Title color (often red/pink in dark themes)
- Bright: `hi_fg` - High-contrast foreground (white/bright)

## Advantages Over Arrows

✅ **More visible** - Entire line colored vs small arrow  
✅ **4 levels of severity** - More granular than binary up/down  
✅ **Absolute deviation** - Direction doesn't matter, only magnitude  
✅ **Universal understanding** - Green=good, Red=bad is intuitive  
✅ **Better for quick scanning** - Eye catches colors faster

## Testing Colors

To verify colors are working:

1. Check that expected values loaded:

    ```bash
    cat /tmp/bottop_init_debug.log | grep "Final bracket count"
    # Should show: Final bracket count: 11
    ```

2. Check actual vs expected percentages in debug log:

    ```bash
    cat /tmp/bottop_init_debug.log | grep "Bracket"
    # Should show all 11 brackets with their expected %
    ```

3. Compare with actual distribution in bottop:
    - If actual% differs significantly from expected%, you'll see colors
    - If all within 3%, all will be green (which may look greyish)

## Files Modified

- **`src/btop_draw.cpp`** (lines 1811-1851) - Implemented 3-tier color system

## Build Status

✅ Successfully compiled (Dec 15, 2025)

- Build time: 1m 9s
- Binary: `/home/havoc/bottop/bin/bottop`
