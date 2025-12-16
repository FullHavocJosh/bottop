# Zone Display Consolidation

## Changes Made

### 1. Consolidated Level Columns

**Location:** `src/btop_draw.cpp:1999-2013, 2074-2078`

**Before:** Two separate columns "Min Lvl" and "Max Lvl"
**After:** Single "Levels" column showing "min-max" format

#### Header Changes:

```cpp
// Combined sort indicator for Levels (if either min or max level is being sorted)
string levels_sort = (zone_sort_column == Draw::AzerothCore::ZoneSortColumn::MIN_LEVEL ||
                      zone_sort_column == Draw::AzerothCore::ZoneSortColumn::MAX_LEVEL) ? " " + sort_indicator : "";

out += Mv::to(cy, x + 2) + title + "Name" + name_sort;
out += Mv::to(cy, x + 40) + title + "Bots" + bots_sort;
out += Mv::to(cy, x + 52) + "Levels" + levels_sort;  // <-- Single column at x+52
out += Mv::to(cy, x + 70) + "Align" + align_sort;
```

#### Zone Row Changes:

```cpp
// Format level range as "min-max"
string level_range = to_string(zone.min_level) + "-" + to_string(zone.max_level);

out += Mv::to(cy, x + 52) + (is_selected ? row_fg : Theme::c("inactive_fg")) + rjust(level_range, 10);
```

### 2. Remove Duplicate Region Headers

**Location:** `src/btop_draw.cpp:1977-1983`

**Problem:** When a region name matches a zone name (e.g., "Arathi Highlands"), both the region header and zone line appeared

**Solution:** Skip adding region header when `zone.region == zone.name`:

```cpp
// Add region header if changed (only if continent is expanded)
// Skip region header if region name matches zone name (to avoid duplication)
if (expanded_continents.contains(prep_continent) &&
    !zone.region.empty() && zone.region != prep_region && zone.region != zone.name) {
    prep_region = zone.region;
    zone_display_list.push_back({Draw::AzerothCore::DisplayItem::REGION, orig_idx, zone.region});
}
```

## Display Format

### Before:

```
Name            Bots  Min Lvl  Max Lvl  Align
───────────────────────────────────────────────
Total Bots: 1718
  ▼ Eastern Kingdoms
    - Arathi Highlands
      ● Arathi Highlands  23      30       60    100%
```

### After:

```
Name                    Bots     Levels  Align
───────────────────────────────────────────────
Total Bots: 1718
  ▼ Eastern Kingdoms
    ● Arathi Highlands     23      30-60  100%
```

## Benefits

1. **More Compact:** Single level column saves horizontal space
2. **Easier to Read:** "30-60" is clearer than two separate numbers
3. **No Duplication:** Region headers only show when they differ from zone names
4. **Cleaner Hierarchy:** Removes redundant intermediate lines

## Column Layout

- **Name:** x+2 (left-aligned, variable width based on indent)
- **Bots:** x+40 (right-aligned, 6 chars wide)
- **Levels:** x+52 (right-aligned, 10 chars wide, format: "XX-XX")
- **Align:** x+70 (right-aligned, 3 chars + "%")

## Files Modified

- `src/btop_draw.cpp` - Header columns, zone display, region logic

## Build Status

✅ Successfully compiled

- Binary: `/home/havoc/bottop/build/bottop`
- Build time: Dec 14, 2025, 16:02 EST
- Warning: 1 harmless unused-lambda-capture warning (false positive)
