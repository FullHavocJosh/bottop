# Single-Level Bracket Display Fix

## Issue

Bot distribution wasn't showing single-level brackets (60, 70, 80) correctly.

## Root Cause

The `BracketDefinition` constructor was formatting ALL brackets as ranges with a hyphen, even when min_level == max_level.

**Before:**

- Level 60 bracket → displayed as "60-60"
- Level 70 bracket → displayed as "70-70"
- Level 80 bracket → displayed as "80-80"

## Solution

Updated the `BracketDefinition` constructor to detect single-level brackets and format them without a hyphen.

### Code Change (src/btop_azerothcore.hpp:445-453)

**Before:**

```cpp
BracketDefinition(int min, int max) : min_level(min), max_level(max) {
    range = std::to_string(min) + "-" + std::to_string(max);
}
```

**After:**

```cpp
BracketDefinition(int min, int max) : min_level(min), max_level(max) {
    if (min == max) {
        // Single level bracket: "60" not "60-60"
        range = std::to_string(min);
    } else {
        // Range bracket: "1-10"
        range = std::to_string(min) + "-" + std::to_string(max);
    }
}
```

## Impact

This fix affects both:

1. **Display** - The UI now shows "60", "70", "80" instead of "60-60", "70-70", "80-80"
2. **SQL Query** - The database queries now correctly label single-level brackets

## Expected Display (11 Brackets)

```
Levels:
  1-9         5%
  10-19       6%
  20-29      10%
  30-39       9%
  40-49       8%
  50-59       6%
  60         12%    ← Now shows correctly
  61-69       7%
  70         10%    ← Now shows correctly
  71-79      16%
  80         11%    ← Now shows correctly
```

## Verification

Run bottop and check the "Levels:" section in the Distribution pane. You should see:

- ✅ "60" (not "60-60")
- ✅ "70" (not "70-70")
- ✅ "80" (not "80-80")
- ✅ All other ranges displayed normally (e.g., "1-9", "10-19", etc.)

## Build Command

```bash
cd /home/havoc/bottop
make -j$(nproc) STATIC= GPU_SUPPORT=false RSMI_STATIC= ADDFLAGS="-DAZEROTHCORE_SUPPORT"
```

## Status

✅ **FIXED** - Single-level brackets now display correctly as "60", "70", "80"

---

**Fix Date**: December 15, 2025
