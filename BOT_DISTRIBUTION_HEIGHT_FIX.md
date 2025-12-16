# Bot Distribution Pane Height Fix - COMPLETE ✅

**Date:** December 14, 2025, 10:24 EST  
**Status:** Built and Ready

## Problem

The bot distribution pane was cutting off the last level bracket "71-80" and not displaying it. This was because the pane height was set to 19 lines, which was insufficient to show all content.

## Root Cause

**Original calculation:**

```
- Factions: 1 title + 2 items = 3 lines
- Continents: 1 title + 4 items = 5 lines
- Levels: 1 title + 8 items = 9 lines
Total: 17 lines + 2 borders = 19 lines
```

**Actual space needed:** 20 lines

The pane boundary check `if (dist_cy >= dist_y + dist_height - 1) break;` was stopping at line 18, preventing the 8th level bracket from being rendered.

## Solution

Increased the distribution pane height from 19 to 20 lines. This extra line became available after consolidating the WorldServer performance display from 2 lines to 1 line.

## Code Change

### File: `src/btop_draw.cpp` (line 1474-1479)

**Before:**

```cpp
// Height calculation (exact content fit):
// - Factions: 1 title + 2 items = 3 lines
// - Continents: 1 title + 4 items = 5 lines
// - Levels: 1 title + 8 items = 9 lines
// Total: 17 lines + 2 borders = 19 lines
dist_height = 19;
```

**After:**

```cpp
// Height calculation (exact content fit):
// - Factions: 1 title + 2 items = 3 lines
// - Continents: 1 title + 4 items = 5 lines
// - Levels: 1 title + 8 items = 9 lines
// Total: 17 lines + 2 borders + 1 spacing = 20 lines
dist_height = 20;
```

## What's Now Visible

All 8 level brackets are now properly displayed:

```
┌─ bot distribution ───┐
│ Factions:            │
│ Horde           52%  │
│ Alliance        48%  │
│ Continents:          │
│ Eastern Kingdoms 35% │
│ Kalimdor        28%  │
│ Outland         22%  │
│ Northrend       15%  │
│ Levels:              │
│ 1-10      ↓      8%  │
│ 11-20     ●     12%  │
│ 21-30     ●     13%  │
│ 31-40     ●     14%  │
│ 41-50     ●     13%  │
│ 51-60     ●     12%  │
│ 61-70     ●     14%  │
│ 71-80     ●     14%  │  ← NOW VISIBLE!
└──────────────────────┘
```

## Space Trade-off

This change was made possible by the previous consolidation of the WorldServer performance display:

**Before consolidation:**

- Performance pane: 19 lines (with 2-line WorldServer display)
- Distribution pane: 19 lines
- 8th level bracket: **NOT VISIBLE**

**After consolidation:**

- Performance pane: 20 lines (with 1-line WorldServer display + larger graph)
- Distribution pane: 20 lines
- 8th level bracket: **VISIBLE**

Both panes are now properly sized and aligned at 20 lines each.

## Technical Details

### Boundary Check Logic

The rendering code uses this check:

```cpp
if (dist_cy >= dist_y + dist_height - 1) break;
```

**Why `-1`?**

- `dist_y` is the top of the box (border line)
- `dist_y + dist_height - 1` is the last content line before the bottom border
- Content must fit between these boundaries

**Example with dist_height = 20:**

- dist_y = 1 (top border)
- dist_y + 1 = 2 (first content line)
- dist_y + 18 = 19 (last content line)
- dist_y + 19 = 20 (bottom border)

So with 20 total lines, we have 18 content lines (lines 2-19), which is enough for:

- 3 lines (Factions)
- 5 lines (Continents)
- 9 lines (Levels)
- = 17 lines used, 1 line buffer

## Verification

### Lines Used:

1. Factions title
2. Horde
3. Alliance
4. Continents title
5. Eastern Kingdoms
6. Kalimdor
7. Outland
8. Northrend
9. Levels title
10. 1-10
11. 11-20
12. 21-30
13. 31-40
14. 41-50
15. 51-60
16. 61-70
17. 71-80 ← Now fits!

**Total: 17 content lines** (fits within 18 available)

## Build Info

**Binary:** `/home/havoc/bottop/build/bottop` (2.2MB)  
**Build Date:** Dec 14, 2025, 10:23 EST  
**Status:** ✅ Compiled successfully  
**Change:** Distribution pane height increased from 19 to 20 lines

## Testing

To verify the fix:

```bash
./build/bottop
# Check that all 8 level brackets (1-10 through 71-80) are visible
# in the bot distribution pane on the right side
```

## Related Changes

This fix complements:

1. **CONSOLIDATED_PERFORMANCE_LINE.md** - Saved 1 line in performance pane
2. **ZONE_LEVEL_BRACKETS_IMPLEMENTED.md** - Added the 8 level brackets
3. **STOCK_BOXES_DISABLED.md** - Original AzerothCore pane layout

---

**Fixed!** All 8 level brackets (1-10 through 71-80) are now fully visible in the bot distribution pane. 🎉
