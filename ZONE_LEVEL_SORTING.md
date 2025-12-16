# Zone Sorting by Level Bracket

## Change Made

Updated zone sorting to organize subzones by level bracket within each continent.

**Location:** `src/btop_draw.cpp:1890-1928`

## Sorting Order

### Primary Sort: Continent

Continents are displayed in expansion order:

1. Eastern Kingdoms (Classic)
2. Kalimdor (Classic)
3. Outland (The Burning Crusade)
4. Northrend (Wrath of the Lich King)
5. Unknown (any unmapped zones)

### Secondary Sort: User-Selected Column

Users can sort by any column using keybinds:

- **n** - Sort by Name
- **b** - Sort by Bots
- **m** - Sort by Min Level
- **M** - Sort by Max Level
- **a** - Sort by Alignment
- **r** - Reverse sort order

### Tertiary Sort: Level Bracket (Default)

When no specific column sort is active or when values are tied, zones are sorted by:

1. **Min Level** (ascending) - Lowest starting level first
2. **Max Level** (ascending) - If min levels are equal, lowest max level first
3. **Name** (alphabetical) - If both levels are equal

## Example Display

```
Name                    Bots  Level Range  Align
──────────────────────────────────────────────────
All Zones:              1718
  ▼ Eastern Kingdoms     856
    ● Elwynn Forest      456        1-10    88%
    ● Dun Morogh          89        1-10    95%
    ● Westfall           123       10-20    92%
    ● Redridge Mountains  67       15-25    87%
    ● Duskwood            45       18-30    91%
    ● Arathi Highlands    23       30-60   100%
  ▼ Outland              862
    ● Hellfire Peninsula 234       58-63    94%
    ● Zangarmarsh        156       60-64    96%
    ● The Mechanar        12       68-70   100%
    ● The Botanica        18       67-69   100%
```

## Sorting Logic

```cpp
// Tertiary sort: if result is tied, sort by level bracket
if (result == 0) {
    // Sort by min_level first (1-10 before 30-40)
    if (a.min_level != b.min_level) {
        result = (a.min_level < b.min_level) ? -1 : 1;
    }
    // If min_level is same, sort by max_level (1-10 before 1-20)
    else if (a.max_level != b.max_level) {
        result = (a.max_level < b.max_level) ? -1 : 1;
    }
    // If both levels same, sort by name (alphabetical)
    else {
        result = a.name.compare(b.name);
    }
}
```

## Benefits

1. **Logical Progression:** Zones appear in level order, matching natural questing progression
2. **Easy Navigation:** Find appropriate zones for specific level ranges quickly
3. **Clear Organization:** Low-level zones at top, high-level zones at bottom
4. **User Control:** Can still manually sort by any column using keybinds

## Files Modified

- `src/btop_draw.cpp` - Zone sorting logic (lines 1890-1928)

## Build Status

✅ Successfully compiled

- Binary: `/home/havoc/bottop/build/bottop`
- Build time: Dec 14, 2025, 17:33 EST
